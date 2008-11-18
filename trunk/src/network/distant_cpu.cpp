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
#include "include/constant.h"
#include "map/maps_list.h"
#include "menu/network_menu.h"
#include "team/team.h"
#include "team/team_config.h"
#include "team/teams_list.h"
#include "tool/debug.h"
//-----------------------------------------------------------------------------

static const int MAX_PACKET_SIZE = 250*1024;

DistantComputer::DistantComputer(WSocket* new_sock) :
  sock(new_sock),
  state(DistantComputer::STATE_ERROR),
  force_disconnect(false),
  nickname("this is not initialized"),
  owned_teams()
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
    for(TeamsList::iterator team = GetTeamsList().playing_list.begin();
      team != GetTeamsList().playing_list.end();
      ++team) {
      Action b(Action::ACTION_MENU_ADD_TEAM, (*team)->GetId());
      b.Push((*team)->GetPlayerName());
      b.Push((int)(*team)->GetNbCharacters());
      b.WriteToPacket(pack, size);
      SendDatas(pack, size);
      free(pack);
    }
  }
}

DistantComputer::~DistantComputer()
{
  ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_INFO_CLIENT_DISCONNECT, GetAddress()));

  if (Network::GetInstance()->IsConnected())
  {
    for (std::map<const std::string, ConfigTeam>::iterator team = owned_teams.begin();
         team != owned_teams.end();
         ++team)
    {
      ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_MENU_DEL_TEAM, team->first));
    }
  }
  owned_teams.clear();

  delete sock;
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

void DistantComputer::SetNickname(const std::string& _nickname)
{
  nickname = _nickname;
}

const std::string& DistantComputer::GetNickname() const
{
  return nickname;
}

bool DistantComputer::AddTeam(const ConfigTeam& team_conf)
{
  std::pair<std::map<const std::string, ConfigTeam>::iterator, bool> r;
  r = owned_teams.insert(std::make_pair(team_conf.id, team_conf));
  if (!r.second) {
    ForceDisconnection();

    ASSERT(false);
    return false;
  }
  return r.second;
}

bool DistantComputer::RemoveTeam(const std::string& team_id)
{
  size_t size, previous_size;
  previous_size = owned_teams.size();
  size = owned_teams.erase(team_id);

  if (size == previous_size) {
    ForceDisconnection();

    ASSERT(false);
    return false;
  }

  return true;
}

bool DistantComputer::UpdateTeam(const std::string& old_team_id, const ConfigTeam& team_conf)
{
  if (old_team_id == team_conf.id) {

    if (owned_teams.find(team_conf.id) == owned_teams.end()) {
      ASSERT(false);
      return false;
    }

    owned_teams[team_conf.id] = team_conf;
    return true;
  }

  if (!RemoveTeam(old_team_id))
    return false;

  if (!AddTeam(team_conf))
    return false;

  return true;
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
