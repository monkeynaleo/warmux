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
 *  Teams selection box
 *****************************************************************************/

#ifndef TEAMS_SELECTION_BOX_H
#define TEAMS_SELECTION_BOX_H

#include "../gui/box.h"
#include "../gui/label.h"
#include "../gui/picture_widget.h"
#include "../gui/spin_button.h"
#include "../gui/spin_button_big.h"
#include "../gui/text_box.h"

class Team;

const uint MAX_NB_TEAMS=4;

class TeamBox : public HBox
{
 private:
  Team * associated_team;
  PictureWidget *team_logo;
  Label * team_name;
  TextBox * player_name;
  SpinButton * nb_characters;

 public:
  TeamBox(std::string player_name, const Rectanglei &rect);

  void SetTeam(Team& _team, bool read_team_values=false);
  void ClearTeam();
  Team* GetTeam() const;  
  void ValidOptions() const;

  void Update(const Point2i &mousePosition,
	      const Point2i &lastMousePosition,
	      Surface& surf);
  Widget* Clic(const Point2i &mousePosition, uint button);
};

// -----------------------------------------------
// -----------------------------------------------

class TeamsSelectionBox : public HBox
{ 
 private:  
  SpinButtonBig *teams_nb;
  TeamBox* teams_selections[MAX_NB_TEAMS];

  void SetNbTeams(uint nb_teams);
  void PrevTeam(int i);
  void NextTeam(int i);
 public:
  TeamsSelectionBox(const Rectanglei &rect);

  void ValidTeamsSelection();
  Widget* Clic(const Point2i &mousePosition, uint button);
};

#endif
