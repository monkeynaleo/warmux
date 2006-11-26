#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <map>
#include "client.h"
#include "config.h"
#include "net_data.h"
#include "debug.h"
#include "../../src/network/index_svr_msg.h"

// map < version, client >
extern std::multimap<std::string,Client*> clients;

// number of server currently logged depending on the version:
std::map<std::string, int> nb_server;

Client::Client(int client_fd, unsigned int & ip)
{
	version = "";
	handshake_done = false;
	is_hosting = false;
	Host(client_fd, ip);
}

Client::~Client()
{
	if( is_hosting )
	{
		if( version != "" )
		{
			nb_server[ version ]--;
		}
	}
}

bool Client::HandleMsg(const IndexServerMsg & msg_id, const std::string & str)
{
	if( msg_id == TS_MSG_VERSION )
	{
		version = str;
		if(str == "0.8beta1")
		{
			DPRINT(MSG, "Version checked successfully");
			handshake_done = true;
			SetVersion( str );
			return SendSignature();		
		}
		else
		if(str == "WIS")
		{
			DPRINT(MSG, "New index server connected");
			// Send our version, the distant server will shutdown if he has a lower version
			SendInt((int) TS_MSG_WIS_VERSION );
			SendInt( VERSION );
			return true;
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
		if( nb_server.find(version) != nb_server.end() )
			nb_server[ version ]++;
		else
			nb_server[ version ] = 1;

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
		if(str == "0.8beta1")
			return false;
	}
	return true;
}

void Client::SetVersion(const std::string & ver)
{
	version = ver;
	clients.insert( std::make_pair(version, this) );

	// We are currently registered as an unknown version
	// So, we unregister it:
	std::multimap<std::string, Client*>::iterator client;
	client = clients.find( "unknown" );
	if( client != clients.end() )
	{
		do
		{
			if( client->second == this )
			{
				clients.erase( client );
				return;
			}
			++client;
		} while( client != clients.upper_bound( "unknown" ) );
	}
}

bool Client::SendSignature()
{
	DPRINT(MSG, "Sending signature");
	SendInt((int)TS_MSG_VERSION);
	return SendStr("MassMurder!");
}

bool Client::SendList()
{
	DPRINT(MSG, "Sending list..");

	int nb_s = 0;
	if( nb_server.find( version ) != nb_server.end() )
		nb_s = nb_server[ version ];

	if( ! SendInt(nb_s) )
		return false;

	std::multimap<std::string, Client*>::iterator client = clients.find( version);
	if( client != clients.end() )
	{
		do
		{
			if( client->second->is_hosting )
			{
				if( ! SendInt(client->second->GetIP()) )
					return false;
			}
			++client;
		} while (client != clients.upper_bound(version));
	}
	return true;
}
