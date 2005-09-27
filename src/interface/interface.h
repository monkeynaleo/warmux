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
 * Interface affichant différentes informations sur la jeu.
 *****************************************************************************/

#ifndef INTERFACE_H
#define INTERFACE_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "../team/character.h"
#include "../team/team.h"
#include "../gui/progress_bar.h"
#include "weapon_menu.h"
#include <vector>
//-----------------------------------------------------------------------------

#ifdef WIN32
#undef interface
#endif

struct SDL_Surface;

class Interface
{
public:
  Character *ver_pointe_souris;
  Weapon* arme_pointe_souris;
  int chrono; // affiché uniquement si sa valeur est positive
  WeaponsMenu weapons_menu;
#ifdef CL
   CL_Surface weapon_box_button ;
#else
   SDL_Surface *weapon_box_button;
#endif
private:
  bool affiche;
  void AfficheInfoVer (Character &ver);
  void AfficheInfoArme ();
  BarreProg barre_energie;
#ifdef CL
  CL_Surface game_menu;
  CL_Surface bg_time;
#else
  SDL_Surface *game_menu;
  SDL_Surface *bg_time;
  int bottom_bar_ox;
  int bottom_bar_oy;
#endif
public:
  Interface();
  void Init();
  void Reset();
  void Draw();

  bool EstAffiche () const { return affiche; }
  void ChangeAffiche (bool affiche);

  uint GetWidth() const;
  uint GetHeight() const;

};

extern Interface interface;

void AbsoluteDraw(SDL_Surface* s, int x, int y);

//-----------------------------------------------------------------------------
#endif
