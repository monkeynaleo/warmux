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
 * Notify a top server of an opened wormux server
 * Obtain information about running games from a top server
 *****************************************************************************/

#include <SDL_net.h>
#include "top_server.h"
#include "top_server_msg.h"
#include "network.h"
#include "gui/question.h"
#include "include/app.h"
#include "include/constant.h"
#include "tool/debug.h"
#include "tool/i18n.h"


TopServer top_server;


TopServer::TopServer()
{
  hidden_server = false;
  connected = false;
}

TopServer::~TopServer()
{
  if(connected)
    Disconnect();
}

/*************  Connection  /  Disconnection  ******************/
bool TopServer::Connect()
{
  assert(!connected);

  if( hidden_server )
    return true;

  Question question;
  question.Set(_("Contacting main server..."),1,0);
  question.Draw();
  AppWormux::GetInstance()->video.Flip();

  network.Init();

  MSG_DEBUG("top_server", "Opening connection");

  if( SDLNet_ResolveHost(&ip, "defert.dyndns.org" , 9997) == -1 )
  {
    question.Set(_("Invalid top server adress!"),1,0);
    question.Ask();
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return false;
  }

  socket = SDLNet_TCP_Open(&ip);

  if(!socket)
  {
    question.Set(_("Unable to contact top server!"),1,0);
    question.Ask();
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    return false;
  }

  connected = true;

  return HandShake();
}


void TopServer::Disconnect()
{
  if( hidden_server )
  {
    hidden_server = false;
    return;
  }

  if( !connected )
    return;

  MSG_DEBUG("top_server", "Closing connection");

  SDLNet_TCP_Close(socket);
  connected = false;
}

/*************  Basic transmissions  ******************/
void TopServer::Send(const int& nbr)
{
  char packet[4];
  // this is not cute, but we don't want an int -> uint conversion here
  Uint32 u_nbr = *((Uint32*)&nbr);

  SDLNet_Write32(u_nbr, packet);
  SDLNet_TCP_Send(socket, packet, sizeof(packet));
}

void TopServer::Send(const std::string &str)
{
  Send(str.size());
  SDLNet_TCP_Send(socket, (void*)str.c_str(), str.size());
}

int TopServer::ReceiveInt()
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

std::string TopServer::ReceiveStr()
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

bool TopServer::HandShake()
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

void TopServer::SendServerStatus()
{
  assert(network.IsServer());

  if(hidden_server)
    return;
  Send(TS_MSG_HOSTING);
}

std::list<std::string> TopServer::GetServerList()
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
    lst.push_back( std::string(addr) );
  }
  return lst;
}
