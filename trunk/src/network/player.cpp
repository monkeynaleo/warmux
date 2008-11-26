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
 * A network player (local or remote)
 *****************************************************************************/

#include "network/player.h"
//-----------------------------------------------------------------------------
#include "network/network.h"
#include "include/action.h"
#include "include/action_handler.h"
#include "menu/network_menu.h"
#include <WORMUX_team_config.h>
//-----------------------------------------------------------------------------

Player::Player() : nickname("unknown")
{
}

Player::~Player()
{
  Disconnect();
}

void Player::Disconnect()
{
  if (Network::GetInstance()->IsConnected()) {

    for (std::map<const std::string, ConfigTeam>::iterator team = owned_teams.begin();
         team != owned_teams.end();
         ++team) {
      ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_GAME_DEL_TEAM, team->first));
    }
  }
}

void Player::SetNickname(const std::string& _nickname)
{
  nickname = _nickname;
}

const std::string& Player::GetNickname() const
{
  return nickname;
}

bool Player::AddTeam(const ConfigTeam& team_conf)
{
  printf("Player %p :: AddTeam %s\n", this, team_conf.id.c_str());

  std::pair<std::map<const std::string, ConfigTeam>::iterator, bool> r;
  r = owned_teams.insert(std::make_pair(team_conf.id, team_conf));
  if (!r.second) {
    ASSERT(false);
    return false;
  }
  return r.second;
}

bool Player::RemoveTeam(const std::string& team_id)
{
  printf("Player %p :: RemoveTeam %s\n", this, team_id.c_str());

  size_t previous_size;
  previous_size = owned_teams.size();
  owned_teams.erase(team_id);

  if (previous_size == owned_teams.size()) {
    ASSERT(false);
    return false;
  }

  return true;
}

bool Player::UpdateTeam(const std::string& old_team_id, const ConfigTeam& team_conf)
{
  printf("Player %p :: UpdateTeam %s - %s\n", this, old_team_id.c_str(), team_conf.id.c_str());

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

uint Player::GetNbTeams() const
{
  return owned_teams.size();
}

const std::map<const std::string, ConfigTeam>& Player::GetTeams() const
{
  return owned_teams;
}
