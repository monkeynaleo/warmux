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
 * Game menu
 *****************************************************************************/

#ifndef NETWORK_MENU_H
#define NETWORK_MENU_H

#include <list>
#include "menu.h"
#include "network_teams_selection_box.h"
#include "map_selection_box.h"
#include "../include/base.h"
#include "../graphic/font.h"
#include "../gui/button_text.h"
#include "../gui/msg_box.h"
#include "../gui/text_box.h"

class Team;

class NetworkMenu : public Menu
{
  /* Options controllers */
  SpinButton* player_number;
  Box* options_box;
  Label* connected_players;
  Label* inited_players;
  
  /* Chat controllers */
  MsgBox* msg_box;
  TextBox* line_to_send_tbox;
  Button* send_txt_bt;
  
  /* Team controllers */
  NetworkTeamsSelectionBox *team_box;
  
  /* Map controllers */
  MapSelectionBox *map_box;
  
  void SaveOptions();
  void OnClic(const Point2i &mousePosition, int button);
  void Draw(const Point2i &mousePosition);
  
  void sig_ok();
  void __sig_ok();
  void __sig_cancel();
  
public:
  NetworkMenu(); 
  ~NetworkMenu();
  
  void AddTeamCallback(std::string team_id);
  void UpdateTeamCallback(std::string team_id);
  void DelTeamCallback(std::string team_id);
  void ChangeMapCallback();
  
  void ReceiveMsgCallback(std::string msg);
};

#endif
