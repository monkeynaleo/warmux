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
 * Network layer for Wormux.
 *****************************************************************************/

#include "network.h"
//-----------------------------------------------------------------------------
#include "../game/game_mode.h"
#include "../game/game.h"
#include "../include/action_handler.h"
#include "../tool/debug.h"
#include "../tool/i18n.h"

#if defined(DEBUG) && not defined(WIN32)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
//-----------------------------------------------------------------------------
Network network;

//-----------------------------------------------------------------------------

Network::Network()
{
  max_nb_players = 0;

  m_connection = LOCAL_ONLY;
  state = NO_NETWORK; // useless value at beginning
  sdlnet_inited = false;
  sync_lock = false;
  network_menu = NULL;

  //Set nickname
#ifdef WIN32
  nickname = getenv("USERNAME");
#else
  nickname = getenv("USER");
#endif
}

//-----------------------------------------------------------------------------

int net_thread_func(void* no_param)
{
  network.ReceiveActions();
  network.Disconnect();
  return 1;
}

//-----------------------------------------------------------------------------

void Network::Init()
{
  if (sdlnet_inited) return;
  if (SDLNet_Init()) {
      Error(_("Failed to initialize network library!"));
  }
  sdlnet_inited = true;
  max_nb_players = GameMode::GetInstance()->max_teams;

#if defined(DEBUG) && not defined(WIN32)
  fin = open("./network.in", O_RDWR | O_CREAT | O_SYNC, S_IRWXU | S_IRWXG);
  fout = open("./network.out", O_RDWR | O_CREAT | O_SYNC, S_IRWXU | S_IRWXG);
#endif
}

//-----------------------------------------------------------------------------

Network::~Network()
{
  Disconnect();
  if (sdlnet_inited)
  {
    SDLNet_Quit();
#if defined(DEBUG) && not defined(WIN32)
    close(fin);
    close(fout);
#endif
  }
}

//-----------------------------------------------------------------------------

void Network::Disconnect()
{
  if(!IsConnected()) return;

  connection_side_t old_connection = m_connection;
  m_connection = LOCAL_ONLY; // To make the threads terminate

  SDL_WaitThread(thread,NULL);
  printf("Network thread finished\n");
  for(std::list<DistantComputer*>::iterator client = cpu.begin();
      client != cpu.end();
      client++)
  {
    delete *client;
  }
  cpu.clear();
  SDLNet_FreeSocketSet(socket_set);

  if(old_connection == NETWORK_SERVER)
    SDLNet_TCP_Close(server_socket);
}

//-----------------------------------------------------------------------------
//----------------       Action handling methods   ----------------------------
//-----------------------------------------------------------------------------
void Network::ReceiveActions()
{
  char* packet;

  while (IsConnected() && (cpu.size()==1 || IsServer()))
  {
    if (state == NETWORK_PLAYING && cpu.size() == 0)
    {
      // If while playing everybody disconnected, just quit
      break;
    }

    while (SDLNet_CheckSockets(socket_set, 100) == 0 && IsConnected()) //Loop while nothing is received
      if (IsServer() && server_socket)
    {
      // Check for an incoming connection
      TCPsocket incoming;
      incoming = SDLNet_TCP_Accept(server_socket);
      if (incoming)
      {
        cpu.push_back(new DistantComputer(incoming));
        printf("New client connected\n");
        if (GetNbConnectedPlayers() >= max_nb_players)
          RejectIncoming();
        ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_RULES_ASK_VERSION));
      }
      SDL_Delay(100);
    }

    std::list<DistantComputer*>::iterator dst_cpu = cpu.begin();
    while(dst_cpu != cpu.end() && IsConnected())
    {
      if((*dst_cpu)->SocketReady()) // Check if this socket contains data to receive
      {
        // Read the size of the packet
        int packet_size = (*dst_cpu)->ReceiveDatas(packet);
        if( packet_size <= 0)
        {
          dst_cpu = CloseConnection(dst_cpu);
          continue;
        }

#if defined(DEBUG) && not defined(WIN32)
	int tmp = 0xFFFFFFFF;
	write(fin, &packet_size, 4);
	write(fin, packet, packet_size);
	write(fin, &tmp, 4);
#endif

        Action* a = new Action(packet);
        MSG_DEBUG("network.traffic","Received action %s",
                ActionHandler::GetInstance()->GetActionName(a->GetType()).c_str());

	//Add relation between nickname and socket
	if( a->GetType() == Action::ACTION_NICKNAME){
	  std::string nickname = a->PopString();
	  std::cout<<"New nickname: " + nickname<< std::endl;
	  (*dst_cpu)->nickname = nickname;
	  delete a;
	  break;
	}

        if( a->GetType() == Action::ACTION_MENU_ADD_TEAM
        ||  a->GetType() == Action::ACTION_MENU_DEL_TEAM)
        {
          (*dst_cpu)->ManageTeam(a);
          delete a;
        }
        else
        if(a->GetType() == Action::ACTION_CHAT_MESSAGE)
        {
          (*dst_cpu)->SendChatMessage(a);
          delete a;
        }
        else
        {
          ActionHandler::GetInstance()->NewAction(a, false);
        }

        // Repeat the packet to other clients:
        if(a->GetType() != Action::ACTION_RULES_SEND_VERSION
        && a->GetType() != Action::ACTION_NETWORK_CHANGE_STATE
        && a->GetType() != Action::ACTION_CHAT_MESSAGE)
        for(std::list<DistantComputer*>::iterator client = cpu.begin();
            client != cpu.end();
            client++)
        if(client != dst_cpu)
        {
          (*client)->SendDatas(packet, packet_size);
        }
        free(packet);
      }
      dst_cpu++;
    }
  }
}

// Send Messages
void Network::SendAction(Action* a)
{
  if (!IsConnected()) return;

  MSG_DEBUG("network.traffic","Send action %s",
        ActionHandler::GetInstance()->GetActionName(a->GetType()).c_str());

  int size;
  char* packet;
  a->WritePacket(packet, size);
  
  assert(packet != NULL);
  SendPacket(packet, size);

  free(packet);
}

void Network::SendPacket(char* packet, int size)
{
#if defined(DEBUG) && not defined(WIN32)
	int tmp = 0xFFFFFFFF;
	write(fout, &size, 4);
	write(fout, packet, size);
	write(fout, &tmp, 4);
#endif
  for(std::list<DistantComputer*>::iterator client = cpu.begin();
      client != cpu.end();
      client++)
  {
    (*client)->SendDatas(packet, size);
  }
}

void Network::SendChatMessage(const std::string& txt)
{
  if (txt == "") return;

  if(IsServer())
  {
    ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_CHAT_MESSAGE, nickname + std::string("> ") + txt));
  }
  else
  {
    Action a(Action::ACTION_CHAT_MESSAGE, txt);
    network.SendAction(&a);
  }
}

//-----------------------------------------------------------------------------

const bool Network::IsConnected() const { return (m_connection != LOCAL_ONLY); }
const bool Network::IsLocal() const { return (m_connection == LOCAL_ONLY); }
const bool Network::IsServer() const { return (m_connection == NETWORK_SERVER); }
const bool Network::IsClient() const { return (m_connection == NETWORK_CLIENT); }

const uint Network::GetPort()
{
  Uint16 prt;
  prt = SDLNet_Read16(&ip.port);
  return (uint)prt;
}

//-----------------------------------------------------------------------------
