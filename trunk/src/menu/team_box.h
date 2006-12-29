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
 *  Team box
 *****************************************************************************/

#ifndef TEAM_BOX_H
#define TEAM_BOX_H

#include "../gui/box.h"
#include "../gui/label.h"
#include "../gui/picture_widget.h"
#include "../gui/spin_button.h"
#include "../gui/text_box.h"

class Team;

class TeamBox : public HBox
{
 private:
  bool is_local; // local/remote team

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

  bool IsLocal() const;

  void Update(const Point2i &mousePosition,
	      const Point2i &lastMousePosition,
	      Surface& surf);
  Widget* Clic(const Point2i &mousePosition, uint button);
};

#endif
