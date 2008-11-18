/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
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

#include <algorithm>  //std::find
#include "network/distant_cpu.h"
//-----------------------------------------------------------------------------
#include <WORMUX_socket.h>
#include <SDL_thread.h>
#include "network/network.h"
#include "include/action.h"
#include "include/action_handler.h"
#include "map/maps_list.h"
#include "menu/network_menu.h"
#include "team/team_config.h"
#include "tool/debug.h"
//-----------------------------------------------------------------------------

static const int MAX_PACKET_SIZE = 250*1024;

DistantComputer::DistantComputer(WSocket* new_sock) :
  sock(new_sock),
  state(DistantComputer::STATE_ERROR),
  force_disconnect(false)
{
  // If we are the server, we have to tell this new computer
  // what teams / maps have already been selected
  if (Network::GetInstance()->IsServer()) {
    int size;
    char* pack;

    MSG_DEBUG("network", "Server: Sending map information");

    Action a(Action::ACTION_MENU_SET_MAP);
    MapsList::GetInstance()->FillActionMenuSetMap(a);
    a.WriteToPacket(pack, size);
    SendDatas(pack, size);
    free(pack);

    MSG_DEBUG("network", "Server: Sending teams information");

    // Teams infos of already connected computers
    std::list<DistantComputer*>::iterator it;
    std::map<std::string, ConfigTeam>::const_iterator team;

    for (it = Network::GetInstance()->cpu.begin();
        it != Network::GetInstance()->cpu.end();
        it++) {
      for (team = (*it)->GetPlayer().GetTeams().begin();
	   team != (*it)->GetPlayer().GetTeams().end();
	   team++) {

	Action b(Action::ACTION_MENU_ADD_TEAM, team->first);
	b.Push(team->second.player_name);
	b.Push(int(team->second.nb_characters));
	b.WriteToPacket(pack, size);
	SendDatas(pack, size);
	free(pack);
      }
    }

    for (team = Network::GetInstance()->GetPlayer().GetTeams().begin();
	 team != Network::GetInstance()->GetPlayer().GetTeams().end();
	 team++) {
      Action b(Action::ACTION_MENU_ADD_TEAM, team->first);
      b.Push(team->second.player_name);
      b.Push(int(team->second.nb_characters));
      b.WriteToPacket(pack, size);
      SendDatas(pack, size);
      free(pack);
    }
  }
}

DistantComputer::~DistantComputer()
{
  ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_INFO_CLIENT_DISCONNECT, GetAddress()));

  player.Disconnect();

  delete sock;
}

Player& DistantComputer::GetPlayer()
{
  return player;
}

bool DistantComputer::SocketReady() const
{
  return sock->IsReady();
}

bool DistantComputer::ReceiveDatas(void* & data, size_t & len)
{
  return sock->ReceivePacket(data, len);
}

bool DistantComputer::SendDatas(const void* data, size_t len)
{
  return sock->SendPacket(data, len);
}

std::string DistantComputer::GetAddress()
{
  return sock->GetAddress();
}

void DistantComputer::SetState(DistantComputer::state_t _state)
{
  state = _state;
}

DistantComputer::state_t DistantComputer::GetState() const
{
  return state;
}

void DistantComputer::ForceDisconnection()
{
  state = STATE_ERROR;
  force_disconnect = true;
}

bool DistantComputer::MustBeDisconnected()
{
  return force_disconnect;
}
