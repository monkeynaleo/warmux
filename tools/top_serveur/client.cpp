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
#include "client.h"
#include "debug.h"
#include "../../src/network/top_server_msg.h"

// map < socket_fd, client >
extern std::list<Client*> clients;

// number of server currently logged
unsigned int nb_server = 0;

Client::Client(int client_fd, unsigned int & ip)
{
	fd = client_fd;
	msg_id = TS_NO_MSG;
	handshake_done = false;
	str_size = 0;
	is_hosting = false;
	ip_address = *(int*)&ip;
}

Client::~Client()
{
	close(fd);
	if( is_hosting )
		nb_server--;
}

int & Client::GetFD()
{
	return fd;
}

int & Client::GetIp()
{
	return ip_address;
//	return std::string(inet_ntoa(*(struct in_addr*)*info->h_addr_list));
}

bool Client::ReceiveInt(int & nbr)
{
	uint32_t packet;
	if( read(fd, &packet, sizeof(packet)) == -1 )
		TELL_ERROR;

	unsigned int u_nbr = ntohl(packet);
	nbr = *((int*)&u_nbr);
	DPRINT(TRAFFIC, "Received int: %i", nbr);
	received -= 4;
	return true;
}

bool Client::ReceiveStr(std::string & full_str)
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
		TELL_ERROR;

	// Check the client is not sending some 0
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

void Client::SendInt(const int &nbr)
{
	uint32_t packet;

	unsigned int u_nbr = *((unsigned int*)&nbr);
	packet = htonl(u_nbr);

	if( write(fd, &packet, sizeof(packet)) == -1 )
		TELL_ERROR;
	DPRINT(TRAFFIC, "Sent int: %i", nbr);
}

void Client::SendStr(const std::string &full_str)
{
	SendInt((int)full_str.size());

	if( write(fd, full_str.c_str(), full_str.size()) == -1 )
		TELL_ERROR;

	DPRINT(TRAFFIC, "Sent string: %s", full_str.c_str());
}


bool Client::Receive()
{
	if( ioctl(fd, FIONREAD, &received) == -1 )
		TELL_ERROR;

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

	if( msg_id == TS_MSG_VERSION )
	{
		if(full_str == "0.8beta1")
		{
			DPRINT(MSG, "Version checked successfully");
			msg_id = TS_NO_MSG;
			handshake_done = true;
			SendSignature();		
		}
		return true;
	}

	if( !handshake_done )
		return false;

	switch(msg_id)
	{
	case TS_MSG_HOSTING:
		DPRINT(MSG, "This is a server");
		is_hosting = true;
		nb_server++;
		// TODO: try opening a connection to see if it's 
		// firewalled or not
		break;
	case TS_MSG_GET_LIST:
		if( is_hosting )
			return false;
		SendList();
		break;
	default:
		DPRINT(MSG, "Wrong message");
		return false;
	}

	// We are ready to read a new message
	msg_id = TS_NO_MSG;
	return true;
}

void Client::SendSignature()
{
	DPRINT(MSG, "Sending signature");
	SendStr("MassMurder!");
}

void Client::SendList()
{
	DPRINT(MSG, "Sending list..");
	SendInt(nb_server);
	for(std::list<Client*>::iterator client = clients.begin();
		client != clients.end();
		++client)
	{
		if( (*client)->is_hosting )
			SendInt((*client)->GetIp());
	}
}

