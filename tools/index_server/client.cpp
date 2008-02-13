/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ******************************************************************************/

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
#include "sync_slave.h"
#include "stat.h"
#include "../../src/network/index_svr_msg.h"

// map < version, client >
extern std::multimap<std::string,Client*> clients;
// map < version, ip >
extern std::multimap<std::string,FakeClient> fake_clients;

// number of server currently logged depending on the version:
std::map<std::string, int> nb_server;

Client::Client(int client_fd, unsigned int & ip)
{
    version = "";
    game_name = "";
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
            NotifyServers( false );
        }
    }
}

typedef int SOCKET;
#define SOCKET_PARAM    void
#define SOCKET_ERROR    (-1)
#define INVALID_SOCKET  (-1)
#define closesocket(fd) close(fd)

/* Nearly copy/paste from wormux/src/network/network.cpp::CheckHost */
static bool CheckHost(const int ip, int prt)
{
  unsigned char* str_ip = (unsigned char*)&ip;
  char formated_ip[16];
  snprintf(formated_ip, 16, "%i.%i.%i.%i", (int)str_ip[0],
	   (int)str_ip[1],
	   (int)str_ip[2],
	   (int)str_ip[3]);

  std::string host = std::string(formated_ip);

  DPRINT(CONN, "Checking connection to %s:%i", host.c_str(), prt);

  struct hostent* hostinfo;
  hostinfo = gethostbyname(host.c_str());
  if( ! hostinfo )
    return false /*CONN_BAD_HOST*/;

  SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
  if( fd == INVALID_SOCKET )
    return false /*CONN_BAD_SOCKET*/;

  // Set the timeout
#ifdef WIN32
  int timeout = 5000; //ms
#else
  struct timeval timeout;
  memset(&timeout, 0, sizeof(timeout));
  timeout.tv_sec = 5; // 5seconds timeout
#endif
  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (SOCKET_PARAM*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
  {
    fprintf(stderr, "Setting receive timeout on socket failed\n");
    return false /*CONN_BAD_SOCKET*/;
  }

  if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (SOCKET_PARAM*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
  {
    fprintf(stderr, "Setting send timeout on socket failed\n");
    return false /*CONN_BAD_SOCKET*/;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(prt);
#ifndef WIN32
  addr.sin_addr.s_addr = *(in_addr_t*)*hostinfo->h_addr_list;
#else
  addr.sin_addr.s_addr = inet_addr(inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list));
#endif

  if( connect(fd, (struct sockaddr*) &addr, sizeof(addr)) == SOCKET_ERROR )
  {
    return false /*GetError()*/;
  }
  closesocket(fd);

  DPRINT(CONN, "Checking connection to %s:%i : OK", host.c_str(), prt);
  return true /*CONNECTED*/;
}

bool Client::HandleMsg(const std::string & str)
{
    if( msg_id == TS_MSG_VERSION )
    {
        if( handshake_done )
            return false;
        version = str;
        if(str == "0.8beta1"
           || str == "0.8beta2"
           || str == "0.8beta3"
           || str == "0.8beta4"
           || str == "0.8svn"
           || str == "0.7.9rc1"
           || str == "0.7.9rc2"
           || str == "0.7.9")
        {
            DPRINT(MSG, "Version checked successfully");
            handshake_done = true;
            SetVersion( str );
            msg_id = TS_NO_MSG;
            stats.NewClient();
            return SendSignature();
        }
        else
        if(str == "WIS")
        {
            DPRINT(MSG, "New index server connected");
            // Send our version, the distant server will shutdown if he has a lower version
            SetVersion( str );
            SendInt((int) TS_MSG_WIS_VERSION );
            SendInt( VERSION );
            // We are ready to read a new message
            msg_id = TS_NO_MSG;
            handshake_done = true;
            return true;
        }
        return false;
    }

    if( !handshake_done )
        return false;

    switch(msg_id)
    {
    case TS_MSG_HOSTING:
        if(received < 4)
            return true;
        DPRINT(MSG, "This is a server (%s)", version.c_str());
        is_hosting = true;
        if( nb_server.find(version) != nb_server.end() )
            nb_server[ version ]++;
        else
            nb_server[ version ] = 1;
        if(! ReceiveInt(port) )
            return false;

        // try opening a connection to see if it's
        // firewalled or not
	if (!CheckHost(GetIP(), port)) {
	  DPRINT(MSG, "server is not reachable");
	  return false;
	}

        NotifyServers( true );
        stats.NewServer();

        break;
    case TS_MSG_GET_LIST:
        if( is_hosting )
            return false;
        msg_id = TS_NO_MSG;
        return SendList();
        break;
    case TS_MSG_GAMENAME:
        game_name = str;
	DPRINT(MSG, "game name is %s", game_name.c_str());
	break;
    default:
        DPRINT(MSG, "Wrong message");
        //if(str == "0.8beta1")
        return false;
    }
    // We are ready to read a new message
    msg_id = TS_NO_MSG;
    return true;
}

void Client::SetVersion(const std::string & ver)
{
    DPRINT(MSG, "Setting version to %s", ver.c_str());
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

    // TODO : replace this loop by the correct stl function
    std::multimap<std::string, FakeClient>::iterator fclient = fake_clients.find(version);
    if( fake_clients.find( version ) != fake_clients.end() )
    {
        do
        {
            nb_s++;
            ++fclient;
        } while (fclient != fake_clients.upper_bound(version));
    }

    if( ! SendInt(nb_s) )
        return false;

    std::multimap<std::string, Client*>::iterator client = clients.find(version);
    if( client != clients.end() )
    {
        do
        {
            if (client->second->is_hosting)
            {
	      if (! SendInt(client->second->GetIP()))
		return false;
	      if (! SendInt(client->second->port))
		return false;

	      if (client->second->game_name != "" /* this must stay only for old version */ &&
		  ! SendStr(client->second->game_name))
		return false;
            }
            ++client;
        } while (client != clients.upper_bound(version));
    }

    fclient = fake_clients.find(version);
    if( fclient != fake_clients.end() )
    {
        do
        {
            if( ! SendInt(fclient->second.ip) )
                return false;
            if( ! SendInt(fclient->second.port) )
                return false;
	    if (game_name != "" /* this must stay only for old version */ &&
		  ! SendStr(client->second->game_name))
		return false;
            ++fclient;
        } while (fclient != fake_clients.upper_bound(version));
    }
    return true;
}


void Client::NotifyServers(bool joining)
{
    // Tell other index server that a new wormux server just registered
    DPRINT(MSG, "Notify of a new wormux server..");

    std::multimap<std::string, Client*>::iterator serv = clients.find( sync_serv_version );
    if (serv != clients.end())
    {
        do
        {
            if (!serv->second->SendInt(TS_MSG_JOIN_LEAVE))
	      return /*false*/;
            if (! serv->second->SendStr(version))
	      return /*false*/;
            if (! serv->second->SendInt(GetIP()))
	      return /*false*/;
            // Send the port number : if this client is leaving, send -port
            if (! serv->second->SendInt(joining? port : -port))
	      return /*false*/;
	    if (! serv->second->SendStr(game_name))
	      return /*false*/;

            ++serv;
        } while (serv != clients.upper_bound(sync_serv_version));
    }
    return /*true*/;
}
