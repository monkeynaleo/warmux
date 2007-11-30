/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
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
 *  Network teams selection box
 *****************************************************************************/

#ifndef NETWORK_TEAMS_SELECTION_BOX_H
#define NETWORK_TEAMS_SELECTION_BOX_H

#include <vector>
#include "gui/box.h"

class TeamBox;
class SpinButtonWithPicture;
class Team;

const uint NMAX_NB_TEAMS=4;

class NetworkTeamsSelectionBox : public HBox
{
 private:
  /* If you need this, implement it (correctly) */
  NetworkTeamsSelectionBox(const NetworkTeamsSelectionBox&);
  NetworkTeamsSelectionBox operator=(const NetworkTeamsSelectionBox&);
  /**********************************************/

  SpinButtonWithPicture *local_teams_nb;
  std::vector<TeamBox*> teams_selections;
  void SetNbLocalTeams(uint nb_teams, uint previous_nb);
  void AddLocalTeam(uint i);
  void RemoveLocalTeam(uint i);
  void SetLocalTeam(uint i, Team& team, bool remove_previous_team);
  void PrevTeam(uint i);
  void NextTeam(uint i, bool check_null_prev_team = true);

 public:
  NetworkTeamsSelectionBox(const Point2i &size);

  void ValidTeamsSelection();
  Widget* Click(const Point2i &mousePosition, uint button);
  Widget* ClickUp(const Point2i &mousePosition, uint button);

  void AddTeamCallback(const std::string& team_id);
  void UpdateTeamCallback(const std::string& team_id, bool remote=false);
  void DelTeamCallback(const std::string& team_id);

  void SetMaxNbLocalPlayers(uint nb);
};
#endif
