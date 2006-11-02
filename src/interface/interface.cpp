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

#include "interface.h"
#include "mouse.h"
#include <iostream>
#include <SDL.h>
#include <sstream>
#include "../game/game_loop.h"
#include "../game/game_mode.h"
#include "../game/time.h"
#include "../graphic/colors.h"
#include "../graphic/video.h"
#include "../graphic/font.h"
#include "../include/app.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "../team/teams_list.h"
#include "../tool/debug.h"
#include "../tool/string_tools.h"
#include "../tool/i18n.h"
#include "../tool/resource_manager.h"
#include "../weapon/weapons_list.h"

WeaponStrengthBar weapon_strength_bar;

const Point2i WORM_NAME_POS(32, 28);
const Point2i WORM_ENERGY_POS = WORM_NAME_POS + Point2i(0, 20);
const Point2i WEAPON_NAME_POS(508, WORM_NAME_POS.y);
const Point2i AMMOS_POS(WEAPON_NAME_POS.x, WORM_ENERGY_POS.y);
const Point2i BORDER_POSITION(5, 5);
const Point2i WEAPON_ICON_POS(450, 20);

const uint INFO_VER_X2 = 296;

const Point2i ENERGY_BAR_POS(170, WORM_ENERGY_POS.y + 2);
const uint BARENERGIE_LARG = INFO_VER_X2 - ENERGY_BAR_POS.x;
const uint BARENERGIE_HAUT = 15;

const uint MARGIN = 10;

Interface * Interface::singleton = NULL;

Interface * Interface::GetInstance()
{
  if (singleton == NULL) {
    singleton = new Interface();
  }
  return singleton;
}

Interface::Interface()
{
  display = true;
  start_hide_display = 0;

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false);
  game_menu = resource_manager.LoadImage( res, "interface/background_interface");
  clock_background = resource_manager.LoadImage( res, "interface/clock_background");
  weapon_box_button = resource_manager.LoadImage( res, "interface/weapon_box_button");
  clock = resource_manager.LoadImage( res, "interface/clock");
  wind = resource_manager.LoadImage( res, "interface/wind");
  wind_indicator = resource_manager.LoadImage( res, "interface/wind_indicator");
  resource_manager.UnLoadXMLProfile( res);

  // energy bar
  energy_bar.InitVal(0, 0, GameMode::GetInstance()->character.init_energy);
  energy_bar.InitPos(0, 0, 120, 15);

  // wind bar
  wind_bar.InitPos(0, 0, wind_indicator.GetWidth() - 4, wind_indicator.GetHeight() - 4);
  wind_bar.InitVal(0, -100, 100);
  wind_bar.border_color.SetColor(0, 0, 0, 0);
  wind_bar.background_color.SetColor(0, 0, 0, 0);
  wind_bar.value_color = c_red;
  wind_bar.SetReferenceValue (true, 0);

  // strength bar initialisation
  weapon_strength_bar.InitPos (0, 0, 400, 20);
  weapon_strength_bar.InitVal (0, 0, 100);

  weapon_strength_bar.SetValueColor(WeaponStrengthBarValue);
  weapon_strength_bar.SetBorderColor(WeaponStrengthBarBorder);
  weapon_strength_bar.SetBackgroundColor(WeaponStrengthBarBackground);

  // constant text initialisation
  Font * normal_font = Font::GetInstance(Font::FONT_NORMAL);
  Font * small_font = Font::GetInstance(Font::FONT_SMALL);

  global_timer = new Text(ulong2str(0), black_color, normal_font);
  timer = new Text(ulong2str(0), black_color, normal_font);

  t_character_name = new Text("None", black_color, small_font);
  t_team_name = new Text("None", black_color, small_font);
  t_player_name = new Text("None", black_color, small_font);
  t_character_energy = new Text("Dead", white_color, small_font);
  t_weapon_name = new Text("None", black_color, small_font);
  t_weapon_stock = new Text("0", black_color, small_font);
}

Interface::~Interface()
{
  if (global_timer) delete global_timer;
  if (timer) delete timer;
  if (t_character_name) delete t_character_name;
  if (t_team_name) delete t_team_name;
  if (t_player_name) delete t_player_name;
  if (t_character_energy) delete t_character_energy;
  if (t_weapon_name) delete t_weapon_name;
  if (t_weapon_stock) delete t_weapon_stock;
}

void Interface::Reset()
{
  character_under_cursor = NULL;
  weapon_under_cursor = NULL;
  weapons_menu.Reset();
  energy_bar.InitVal(0, 0, GameMode::GetInstance()->character.init_energy);
}

void Interface::DrawCharacterInfo()
{
  AppWormux * app = AppWormux::GetInstance();
  Point2i pos = (app->video.window.GetSize() - GetSize()) * Point2d(0.5, 1);
  // Get the character
  if (character_under_cursor == NULL) character_under_cursor = &ActiveCharacter();

  // Display energy bar
  Point2i energy_bar_offset = BORDER_POSITION + Point2i(MARGIN + character_under_cursor->GetTeam().flag.GetWidth(),
                                                        character_under_cursor->GetTeam().flag.GetHeight() / 2);
  energy_bar.DrawXY(bottom_bar_pos + energy_bar_offset);
  // Display team logo
  app->video.window.Blit(character_under_cursor->GetTeam().flag, bottom_bar_pos + BORDER_POSITION);
  // Display team name
  t_team_name->Set(character_under_cursor->GetTeam().GetName());
  Point2i team_name_offset = energy_bar_offset + Point2i(energy_bar.GetWidth() / 2, energy_bar.GetHeight() + t_team_name->GetHeight() / 2);
  t_team_name->DrawCenter(bottom_bar_pos + team_name_offset);
  // Display character's name
  t_character_name->Set(character_under_cursor->GetName());
  Point2i character_name_offset = energy_bar_offset + Point2i(energy_bar.GetWidth() / 2, -t_character_name->GetHeight() / 2);
  t_character_name->DrawCenter(bottom_bar_pos + character_name_offset);
  // Display player's name
  t_player_name->Set(_("general: ") + character_under_cursor->GetTeam().GetPlayerName());
  Point2i player_name_offset = energy_bar_offset + Point2i(energy_bar.GetWidth() / 2, t_team_name->GetHeight() + t_player_name->GetHeight() + MARGIN);
  t_player_name->DrawCenter(bottom_bar_pos + player_name_offset);
  // Display energy
  if (!character_under_cursor->IsDead()) {
    t_character_energy->Set(ulong2str(character_under_cursor->GetEnergy())+"%");
    energy_bar.Actu(character_under_cursor->GetEnergy());
  } else {
    t_character_energy->Set(_("(dead)"));
    energy_bar.Actu(0);
  }

  t_character_energy->DrawCenter(bottom_bar_pos + energy_bar_offset + energy_bar.GetSize()/2);
}

void Interface::DrawWeaponInfo()
{
  AppWormux * app = AppWormux::GetInstance();
  Weapon* weapon;
  int nbr_munition;

  // Get the weapon
  if(weapon_under_cursor==NULL) {
    weapon = &ActiveTeam().AccessWeapon();
    nbr_munition = ActiveTeam().ReadNbAmmos();
  } else {
    weapon = weapon_under_cursor;
    nbr_munition = ActiveTeam().ReadNbAmmos(weapon_under_cursor->GetName());
  }

  std::string tmp;
  // Draw weapon icon
  Point2i weapon_icon_offset = Point2i(game_menu.GetWidth() / 2 - clock_background.GetWidth(),game_menu.GetHeight() - weapon->icon.GetHeight());
  app->video.window.Blit(weapon->icon, bottom_bar_pos + weapon_icon_offset);
  // Draw weapon name
  t_weapon_name->Set(weapon->GetName());
  Point2i weapon_name_offset = Point2i(game_menu.GetWidth() / 2 - clock_background.GetWidth() / 2 - t_weapon_name->GetWidth() - MARGIN, 0);
  t_weapon_name->DrawTopLeft(bottom_bar_pos + weapon_name_offset);
  // Display number of ammo
  t_weapon_stock->Set(_("Stock:") + (nbr_munition ==  INFINITE_AMMO ? _("(unlimited)") : Format("%i", nbr_munition)));
  Point2i weapon_stock_offset = Point2i(game_menu.GetWidth() / 2 - clock_background.GetWidth() / 2 - t_weapon_stock->GetWidth() - MARGIN, t_weapon_name->GetHeight());
  t_weapon_stock->DrawTopLeft(bottom_bar_pos + weapon_stock_offset);
  // Display CURRENT weapon icon on top
  weapon = &ActiveTeam().AccessWeapon();
  if (weapon != NULL) weapon->DrawWeaponBox();
}

// display time left in a turn
void Interface::DrawClock()
{
  AppWormux * app = AppWormux::GetInstance();
  Point2i turn_time_pos = (app->video.window.GetSize() - clock_background.GetSize()) * Point2d(0.5, 1) + 
      Point2i(0, - game_menu.GetHeight() / 2 + clock_background.GetHeight() / 2);
  Rectanglei dr(turn_time_pos, clock_background.GetSize());
  // Draw background interface
  app->video.window.Blit(clock_background, turn_time_pos);
  world.ToRedrawOnScreen(dr);
  // Draw clock
  app->video.window.Blit(clock, turn_time_pos + clock_background.GetSize() / 2 - clock.GetSize() / 2);
  world.ToRedrawOnScreen(dr);
  // Draw global timer
  std::string tmp(Time::GetInstance()->GetString());
  global_timer->Set(tmp);
  global_timer->DrawCenter(turn_time_pos + clock_background.GetSize() / 2 + Point2i(0, clock_background.GetHeight()/3));
  // Draw turn time
  if (display_timer)
    timer->DrawCenter(turn_time_pos + clock_background.GetSize() / 2 - Point2i(0, clock_background.GetHeight()/3));
  world.ToRedrawOnScreen(dr);
}

// display wind info
void Interface::DrawWindInfo()
{
  AppWormux * app = AppWormux::GetInstance();
  Point2i wind_pos = app->video.window.GetSize() * Point2d(0.5, 1) + Point2i(clock_background.GetWidth() / 2 + MARGIN, -game_menu.GetHeight() + wind.GetHeight() / 2);
  Rectanglei dr(wind_pos, wind.GetSize());
  // draw wind icon
  app->video.window.Blit(wind, wind_pos);
  // draw wind indicator
  Point2i wind_bar_offset = Point2i(0, wind.GetHeight() - wind_indicator.GetHeight());
  wind_bar.DrawXY(wind_pos + wind_bar_offset + Point2i(2,2));
  app->video.window.Blit(wind_indicator, wind_pos + wind_bar_offset);
  world.ToRedrawOnScreen(dr);
}

void Interface::Draw()
{
  AppWormux * app = AppWormux::GetInstance();
  bottom_bar_pos = (app->video.window.GetSize() - GetSize()) * Point2d(0.5, 1);

  if ( GameLoop::GetInstance()->ReadState() == GameLoop::PLAYING && weapon_strength_bar.visible)
  {
    // Position on the screen
    Point2i barPos = (app->video.window.GetSize() - weapon_strength_bar.GetSize()) * Point2d(0.5, 1)
                     - Point2i(0, GetHeight() + 10);

    // Drawing on the screen
     weapon_strength_bar.DrawXY(barPos);
  }

  weapons_menu.Draw();

  DrawClock();

  // Display the background of both Character info and weapon info
  Rectanglei dr(bottom_bar_pos, game_menu.GetSize());
  app->video.window.Blit( game_menu, bottom_bar_pos);

  world.ToRedrawOnScreen(dr);

  // display wind, character and weapon info
  DrawWindInfo();
  DrawCharacterInfo();
  DrawWeaponInfo();
}

int Interface::GetWidth() const
{
  return game_menu.GetWidth();
}

int Interface::GetHeight() const
{
  if(!display) {
    int height = game_menu.GetHeight() - (Time::GetInstance()->Read() - start_hide_display)/3;
    return (height > 0 ? height : 0);
  }
  return game_menu.GetHeight();
}

Point2i Interface::GetSize() const
{
  return Point2i(GetWidth(), GetHeight());
}

void Interface::EnableDisplay(bool _display)
{
  display = _display;
  camera.CenterOnFollowedObject();
}

void Interface::Show()
{
  if(display) return;
  display = true;
}

void Interface::Hide()
{
  if(!display) return;
  display = false;
  start_hide_display = Time::GetInstance()->Read();
}

bool Interface::IsVisible() const
{
  return display;
}

void Interface::UpdateTimer(uint utimer)
{
  timer->Set(ulong2str(utimer));
}

void Interface::UpdateWindIndicator(int wind_value)
{
  wind_bar.Actu(wind_value);
}

void AbsoluteDraw(Surface &s, Point2i pos)
{
  Rectanglei rectSurface(pos, s.GetSize());

  if( !rectSurface.Intersect(camera) )
    return;

  world.ToRedrawOnMap(rectSurface);

  rectSurface.Clip( camera );

  Rectanglei rectSource(rectSurface.GetPosition() - pos, rectSurface.GetSize());
  Point2i ptDest = rectSurface.GetPosition() - camera.GetPosition();

  AppWormux::GetInstance()->video.window.Blit(s, rectSource, ptDest);
}

void HideGameInterface()
{
  if(Interface::GetInstance()->GetWeaponsMenu().IsDisplayed()) return;
  Mouse::GetInstance()->Hide();
  Interface::GetInstance()->Hide();
}

void ShowGameInterface()
{
  Mouse::GetInstance()->Show();
  Interface::GetInstance()->Show();
}
