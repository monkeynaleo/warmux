#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <list>
#include "net_data.h"
#include "debug.h"

NetData::NetData()
{
	str_size = 0;
	connected = false;
	msg_id = TS_NO_MSG;
}

NetData::~NetData()
{
	if(connected)
		close(fd);
	DPRINT(CONN, "Disconnected.");
}

void NetData::Host(const int & client_fd, const unsigned int & ip)
{
	fd = client_fd;
	ip_address = *(int*)&ip;
	connected = true;
}

void NetData::ConnectTo(const std::string & address, const int & port)
{
	fd = socket(AF_INET, SOCK_STREAM, 0);

	if(fd == -1)
	{
		PRINT_ERROR;
		DPRINT(INFO, "Rejected");
		return;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr( address.c_str() );
	addr.sin_port = htons(port);

	if( connect(fd, (struct sockaddr*) &addr, sizeof(addr)) != 0 )
		TELL_ERROR;

	connected = true;
	DPRINT(CONN, "Connected.");
}

bool NetData::ReceiveInt(int & nbr)
{
	uint32_t packet;
	if( read(fd, &packet, sizeof(packet)) == -1 )
	{
		PRINT_ERROR;
		return false;
	}

	unsigned int u_nbr = ntohl(packet);
	nbr = *((int*)&u_nbr);
	DPRINT(TRAFFIC, "Received int: %i", nbr);
	received -= 4;
	return true;
}

bool NetData::ReceiveStr(std::string & full_str)
{
	const unsigned int max_str_size = 10;

	if(str_size == 0)
	{
		// We don't know the string size -> read it
		if(received < 4)
			return true;

		int size;
		if( !ReceiveInt(size) )
			return false;

		str_size = (unsigned int)size;
		if(str_size <= 0 || str_size > max_str_size)
			return false;

		str = new char[str_size+1];
		memset(str, 0, str_size+1);
	}

	// Check if the string is already arrived
	if(received == 0)
	{
		full_str = "";
		return true;
	}

	unsigned int old_size = strlen(str);
	unsigned int to_receive = str_size - old_size;
	int str_received;

	str_received = read(fd, str + old_size, to_receive);

	if( str_received == -1 )
	{
		PRINT_ERROR;
		return false;
	}

	// Check the client is not sending some 0
	// that could lock us
	if( strlen(str) != old_size + str_received )
		return false;

	received -= str_received;
	DPRINT(TRAFFIC, "Received string: %s", str);

	if(strlen(str) == str_size)
	{
		full_str = std::string(str);
		delete []str;
		str_size = 0;
	}
	return true;
}

bool NetData::SendInt(const int &nbr)
{
	uint32_t packet;

	unsigned int u_nbr = *((unsigned int*)&nbr);
	packet = htonl(u_nbr);

	if( write(fd, &packet, sizeof(packet)) == -1 )
	{
		PRINT_ERROR;
		return false;
	}

	DPRINT(TRAFFIC, "Sent int: %i", nbr);
	return true;
}

bool NetData::SendStr(const std::string &full_str)
{
	if( ! SendInt((int)full_str.size()) )
		return false;

	if( write(fd, full_str.c_str(), full_str.size()) == -1 )
	{
		PRINT_ERROR;
		return false;
	}

	DPRINT(TRAFFIC, "Sent string: %s", full_str.c_str());
	return true;
}



bool NetData::Receive()
{
	if( ioctl( GetFD(), FIONREAD, &received) == -1 )
	{
		PRINT_ERROR;
		return false;
	}

	// received < 1 when the client disconnect
	if(received < 1)
		return false;

	// Get the ID of the message
	// if we don't already know it
	if(msg_id == TS_NO_MSG)
	{
		int id;
		if( !ReceiveInt(id) )
			return false;
		msg_id = (TopServerMsg)id;
	}

	std::string full_str = "";
	// If a string is embedded in the msg, get it
	if( msg_id == TS_MSG_VERSION )
	{
		if( ! ReceiveStr(full_str) )
			return false;

		// If we didn't received the full string yet,
		// just go back
                if( full_str == "" )
			return true;
	}

	bool result = HandleMsg( msg_id, full_str );

	// We are ready to read a new message
	msg_id = TS_NO_MSG;
	return result;
}
