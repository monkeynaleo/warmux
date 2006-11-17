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
 ******************************************************************************
 * Notify an index server of an opened wormux server
 * Obtain information about running games from an index server
 *****************************************************************************/

#include <SDL_net.h>
#include <fstream>
#include "download.h"
#include "index_server.h"
#include "index_svr_msg.h"
#include "network.h"
#include "gui/question.h"
#include "include/app.h"
#include "include/constant.h"
#include "tool/debug.h"
#include "tool/i18n.h"
#include "tool/random.h"

IndexServer index_server;

IndexServer::IndexServer()
{
  hidden_server = false;
  connected = false;
  server_lst.clear();
  first_server = server_lst.end();
  current_server = server_lst.end();
}

IndexServer::~IndexServer()
{
  server_lst.clear();
  if(connected)
    Disconnect();
}

/*************  Connection  /  Disconnection  ******************/
bool IndexServer::Connect()
{
  MSG_DEBUG("index_server", "Connecting..");
  assert(!connected);

  if( hidden_server )
    return true;

  if( !GetServerList() )
    return false;

  std::string addr;
  int port;

  // Cycle through the list of server
  // Until we find one running
  while( GetServerAddress( addr, port) )
  {
    if( ConnectTo( addr, port) )
      return true;
  }

  Question question;
  question.Set(_("Unable to contact an index server!"),1,0);
  question.Ask();

  return false;
}

bool IndexServer::ConnectTo(const std::string & address, const int & port)
{
  MSG_DEBUG("index_server", "Connecting to %s %i", address.c_str(), port);
  Question question;
  question.Set(_("Contacting main server..."),1,0);
  question.Draw();
  AppWormux::GetInstance()->video.Flip();

  network.Init(); // To get SDL_net initialized

  MSG_DEBUG("index_server", "Opening connection");

//  if( SDLNet_ResolveHost(&ip, "localhost" , port) == -1 )
  if( SDLNet_ResolveHost(&ip, address.c_str() , port) == -1 )
  {
    question.Set(_("Invalid index server adress!"),1,0);
    question.Ask();
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return false;
  }

  socket = SDLNet_TCP_Open(&ip);

  if(!socket)
  {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return false;
  }

  connected = true;

  return HandShake();
}

void IndexServer::Disconnect()
{
  if( hidden_server )
  {
    hidden_server = false;
    return;
  }

  if( !connected )
    return;

  MSG_DEBUG("index_server", "Closing connection");
  first_server = server_lst.end();
  current_server = server_lst.end();

  SDLNet_TCP_Close(socket);
  connected = false;
}

static ssize_t getline(std::string& line, std::ifstream& file)
{
	std::getline(file, line);
	return file.gcount();
}

bool IndexServer::GetServerList()
{
  MSG_DEBUG("index_server", "Retrieving server list");
  // If we already have it, no need to redownload it
  if(server_lst.size() != 0)
    return true;

  // Download the list of user
  const std::string server_list_url = "http://www.wormux.org/server_list";
  const std::string server_file = Config::GetInstance()->GetPersonalDir() + "server_list";

  if( !downloader.Get(server_list_url.c_str(), server_file.c_str()) )
    return false;

  // Parse the file
  std::ifstream fin;
  fin.open(server_file.c_str(), std::ios::in);
  if(!fin)
  	return false;

  /*char * line = NULL;
  size_t len = 0;*/
  ssize_t read;
  std::string line;
  
  while ((read = getline(line, fin)) > 0)
  {
    // Parse the line
    char* str = (char*) malloc(line.size() + 1);
    str = (char*) memcpy(str, line.c_str(), line.size());

    if(*str == '#' || *str == '\n' || *str == '\0')
      continue;

    // find the port delimiter ':'
    int str_size = 0;
    while( *str != ':' && *str != '\n' && *str != '\0' )
    {
      str_size++;
      str++;
    }

    if( *str != ':' )
      continue;

    *str = '\0';

    str++;
    int port = 0;
    sscanf(str, "%i", &port);

    server_lst[ line ] = port;
  }

  fin.close();

  first_server = server_lst.end();
  current_server = server_lst.end();
  MSG_DEBUG("index_server", "Server list retrieved. %i servers are running", server_lst.size());

  return (server_lst.size() != 0);
}

bool IndexServer::GetServerAddress( std::string & address, int & port)
{
  MSG_DEBUG("index_server", "Trying a new server");
  // Cycle through the server list to find the first one
  // accepting connection
  if( first_server == server_lst.end() )
  {
    // First try :
    // Randomly select a server in the list
    int nbr = randomObj.GetLong( 0, server_lst.size()-1 );
    first_server = server_lst.begin();
    while(nbr--)
      ++first_server;

    assert(first_server != server_lst.end());

    current_server = first_server;

    address = current_server->first;
    port = current_server->second;
    return true;
  }

  ++current_server;
  if( current_server == server_lst.end() )
    current_server = server_lst.begin();

  address = current_server->first;
  port = current_server->second;

  return (current_server != first_server);
}

/*************  Basic transmissions  ******************/
void IndexServer::Send(const int& nbr)
{
  char packet[4];
  // this is not cute, but we don't want an int -> uint conversion here
  Uint32 u_nbr = *((Uint32*)&nbr);

  SDLNet_Write32(u_nbr, packet);
  SDLNet_TCP_Send(socket, packet, sizeof(packet));
}

void IndexServer::Send(const std::string &str)
{
  Send(str.size());
  SDLNet_TCP_Send(socket, (void*)str.c_str(), str.size());
}

int IndexServer::ReceiveInt()
{
  char packet[4];
  if( SDLNet_TCP_Recv(socket, packet, sizeof(packet)) < 1 )
  {
    Disconnect();
    return 0;
  }

  Uint32 u_nbr = SDLNet_Read32(packet);
  int nbr = *((int*)&u_nbr);
  return nbr;
}

std::string IndexServer::ReceiveStr()
{
  int size = ReceiveInt();

  if(!connected)
    return "";

  assert(size > 0);
  char* str = new char[size+1];

  if( SDLNet_TCP_Recv(socket, str, size) < 1 )
  {
    Disconnect();
    return "";
  }

  str[size] = '\0';

  std::string st(str);
  delete []str;
  return st;
}

bool IndexServer::HandShake()
{
  Send(TS_MSG_VERSION);
  Send(Constants::VERSION);

  std::string sign = ReceiveStr();
  if(sign != "MassMurder!")
  {
    Question question;
    question.Set(_("It doesn't seem to be a valid Wormux server..."),1,0);
    question.Ask();
    Disconnect();
    return false;
  }
  return true;
}

void IndexServer::SendServerStatus()
{
  assert(network.IsServer());

  if(hidden_server)
    return;
  Send(TS_MSG_HOSTING);
}

std::list<std::string> IndexServer::GetHostList()
{
  Send(TS_MSG_GET_LIST);
  int lst_size = ReceiveInt();
  std::list<std::string> lst;
  while(lst_size--)
  {
    IPaddress ip;
    ip.host = ReceiveInt();
    ip.port = 0;
    const char* addr = SDLNet_ResolveIP(&ip);
    if(addr != NULL)
      lst.push_back( std::string(addr) );
    else
    {
      // We can't resolve the hostname, so just show the ip address
      unsigned char* str_ip = (unsigned char*)&ip.host;
      char formated_ip[16];
      snprintf(formated_ip, 16, "%i.%i.%i.%i", (int)str_ip[0],
                                           (int)str_ip[1],
                                           (int)str_ip[2],
                                           (int)str_ip[3]);
      lst.push_back( std::string(formated_ip) );
    }
  }
  return lst;
}
