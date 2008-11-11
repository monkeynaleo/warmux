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
#include "team/teams_list.h"
#include "tool/debug.h"
//-----------------------------------------------------------------------------

static const int MAX_PACKET_SIZE = 250*1024;

DistantComputer::DistantComputer(WSocket* new_sock) :
  sock(new_sock),
  owned_teams(),
  state(DistantComputer::STATE_ERROR),
  nickname("this is not initialized"),
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
    for (std::list<std::string>::iterator team = owned_teams.begin();
         team != owned_teams.end();
         ++team)
    {
      ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_MENU_DEL_TEAM, *team));
    }
  }
  owned_teams.clear();

  delete sock;
}

bool DistantComputer::SocketReady() const
{
  return sock->IsReady();
}

int DistantComputer::ReceiveDatas(char* & buf)
{
  bool r;
  int ret;

  sock->Lock();
  MSG_DEBUG("network", "locked");

  int packet_size;
  char* packet;

  // Firstly, we read the size of the incoming packet
  r = sock->ReceiveInt_NoLock(packet_size);
  if (!r) {
    ret = -1;
    goto out_unlock;
  }

  if (packet_size > MAX_PACKET_SIZE) {
    MSG_DEBUG("network", "packet is too big");
    ret = -1;
    goto out_unlock;
  }

  packet = (char*)malloc(packet_size);
  if (!packet) {
    MSG_DEBUG("network", "memory allocated failed");
    ret = -1;
    goto out_unlock;
  }

  r = sock->ReceiveBuffer_NoLock(packet, packet_size);
  if (!r) {
    free(packet);
    packet = NULL;
    ret = -1;
    goto out_unlock;
  }

  buf = packet;
  ret = packet_size;

 out_unlock:
  sock->UnLock();
  MSG_DEBUG("network", "unlocked");
  return ret;
}

bool DistantComputer::SendDatas(char* packet, int size)
{
  bool r;
  sock->Lock();
  MSG_DEBUG("network", "locked");

  r = sock->SendInt_NoLock(size);
  if (!r)
    goto out_unlock;

  r = sock->SendBuffer_NoLock(packet, size);
  if (!r)
    goto out_unlock;

  MSG_DEBUG("network", "%i sent", 4 + size);

 out_unlock:
  sock->UnLock();
  MSG_DEBUG("network", "unlocked");
  return r;
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

bool DistantComputer::AddTeam(const std::string& team_id)
{
  int index = 0;
  Team * the_team = GetTeamsList().FindById(team_id, index);

  if (the_team) {
    owned_teams.push_back(team_id);
    return true;
  }

  ForceDisconnection();

  std::cerr << "Team "<< team_id << "does not exist!" << std::endl;
  ASSERT(false);
  return false;
}

bool DistantComputer::RemoveTeam(const std::string& team_id)
{
  std::list<std::string>::iterator it;
  it = find(owned_teams.begin(), owned_teams.end(), team_id);
  printf("size of owned teams: %d\n", (int)owned_teams.size());

  if (it != owned_teams.end()) {
    owned_teams.erase(it);
    return true;
  }

  ForceDisconnection();

  ASSERT(false);
  return false;
}

bool DistantComputer::UpdateTeam(const std::string& old_team_id, const std::string& team_id)
{
  if (old_team_id == team_id) // nothing to do !
    return true;

  if (!RemoveTeam(old_team_id))
    return false;

  if (!AddTeam(team_id))
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
