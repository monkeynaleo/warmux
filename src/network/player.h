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

#ifndef NET_PLAYER_H
#define NET_PLAYER_H
//-----------------------------------------------------------------------------
#include <SDL_net.h>
#include <map>
#include <string>
#include "include/base.h"
#include <WORMUX_team_config.h>
//-----------------------------------------------------------------------------

class ConfigTeam;

class Player
{
private:
  std::string nickname;
  std::map<const std::string, ConfigTeam> owned_teams;

public:
  Player();
  ~Player();
  void Disconnect();

  void SetNickname(const std::string& nickname);
  const std::string& GetNickname() const;

  bool AddTeam(const ConfigTeam& team_conf);
  bool RemoveTeam(const std::string& team_id);
  bool UpdateTeam(const std::string& old_team_id, const ConfigTeam& team_conf);

  uint GetNbTeams() const;
  const std::map<const std::string, ConfigTeam>& GetTeams() const;
};

#endif

