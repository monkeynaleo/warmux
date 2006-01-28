/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
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
 * Menu du jeu permettant de lancer une partie, modifier les options, d'obtenir
 * des informations, ou encore quitter le jeu.
 *****************************************************************************/

#ifndef MAIN_MENU_H
#define MAIN_MENU_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "../gui/button_text.h"
#include "../graphic/sprite.h"
#include <SDL.h>
#include <vector>
//-----------------------------------------------------------------------------

typedef enum
{
  menuNULL=0,
  menuPLAY,
  menuNETWORK,
  menuOPTIONS,
  menuQUIT
} menu_item;

//-----------------------------------------------------------------------------

class Main_Menu
{
private:
  Sprite *background, *skin_left, *skin_right, *title;
  ButtonText *play, *network, *options, *infos, *quit;
  Text * version_text, * website_text;
  uint start_time;
  uint last_refresh;
  uint anim_frame_count;
  uint button_height, button_width, title_offset, skin_offset;
  int title_y, skinl_y, skinr_y;
public:
  menu_item choice;

  Main_Menu();
  ~Main_Menu();
  menu_item Run ();
private:  
  void onClick ( int x, int y, int button);

  // Main drawing function: refresh parts of screen 
  void Draw (int mx, int my);

  //Draws gfx needing a refresh
  void DrawGfx(int mx, int my, uint dt);

  //Draw gfx
  void DrawTitle(uint dt);
  void DrawSkins(uint dt);
  void DrawButtons(int mx, int my, uint dt);

  // Erase gfx which have moved
  void EraseGfx(uint dt);

  // Erase the whole window
  void EraseAll();


  void button_click();
  bool sig_play();
  bool sig_network();
  bool sig_options();
  bool sig_infos();
  bool sig_quit();
};

//-----------------------------------------------------------------------------
#endif
