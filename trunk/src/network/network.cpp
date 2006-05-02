/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
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
 * Network layer for Wormux.
 *****************************************************************************/

#include "network.h"
//-----------------------------------------------------------------------------
#include <SDL_net.h>
#include <SDL_thread.h>
#include "../include/action_handler.h"
#include "../tool/debug.h"
#include "../tool/i18n.h"
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
Network network;
//-----------------------------------------------------------------------------

Network::Network()
{
  thread = NULL;
  m_is_connected = false;
  m_is_server = false;
  m_is_client = false;
  state = NETWORK_NOT_CONNECTED;
}

//-----------------------------------------------------------------------------
int net_thread_func(void* no_param)
{
  network.ReceiveActions();
  return 1;
}

//-----------------------------------------------------------------------------

void Network::Init()
{
  assert(thread == NULL);
  if (SDLNet_Init()) {
      Error(_("Fail to initialize network library!"));
  }
  thread = SDL_CreateThread( &net_thread_func, NULL);
}

//-----------------------------------------------------------------------------

Network::~Network() 
{
  SDLNet_Quit();
}

//-----------------------------------------------------------------------------

void Network::disconnect() 
{
  if(m_is_server)
    SDLNet_TCP_Close(client);

  SDLNet_TCP_Close(socket);

  m_is_connected = false;
  m_is_server = false;
  m_is_client = false;
}

//-----------------------------------------------------------------------------

void Network::client_connect(const std::string &host, const std::string& port) 
{
  MSG_DEBUG("network", "Client connect to %s:%s", host.c_str(), port.c_str());

  m_is_client = true;
  m_is_server = false;
  state = NETWORK_WAIT_SERVER;

  int prt=0;
  IPaddress ip;
  sscanf(port.c_str(),"%i",&prt);

  if(SDLNet_ResolveHost(&ip,host.c_str(),prt)==-1)
  {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(1);
  }

  socket = SDLNet_TCP_Open(&ip);

  if(!socket)
  {
    printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    exit(2);
  }

	m_is_connected = true;

  printf("\nConnected\n");
}

//-----------------------------------------------------------------------------

void Network::server_start(const std::string &port) 
{
  MSG_DEBUG("network", "Start server on port %s", port.c_str());

  m_is_server = true;
  m_is_client = false;
  state = NETWORK_WAIT_CLIENTS;

  IPaddress ip;
  int prt;
  sscanf(port.c_str(),"%i",&prt);

  if(SDLNet_ResolveHost(&ip,NULL,prt)==-1)
  {
    printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
    exit(1);
  }

  socket = SDLNet_TCP_Open(&ip);

  if(!socket)
  {
    printf("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
    exit(2);
  }

  do
  {
    client = SDLNet_TCP_Accept(socket);
    printf("Waiting for client ... \n");
    SDL_Delay(1000);
  } while(!client);

  m_is_connected = true;
  printf("\nConnected\n");
  state = NETWORK_SERVER_INIT_GAME;
}

//-----------------------------------------------------------------------------

// Send Messages
void Network::SendAction(const Action &action) 
{
  if (!m_is_connected) return;

  Uint32 packet[100];
  memset(packet,0,400);
  action.Write(packet);

  if(m_is_client)
    SDLNet_TCP_Send(socket, packet, 100);
  else
    SDLNet_TCP_Send(client, packet, 100);

  std::cout << "sending " << action << std::endl;
}

//-----------------------------------------------------------------------------
// Send Messages
void Network::ReceiveActions() 
{
	while (!m_is_connected) SDL_Delay(1000);

  Uint32 packet[100];
  memset(packet,0, 400);

  if(m_is_client)
  {
    int received;
    do
    {
      received = SDLNet_TCP_Recv(socket, packet, 100);
      Action* a = make_action(packet);
      std::cout << "received " << *a << std::endl;
      ActionHandler::GetInstance()->NewAction(*a, false);
      memset(packet,0, 400);
      delete a;
    } while(received > 0);
  }
  else
  {
    do
    {
      if( SDLNet_TCP_Recv(client, packet, 100) > 0 )
      {
        Action* a = make_action(packet);
        std::cout << "received " << *a << std::endl;
        ActionHandler::GetInstance()->NewAction(*a, false);
        memset(packet,0, 400);
        delete a;
      }
    } while(true);
  }
}

//-----------------------------------------------------------------------------

Action* Network::make_action(Uint32* packet)
{
  Action_t type = (Action_t)(packet[0]);
  Uint32* input = &packet[1];

  switch(type)
  {
  case ACTION_SHOOT:
    return new ActionDoubleInt(type, input);

  case ACTION_SEND_RANDOM:
    return new ActionDouble(type, input);

  case ACTION_MOVE_CHARACTER:
    return new ActionInt2(type, input);

  case ACTION_CHANGE_CHARACTER:
  case ACTION_CHANGE_WEAPON:
  case ACTION_WIND:
  case ACTION_SET_CHARACTER_DIRECTION:
  case ACTION_SET_FRAME:
    return new ActionInt(type, input);

  case ACTION_SET_GAME_MODE:
  case ACTION_SET_MAP:
  case ACTION_CHANGE_TEAM:
  case ACTION_NEW_TEAM:
  case ACTION_SEND_VERSION:
  case ACTION_SEND_TEAM:
  case ACTION_SET_SKIN:
    return new ActionString(type, input);

  case ACTION_WALK:
  case ACTION_MOVE_LEFT:
  case ACTION_MOVE_RIGHT:
  case ACTION_JUMP:
  case ACTION_HIGH_JUMP:
  case ACTION_UP:
  case ACTION_DOWN:
  case ACTION_CLEAR_TEAMS:
  case ACTION_START_GAME:
  case ACTION_ASK_VERSION:
    return new Action(type);

  default:
    assert(false);
    return new Action(type);
  }
}

//-----------------------------------------------------------------------------

bool Network::is_connected() { return m_is_connected; }
bool Network::is_local() { return !m_is_server && !m_is_client; }
bool Network::is_server() { return m_is_server; }
bool Network::is_client() { return m_is_client; }

//-----------------------------------------------------------------------------
