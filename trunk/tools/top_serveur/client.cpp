#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "client.h"
#include "debug.h"
#include "../../src/network/top_server_msg.h"


Client::Client(int client_fd, struct sockaddr_in client_address)
{
	address = client_address;
	fd = client_fd;
	msg_id = TS_NO_MSG;
	handshake_done = false;
	str_size = 0;
}

Client::~Client()
{
	close(fd);
}

int & Client::GetFD()
{
	return fd;
}

bool Client::ReceiveInt(int & nbr)
{
	uint32_t packet;
	if( read(fd, &packet, sizeof(packet)) == -1 )
		TELL_ERROR;

	unsigned int u_nbr = ntohl(packet);
	nbr = *((int*)&u_nbr);
	DPRINT("Received int: %i", nbr);
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
	DPRINT("Received string: %s", str);

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
	DPRINT("Sent int: %i", nbr);
}

void Client::SendStr(const std::string &full_str)
{
	SendInt((int)full_str.size());

	if( write(fd, full_str.c_str(), full_str.size()) == -1 )
		TELL_ERROR;

	DPRINT("Sent string: %s", full_str.c_str());
}


bool Client::Receive()
{
	if( ioctl(fd, FIONREAD, &received) == -1 )
		TELL_ERROR;

	if(received < 1)
		return false;

	if(msg_id == TS_NO_MSG)
	{
		int id;
		if( !ReceiveInt(id) )
			return false;
		msg_id = (TopServerMsg)id;
	}

	if( msg_id == TS_MSG_VERSION )
	{
		std::string version;
		if( ! ReceiveStr(version) )
			return false;

		if(version == "")
		{
			// version not finnished to receive
			return true;
		}

		if(version == "0.8beta1")
		{
			DPRINT("Version checked successfully");
			msg_id = TS_NO_MSG;
			SendSignature();
		}
		return true;
	}

	if( !handshake_done )
		return false;

	switch(msg_id)
	{
	default:
		DPRINT("Wrong message");
		return false;
	}
	return true;
}

void Client::SendSignature()
{
	DPRINT("Sending signature");
	SendStr("MassMurder!");
}

