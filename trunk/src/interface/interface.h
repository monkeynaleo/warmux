/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
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
#include <WORMUX_base.h>
#include <WORMUX_singleton.h>
#include "weapon_menu.h"
#include "graphic/colors.h"
#include "gui/energy_bar.h"
#include "gui/energy_bar.h"
#include "gui/progress_bar.h"
#include "weapon/weapon_strength_bar.h"

// Forward declarations
class Character;
class Team;
class Text;
class Polygon;
class Weapon;
class WeaponHelp;
class Profile;

#ifdef WIN32
#undef interface
#endif

class Interface : public Singleton<Interface>
{
public:
  Character *character_under_cursor;
  Weapon *weapon_under_cursor;
  WeaponsMenu weapons_menu;
  WeaponHelp *help;
  Team *tmp_team;

private:
  // Timers
  Text * global_timer;
  Text * timer;
  uint remaining_turn_time;

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
  int start_show_display;
  bool display_timer;
  bool display_minimap;
  EnergyBar * energy_bar;
  ProgressBar wind_bar;
  WeaponStrengthBar weapon_strength_bar;

  Surface game_menu;
  Surface small_interface;
  Sprite *clock, *clock_normal, *clock_emergency;
  int     clock_width;
  Point2i bottom_bar_pos;
  int last_width;
  float zoom;
  Surface shoot;

  //Minimap
  Surface *minimap;
  uint m_last_minimap_redraw;
  Point2i m_last_preview_size;
  //Styled box
  Surface *mask;
  Surface *scratch;

  Color m_camera_preview_color;
  Color m_playing_character_preview_color;
  Color m_text_color, m_energy_text_color;

  void FreeDrawElements();
  void DrawCharacterInfo();
  void DrawTeamEnergy() const;
  void DrawWeaponInfo() const;
  void DrawWindIndicator(const Point2i &wind_bar_pos) const;
  void DrawWindInfo() const;
  void DrawClock(const Point2i &time_pos) const;
  void DrawTimeInfo() const;
  void DrawMapPreview();
  void DrawSmallInterface() const;

  void LoadDataInternal(Profile *res);
  int GetWidth() const { return game_menu.GetWidth(); }
  int GetHeight() const;
  int GetMenuHeight() const;
  Point2i GetSize() const { return Point2i(GetWidth(), GetHeight()); }

protected:
  friend class Singleton<Interface>;
  Interface();
  ~Interface();

public:
  const WeaponsMenu &GetWeaponsMenu() const { return weapons_menu; }

  void Reset();
  void Draw();
  void LoadData();

  Point2i GetMenuPosition() const { return bottom_bar_pos; }
  bool IsDisplayed () const { return display; }
  void EnableDisplay(bool _display) { display = _display; }
  void Show();
  void Hide();

  void SetCurrentOverflyWeapon(Weapon * weapon) { weapon_under_cursor = weapon; }
  void UpdateTimer(uint utimer, bool emergency, bool reset_anim);
  void UpdateWindIndicator(int wind_value);
  void EnableDisplayTimer(bool _display) { display_timer = _display; }
  void ToggleMinimap() { display_minimap = !display_minimap; }
  void MinimapSizeDelta(int delta);
  bool ActionClickUp(const Point2i &mouse_pos);
  bool ActionLongClick(const Point2i &mouse_pos, const Point2i &old_mouse_pos);
  bool ActionClickDown(const Point2i &mouse_pos);
  bool Intersect(const Point2i &mouse_pos);
};

void AbsoluteDraw(const Surface& s, const Point2i& pos);
void HideGameInterface();
inline void ShowGameInterface() { Interface::GetInstance()->Show(); }

#endif
