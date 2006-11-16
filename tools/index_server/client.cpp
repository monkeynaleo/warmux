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
#include "net_data.h"
#include "debug.h"
#include "../../src/network/top_server_msg.h"

// map < socket_fd, client >
extern std::list<Client*> clients;

// number of server currently logged
unsigned int nb_server = 0;

Client::Client(int client_fd, unsigned int & ip)
{
	handshake_done = false;
	is_hosting = false;
	Host(client_fd, ip);
}

Client::~Client()
{
	if( is_hosting )
		nb_server--;
}

bool Client::HandleMsg(const TopServerMsg & msg_id, const std::string & str)
{
	if( msg_id == TS_MSG_VERSION )
	{
		if(str == "0.8beta1")
		{
			DPRINT(MSG, "Version checked successfully");
			handshake_done = true;
			return SendSignature();		
		}
		return false;
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
		return SendList();
		break;
	default:
		DPRINT(MSG, "Wrong message");
		return false;
	}
	return true;
}

bool Client::SendSignature()
{
	DPRINT(MSG, "Sending signature");
	return SendStr("MassMurder!");
}

bool Client::SendList()
{
	DPRINT(MSG, "Sending list..");

	if( ! SendInt(nb_server) )
		return false;

	for(std::list<Client*>::iterator client = clients.begin();
		client != clients.end();
		++client)
	{
		if( (*client)->is_hosting )
		{
			if( ! SendInt((*client)->GetIP()) )
				return false;
		}
	}
	return true;
}

