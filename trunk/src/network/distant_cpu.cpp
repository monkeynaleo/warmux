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
 * Distant Computer handling
 *****************************************************************************/

#include "distant_cpu.h"
//-----------------------------------------------------------------------------
#include <SDL_net.h>
#include <SDL_thread.h>
#include "network.h"
#include "../include/action_handler.h"
#include "../map/maps_list.h"
#include "../menu/network_menu.h"
#include "../team/teams_list.h"
#include "../tool/debug.h"
#include "../tool/i18n.h"
//-----------------------------------------------------------------------------

DistantComputer::DistantComputer(TCPsocket new_sock)
  : sock(new_sock)
{
  sock_lock = SDL_CreateMutex();

  SDLNet_TCP_AddSocket(network.socket_set, sock);

  // If we are the server, we have to tell this new computer
  // what teams / maps have alreayd been selected
  if( network.IsServer() )
  {
    Action a(ACTION_SET_MAP, ActiveMap().name);
    int size;
    char* pack;
    a.WritePacket(pack, size);
    SendDatas(pack, size);
    free(pack);

    // Teams infos of already connected computers
    for(TeamsList::iterator team = teams_list.playing_list.begin();
      team != teams_list.playing_list.end();
      ++team)
    {
      Action b(ACTION_NEW_TEAM, (*team)->GetId());
      b.WritePacket(pack, size);
      SendDatas(pack, size);
      free(pack);
    }
  }

  if(network.network_menu != NULL)
  {
    // Display a message in the network menu
    network.network_menu->msg_box->NewMessage( GetAdress() + _(" has joined the party"));
  }
}

DistantComputer::~DistantComputer()
{
  // Display a message in the network menu
  network.network_menu->msg_box->NewMessage( GetAdress() + _(" has left the party"));

  SDLNet_TCP_DelSocket(network.socket_set, sock);
  SDLNet_TCP_Close(sock);

  if(network.IsServer())
  {
    for(std::list<std::string>::iterator team = owned_teams.begin();
      team != owned_teams.end();
      ++team)
    {
      ActionHandler::GetInstance()->NewAction(new Action(ACTION_DEL_TEAM, *team));
    }
  }
  owned_teams.clear();

  SDL_DestroyMutex(sock_lock);
}

bool DistantComputer::SocketReady()
{
  return SDLNet_SocketReady(sock);
}

int DistantComputer::ReceiveDatas(char* & buf)
{
  SDL_LockMutex(sock_lock);
MSG_DEBUG("network","locked");

  // Firstly, we read the size of the incoming packet
  Uint32 net_size;
  if(SDLNet_TCP_Recv(sock, &net_size, 4) <= 0)
    return -1;

  int size = (int)SDLNet_Read32(&net_size);
  assert(size > 0);

  // Now we read the packet
  buf = (char*)malloc(size);

  int total_received = 0;
  while(total_received != size)
  {
    int received = SDLNet_TCP_Recv(sock, buf + total_received, size - total_received);
    if(received > 0)	
    {
      MSG_DEBUG("network", "%i received", received);
      total_received += received;
    }

    if(received < 0)
    {
      assert(false);
      std::cerr << "Malformed packet" << std::endl;
      total_received = received;
      break;
    }
  }
  SDL_UnlockMutex(sock_lock);
MSG_DEBUG("network","unlocked");
  return total_received;
}

void DistantComputer::SendDatas(char* packet, int size_tmp)
{
  SDL_LockMutex(sock_lock);
MSG_DEBUG("network","locked");
  Uint32 size;
  SDLNet_Write32(size_tmp, &size);
  SDLNet_TCP_Send(sock,&size,4);
  SDLNet_TCP_Send(sock,packet, size_tmp);
  MSG_DEBUG("network","%i sent", 4 + size_tmp);

  SDL_UnlockMutex(sock_lock);
MSG_DEBUG("network","unlocked");
}

std::string DistantComputer::GetAdress()
{
  IPaddress* ip = SDLNet_TCP_GetPeerAddress(sock);
  std::string address;
  if(SDLNet_ResolveIP(ip))
    address = SDLNet_ResolveIP(ip);
  else
    return "Unresolved address";
  return address;
}

void DistantComputer::ManageTeam(Action* team)
{
  std::string name = team->PopString();
  if(team->GetType() == ACTION_NEW_TEAM)
  {
    owned_teams.push_back(name);
  }
  else
  if(team->GetType() == ACTION_DEL_TEAM)
  {
    std::list<std::string>::iterator it;
    it = find(owned_teams.begin(), owned_teams.end(), name);
    assert(it != owned_teams.end());
    owned_teams.erase(it);
  }
  else
    assert(false);

  ActionHandler::GetInstance()->NewAction(new Action(team->GetType(), name), false);
}

void DistantComputer::SendChatMessage(Action* a)
{
  std::string txt = a->PopString();
  if(network.IsServer())
  {
    ActionHandler::GetInstance()->NewAction(new Action(ACTION_CHAT_MESSAGE, GetAdress() + ">"+txt));
  }
  else
  {
    ActionHandler::GetInstance()->NewAction(new Action(ACTION_CHAT_MESSAGE, txt), false);
  }
}
