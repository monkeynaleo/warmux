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
 * Graphical interface showing various information about the game.
 *****************************************************************************/

#ifndef INTERFACE_H
#define INTERFACE_H
#include <vector>
#include "weapon_menu.h"
#include "../graphic/surface.h"
#include "../graphic/sprite.h"
#include "../gui/progress_bar.h"
#include "../include/base.h"
#include "../character/character.h"
#include "../team/team.h"

#ifdef WIN32
#undef interface
#endif

class Interface
{

public:
  Character *character_under_cursor;
  Weapon* weapon_under_cursor;
  WeaponsMenu weapons_menu;
  Team * tmp_team;

 private:
   // Timers
   Text * global_timer;
   Text * timer;

   // Character information
   Text * t_character_name;
   Text * t_team_name;
   Text * t_player_name;

   Text * t_character_energy;

   // Weapon information
   Text * t_weapon_name;
   Text * t_weapon_stock;

   bool display;
   int start_hide_display;
   bool display_timer;
   EnergyBar energy_bar;
   BarreProg wind_bar;

   Surface game_menu;
   Surface clock_background;
   Surface clock;
   Surface wind;
   Surface wind_indicator;
   Point2i bottom_bar_pos;

   static Interface * singleton;

 private:
   Interface();
   ~Interface();

 public:
   static Interface * GetInstance();
   WeaponsMenu & GetWeaponsMenu() { return weapons_menu; };

   void Reset();
   void Draw();

   void DrawCharacterInfo();
   void DrawTeamEnergy();
   void DrawWeaponInfo();
   void DrawWindInfo();
   void DrawClock();

   bool IsDisplayed () const { return display; };
   void EnableDisplay(bool _display);
   void Show();
   void Hide();
   bool IsVisible() const;

   int GetWidth() const;
   int GetHeight() const;
   int GetMenuHeight() const;
   Point2i GetSize() const;

   void UpdateTimer(uint utimer);
   void UpdateWindIndicator(int wind_value);
   void EnableDisplayTimer (bool _display) {display_timer = _display;};
};

void AbsoluteDraw(Surface& s, Point2i pos);
void HideGameInterface();
void ShowGameInterface();

#endif
