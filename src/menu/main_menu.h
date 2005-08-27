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
#include <vector>
#include <ClanLib/core.h>
#include <ClanLib/display.h>
//-----------------------------------------------------------------------------

typedef enum
{
  menuNULL=0,
  menuJOUER,
  menuOPTIONS,
  menuINFOS,
  menuQUITTER
} menu_item;

//-----------------------------------------------------------------------------

class Menu
{
private:
  CL_ResourceManager *pictures;
  CL_Surface background;

  ButtonText jouer, network, options, infos, quitter;
  typedef struct touche_menu
  {
    int touche;
    menu_item item;
    touche_menu(int t, menu_item i) { touche = t; item = i; }
  } touche_menu;
  std::vector<touche_menu> touches;
  CL_Slot keyboard_slot, mouse_slot, slot_quit;
  menu_item choix;
  volatile bool fin_boucle;

public:
  Menu();
  void ChargeImage();
  menu_item Lance ();

private:
  void TraiteTouche (const CL_InputEvent &event);
  void TraiteClic (const CL_InputEvent &event);
  menu_item Boucle ();
  void SignalWM_QUIT ();
};

extern Menu menu;

//-----------------------------------------------------------------------------
#endif
