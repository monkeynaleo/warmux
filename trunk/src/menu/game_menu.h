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

#ifndef GAME_MENU_H
#define GAME_MENU_H

#include "menu.h"
#include "map_selection_box.h"
#include "teams_selection_box.h"
#include "../include/base.h"
#include "../graphic/font.h"

class GameMenu : public Menu
{
   /* Team controllers */
   TeamsSelectionBox * team_box;

   /* Map controllers */
   MapSelectionBox * map_box;

   /* Game options controllers */
   Box * game_options;
   SpinButtonWithPicture *opt_duration_turn;
   //SpinButtonWithPicture *opt_duration_end_turn;
   //SpinButtonBig *opt_nb_characters;
   SpinButtonWithPicture *opt_energy_ini;

   void SaveOptions();
   void OnClic(const Point2i &mousePosition, int button);
   void Draw(const Point2i &mousePosition);

   void __sig_ok();
   void __sig_cancel();

public:
   GameMenu(); 
   ~GameMenu();
};

#endif
