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
 * Interface affichant diff�rentes informations sur la jeu.
 *****************************************************************************/

#ifndef WEAPON_MENU_H
#define WEAPON_MENU_H
//-----------------------------------------------------------------------------
#include "../include/base.h"
#include "../team/character.h"
#include "../team/team.h"
#include "../weapon/weapon.h"
#include <vector>
#ifdef CL
# include <ClanLib/display.h>
#endif
//-----------------------------------------------------------------------------

struct Sprite;

class WeaponMenuItem
{
public:
  int x, y;
  double scale;
  Weapon* weapon;
#ifndef CL
  Sprite *weapon_icon;
#endif
  uint zoom_start_time;
  uint weapon_type;

private:
  bool zoom,dezoom;

public:
  WeaponMenuItem(uint num_sort);
  void Reset();

  void Draw();
  void ChangeZoom();

  bool MouseOn(int s_x, int s_y);

private:
  void ComputeScale();
};

class WeaponsMenu
{
public:
#ifdef CL
  CL_Surface my_button1 ;
  CL_Surface my_button2 ;
  CL_Surface my_button3 ;
  CL_Surface my_button4 ;
  CL_Surface my_button5 ;
#else
  Sprite *my_button1;
  Sprite *my_button2;
  Sprite *my_button3;
  Sprite *my_button4;
  Sprite *my_button5;
#endif
private:
  std::vector<WeaponMenuItem> boutons;
  typedef std::vector<WeaponMenuItem>::iterator iterator;
  typedef std::vector<WeaponMenuItem>::const_iterator const_iterator;

  bool display;
  bool show; // True during the motion to show the weapon menu.
  bool hide; // True during the motion to hide the weapon menu.

  uint motion_start_time;

  uint nbr_weapon_type; //nombre de type d'arme = nbr de colonnes
  uint max_weapon;  //nombre max d'arme dans les differents type = nbr de lignes

public:
  WeaponsMenu();

  // Renvoie true si un bouton a �t� cliqu�
  bool ActionClic (int x, int y);

  void Draw();
  void Init();

  int GetX() const;
  int GetY() const;
  int GetWidth() const;
  int GetHeight() const;
  bool IsDisplayed() const;

  void NewItem(Weapon* new_item, uint num_sort);
  void ChangeAffichage();

  void ReactionSouris(int x,int y);

private:
  void ComputeSize();
  void Show();
  void Hide();
  void DrawBouton(iterator bouton);
  void ShowMotion(int nr_bottons,int button_no,iterator it, int column);
  bool HideMotion(int nr_buttons,int button_no,iterator it,int column);
};

//-----------------------------------------------------------------------------
#endif
