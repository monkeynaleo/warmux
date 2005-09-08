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
#include <SDL/SDL.h>
#include <pgbutton.h>
#include <vector>
#include "../include/base.h"
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
  SDL_Surface* background;
  PG_Button *play, *network, *options, *infos, *quit;

public:
  menu_item choice;

  Main_Menu();
  void Init();
  void FreeMem();
  menu_item Run ();
};

//-----------------------------------------------------------------------------
extern Main_Menu main_menu;
//-----------------------------------------------------------------------------
#endif
