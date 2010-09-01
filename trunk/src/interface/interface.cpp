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

#include "include/action_handler.h"
#include "interface/interface.h"
#include "interface/mouse.h"
#include "character/character.h"
#include "game/game.h"
#include "game/game_mode.h"
#include "game/time.h"
#include "graphic/text.h"
#include "graphic/sprite.h"
#include "graphic/video.h"
#include "include/app.h"
#include "map/camera.h"
#include "map/map.h"
#include "team/macro.h"
#include "team/team.h"
#include "tool/resource_manager.h"
#include "tool/string_tools.h"
#include "weapon/weapon.h"
#include "weapon/weapon_strength_bar.h"

static const Point2i BORDER_POSITION(5, 5);

#define MARGIN  4

void Interface::LoadDataInternal(Profile *res)
{
  Surface tmp     = LOAD_RES_IMAGE("interface/background_interface");
  Double  zoom    = 1.0;

  clock_normal    = LOAD_RES_SPRITE("interface/clock_normal");
  clock_emergency = LOAD_RES_SPRITE("interface/clock_emergency");

  last_width = AppWormux::GetInstance()->video->window.GetWidth();
  if (last_width < tmp.GetWidth()+20) {
    zoom = last_width / Double(tmp.GetWidth()+20);
    game_menu = tmp.RotoZoom(0.0, zoom, zoom);
    shoot = LOAD_RES_IMAGE("interface/shoot").RotoZoom(0.0, zoom, zoom);
    wind_icon = LOAD_RES_IMAGE("interface/wind").RotoZoom(0.0, zoom, zoom);
    wind_indicator = LOAD_RES_IMAGE("interface/wind_indicator").RotoZoom(0.0, zoom, zoom);
    clock_normal->Scale(zoom, zoom);
    clock_emergency->Scale(zoom, zoom);
    clock_normal->EnableLastFrameCache();
    clock_emergency->EnableLastFrameCache();
    shoot->EnableLastFrameCache();
  }
  else {
    game_menu = tmp;
    shoot = LOAD_RES_IMAGE("interface/shoot");
    wind_icon = LOAD_RES_IMAGE("interface/wind");
    wind_indicator = LOAD_RES_IMAGE("interface/wind_indicator");
  }
  clock_width = 70*zoom;
  small_background_interface = LOAD_RES_IMAGE("interface/small_background_interface");

  // energy bar
  if (energy_bar)
    delete energy_bar;
  energy_bar = new EnergyBar(0, 0, 120*zoom, 15*zoom,
                             0, 0,
                             GameMode::GetInstance()->character.init_energy);

  // Timer
  if (global_timer)
    delete global_timer;
  global_timer = new Text("0", gray_color, Font::FONT_BIG*zoom, Font::FONT_BOLD, false);
}

void Interface::LoadData()
{
  Profile *res   = GetResourceManager().LoadXMLProfile("graphism.xml", false);
  LoadDataInternal(res);
  GetResourceManager().UnLoadXMLProfile(res);
}

Interface::Interface()
  : global_timer(NULL)
  , display(true)
  , start_hide_display(0)
  , start_show_display(0)
  , display_minimap(true)
  , energy_bar(NULL)
  , clock(NULL)
  , minimap(NULL)
  , m_last_minimap_redraw(0)
  , m_last_preview_size(0, 0)
  , mask(NULL)
  , scratch(NULL)
{
  Profile *res = GetResourceManager().LoadXMLProfile("graphism.xml", false);

  LoadDataInternal(res);

  // wind bar
  wind_bar.InitPos(0, 0, wind_indicator.GetWidth() - 4, wind_indicator.GetHeight() - 4);
  wind_bar.SetMinMaxValueColor(LOAD_RES_COLOR("interface/wind_color_min"),
                               LOAD_RES_COLOR("interface/wind_color_max"));
  wind_bar.InitVal(0, -100, 100);

  wind_bar.border_color.SetColor(0, 0, 0, 0);
  wind_bar.background_color.SetColor(0, 0, 0, 0);
  //wind_bar.value_color = c_red;

  wind_bar.SetReferenceValue (true, 0);

  // strength bar initialisation
  weapon_strength_bar.InitPos (0, 0, 300, 15);
  weapon_strength_bar.InitVal (0, 0, 100);

  weapon_strength_bar.SetValueColor(LOAD_RES_COLOR("interface/weapon_strength_bar_value"));
  weapon_strength_bar.SetBorderColor(LOAD_RES_COLOR("interface/weapon_strength_bar_border"));
  weapon_strength_bar.SetBackgroundColor(LOAD_RES_COLOR("interface/weapon_strength_bar_background"));

  Color text_color = LOAD_RES_COLOR("interface/text_color");
  Color energy_text_color = LOAD_RES_COLOR("interface/energy_text_color");

  m_camera_preview_color = LOAD_RES_COLOR("interface/camera_preview_color");

  m_playing_character_preview_color = LOAD_RES_COLOR("interface/playing_character_preview_color");

  timer = new Text("0", black_color, Font::FONT_MEDIUM, Font::FONT_BOLD, false);

  t_character_name = new Text("None", text_color, Font::FONT_SMALL, Font::FONT_BOLD, false);
  t_team_name = new Text("None", text_color, Font::FONT_SMALL, Font::FONT_BOLD, false);
  t_player_name = new Text("None", text_color, Font::FONT_SMALL, Font::FONT_BOLD, false);
  t_weapon_name = new Text("None", text_color, Font::FONT_SMALL, Font::FONT_BOLD, false);
  t_weapon_stock = new Text("0", text_color, Font::FONT_SMALL, Font::FONT_BOLD, false);
  t_character_energy = new Text("Dead", energy_text_color, Font::FONT_SMALL, Font::FONT_BOLD);

  GetResourceManager().UnLoadXMLProfile(res);
}

Interface::~Interface()
{
  if (clock_normal) delete clock_normal;
  if (clock_emergency) delete clock_emergency;
  if (global_timer) delete global_timer;
  if (timer) delete timer;
  if (t_character_name) delete t_character_name;
  if (t_team_name) delete t_team_name;
  if (t_player_name) delete t_player_name;
  if (t_character_energy) delete t_character_energy;
  if (t_weapon_name) delete t_weapon_name;
  if (t_weapon_stock) delete t_weapon_stock;

  if (minimap) delete minimap;
  if (mask) delete mask;
  if (scratch) delete scratch;

  if (energy_bar) delete energy_bar;
}

void Interface::Reset()
{
  delete minimap;
  minimap = NULL;
  m_last_minimap_redraw = 0;
  start_hide_display = 0;
  start_show_display = 0;
  character_under_cursor = NULL;
  weapon_under_cursor = NULL;
  weapons_menu.Reset();
  energy_bar->InitVal(0, 0, GameMode::GetInstance()->character.init_energy);
}

void Interface::DrawCharacterInfo()
{
  Surface& window = GetMainWindow();

  // Get the character
  if (!character_under_cursor)
    character_under_cursor = &ActiveCharacter();
  const Team &team = character_under_cursor->GetTeam();

  // Display energy bar
  Point2i energy_bar_offset = BORDER_POSITION + Point2i(MARGIN + team.GetFlag().GetWidth(),
                                                        team.GetFlag().GetHeight() / 2);
  energy_bar->DrawXY(bottom_bar_pos + energy_bar_offset);

  // Display team logo
  if (energy_bar->GetCurrentValue() == energy_bar->GetMinValue())
    window.Blit(team.GetDeathFlag(), bottom_bar_pos + BORDER_POSITION);
  else
    window.Blit(team.GetFlag(), bottom_bar_pos + BORDER_POSITION);

  // Display team name
  t_team_name->SetText(team.GetName());
  Point2i team_name_offset = energy_bar_offset + Point2i(energy_bar->GetWidth() / 2,
                                                         energy_bar->GetHeight() + t_team_name->GetHeight() / 2);
  t_team_name->DrawCenter(bottom_bar_pos + team_name_offset);

  // Display character's name
  t_character_name->SetText(character_under_cursor->GetName());
  Point2i character_name_offset = energy_bar_offset + Point2i((energy_bar->GetWidth() > t_character_name->GetWidth() ? energy_bar->GetWidth() : t_character_name->GetWidth()) / 2, -t_character_name->GetHeight() / 2);
  t_character_name->DrawCenter(bottom_bar_pos + character_name_offset);

  // Display player's name
  if (window.GetHeight() > 480) {
    t_player_name->SetText(_("Head commander") + std::string(": ") + team.GetPlayerName());
    Point2i player_name_offset = energy_bar_offset
      + Point2i(energy_bar->GetWidth() / 2, t_team_name->GetHeight() + t_player_name->GetHeight() + MARGIN);
    t_player_name->DrawCenter(bottom_bar_pos + player_name_offset);
  }

  // Display energy
  if (!character_under_cursor->IsDead()) {
    t_character_energy->SetText(uint2str(character_under_cursor->GetEnergy())+"%");
    energy_bar->Actu(character_under_cursor->GetEnergy());
  } else {
    t_character_energy->SetText(_("(dead)"));
    energy_bar->Actu(0);
  }

  t_character_energy->DrawCenter(bottom_bar_pos + energy_bar_offset + energy_bar->GetSize()/2);
}

void Interface::DrawWeaponInfo() const
{
  Weapon* weapon;
  int nbr_munition;
  Double icon_scale_factor = 0.75;

  // Get the weapon
  if (weapon_under_cursor==NULL) {
    weapon = &ActiveTeam().AccessWeapon();
    nbr_munition = ActiveTeam().ReadNbAmmos();
  } else {
    weapon = weapon_under_cursor;
    nbr_munition = ActiveTeam().ReadNbAmmos(weapon_under_cursor->GetType());
    icon_scale_factor = cos((Double)Time::GetInstance()->Read() / 1000 * PI) * (Double)0.9;
  }

  std::string tmp;

  // Draw weapon name
  int offset = (game_menu.GetWidth() - clock_width) / 2 - MARGIN - 0.75*48;
  t_weapon_name->SetText(weapon->GetName());
  t_weapon_name->DrawCenterTop(bottom_bar_pos + Point2i(offset, 0));

  // Display number of ammo
  t_weapon_stock->SetText(nbr_munition ==  INFINITE_AMMO ? _("(unlimited)")
                                                         : _("Stock:") + Format("%i", nbr_munition));
  t_weapon_stock->DrawCenterTop(bottom_bar_pos + Point2i(offset, t_weapon_name->GetHeight()));

  // Draw weapon icon
  Sprite& icon = weapon->GetIcon();
  icon.Scale(icon_scale_factor, 0.75);
  Point2i weapon_icon_offset(offset - icon.GetWidth()/2,
                             game_menu.GetHeight() - icon.GetHeight() - MARGIN);
  icon.DrawXY(bottom_bar_pos + weapon_icon_offset);

  // Draw shoot button
  GetMainWindow().Blit(shoot, bottom_bar_pos + Point2i(game_menu.GetWidth() - 2*MARGIN - shoot.GetWidth(),
                                                       (game_menu.GetHeight() - shoot.GetHeight())/2));
}

void Interface::DrawTimeInfo() const
{
  Surface& window = GetMainWindow();
  Point2i turn_time_pos((window.GetWidth() - clock_width)/2,
                        window.GetHeight()  - GetHeight());
  Rectanglei dr(turn_time_pos, Point2i(clock_width, game_menu.GetHeight()));

  // Draw background interface
  GetWorld().ToRedrawOnScreen(dr);
  DrawClock(turn_time_pos + clock->GetHeight());
}

// display time left in a turn
void Interface::DrawClock(const Point2i &time_pos) const
{
  // Draw turn time
  if (display_timer)
    timer->DrawCenter(time_pos - Point2i(0, game_menu.GetHeight()/3));

  // Draw clock
  Point2i tmp_point = time_pos - clock->GetSize() / 2;
  clock->Update();
  clock->DrawXY(tmp_point);

  // Draw global timer
  std::string tmp(Time::GetInstance()->GetString());
  global_timer->SetText(tmp);
  global_timer->DrawCenter(time_pos + Point2i(0, game_menu.GetHeight()/3));
}

// draw wind indicator
void Interface::DrawWindIndicator(const Point2i &wind_bar_pos, const bool draw_icon) const
{
  Surface& window = GetMainWindow();
  int height;

  // draw wind icon
  if (draw_icon) {
    window.Blit(wind_icon, wind_bar_pos);
    GetWorld().ToRedrawOnScreen(Rectanglei(wind_bar_pos, wind_icon.GetSize()));
    height = wind_icon.GetHeight() - wind_indicator.GetHeight();
  } else {
    height = MARGIN;
  }

  // draw wind indicator
  Point2i wind_bar_offset = Point2i(0, height);
  Point2i tmp = wind_bar_pos + wind_bar_offset + Point2i(2, 2);
  window.Blit(wind_indicator, wind_bar_pos + wind_bar_offset);
  wind_bar.DrawXY(tmp);
  GetWorld().ToRedrawOnScreen(Rectanglei(wind_bar_pos + wind_bar_offset,
                                         wind_indicator.GetSize()));
}

// display wind info
void Interface::DrawWindInfo() const
{
  Point2i wind_pos_offset = Point2i((game_menu.GetWidth() + clock_width) / 2 + MARGIN,
                                    (game_menu.GetHeight() - wind_icon.GetHeight()) / 2);
  DrawWindIndicator(bottom_bar_pos + wind_pos_offset, true);
}

// draw mini info when hidding interface
void Interface::DrawSmallInterface() const
{
  if (display)
    return;
  Surface& window = GetMainWindow();
  int height = ((int)Time::GetInstance()->Read() - start_hide_display - 1000) / 3 - 30;
  height = height > 0 ? height : 0;
  height = (height < small_background_interface.GetHeight()) ? height : small_background_interface.GetHeight();
  Point2i small_interface_position = Point2i((window.GetWidth() - small_background_interface.GetWidth()) / 2,
                                              window.GetHeight() - height);
  window.Blit(small_background_interface, small_interface_position);
  DrawWindIndicator(small_interface_position + 2*MARGIN, false);
  if (display_timer) {
    timer->DrawLeftTop(small_interface_position + Point2i(MARGIN * 4 + wind_bar.GetWidth(), 2*MARGIN+2));
  }
  GetWorld().ToRedrawOnScreen(Rectanglei(small_interface_position,small_background_interface.GetSize()));
}

// draw team energy
void Interface::DrawTeamEnergy() const
{
  Point2i team_bar_offset = Point2i((game_menu.GetWidth()+clock_width) / 2 + wind_icon.GetWidth(),
                                    (game_menu.GetHeight() - 50)/2);
  FOR_EACH_TEAM(tmp_team) {
    Team* team = *tmp_team;
    if (!display) // Fix bug #7753 (Team energy bar visible when the interface is hidden)
      team->GetEnergyBar().FinalizeMove();
    team->DrawEnergy(bottom_bar_pos + team_bar_offset);
  }
}

// Draw map preview
void Interface::DrawMapPreview()
{
  Surface   &window  = GetMainWindow();
  Point2i    offset(window.GetWidth() - GetWorld().ground.GetPreviewSize().x - 2*MARGIN,
                    2*MARGIN);
  Rectanglei rect_preview(offset, GetWorld().ground.GetPreviewSize());

  if (minimap == NULL ||
      GetWorld().ground.GetLastPreviewRedrawTime() > m_last_minimap_redraw ||
      GetWorld().water.GetLastPreviewRedrawTime() > m_last_minimap_redraw) {

    m_last_minimap_redraw = Time::GetInstance()->Read();
    const Point2i& preview_size = GetWorld().ground.GetPreviewSize();

    // Check whether the whole minimap must be updated
    if (m_last_preview_size != preview_size) {
      if (mask) {
        delete mask;
        mask = NULL;
      }
      if (minimap) {
        delete minimap;
        minimap = NULL;
      }
      if (scratch) {
        delete scratch;
        scratch = NULL;
      }
    }

    if (!minimap)
      minimap = new Surface(preview_size, SDL_HWSURFACE, true);

    // Recreate the scratch buffer
    if (!scratch)
      scratch = new Surface(preview_size, SDL_HWSURFACE, true);

    Point2i mergePos = -GetWorld().ground.GetPreviewRect().GetPosition();
    scratch->Blit(*GetWorld().ground.GetPreview(), mergePos);

    // Draw water
    if (GetWorld().water.IsActive()) {
      const Color * color = GetWorld().water.GetColor();
      ASSERT(color);

      // Scale water height according to preview size
      int y = GetWorld().GetSize().GetY() - GetWorld().water.GetSelfHeight();
      int h = GetWorld().ground.PreviewCoordinates(Point2i(0, y)).GetY();

      Surface water_surf(rect_preview.GetSize() - Point2i(0, h),
                         SDL_HWSURFACE, false);
      water_surf.SetAlpha(SDL_SRCALPHA|SDL_RLEACCEL, 200);
      water_surf.Fill(*color);

      // Draw box with color according to water type
      scratch->Blit(water_surf, Point2i(0, h));
    }

    //scratch->SetAlpha(SDL_SRCALPHA, 0);
    if (!mask) {
      m_last_preview_size = GetWorld().ground.GetPreviewSize();
      mask = new Surface(m_last_preview_size, SDL_HWSURFACE, true);

      GenerateStyledBorder(*mask, DecoratedBox::STYLE_ROUNDED);

      mask->SetAlpha(0, 0);
    }

    // Compose
    minimap->Blit(*mask);
    minimap->Blit(*scratch);
  }

  Rectanglei clip = rect_preview;
  SwapWindowClip(clip);
  window.Blit(*minimap, offset);

  Point2i coord;

  FOR_EACH_TEAM(team) {
    const Surface & icon = (*team)->GetMiniFlag();

    for (Team::iterator character = (*(team))->begin(), end_character = (*(team))->end();
         character != end_character;
         ++character) {

      if (character->IsDead()) {
        continue;
      }

      coord = GetWorld().ground.PreviewCoordinates(character->GetPosition()) + offset;
      window.Blit(icon, coord - icon.GetSize()/2);

      if (character->IsActiveCharacter()) {
        uint radius = (icon.GetSize().x < icon.GetSize().y) ? icon.GetSize().y : icon.GetSize().x;
        radius = (radius/2) + 1;
        window.CircleColor(coord.x, coord.y, radius, m_playing_character_preview_color);
        GetWorld().ToRedrawOnScreen(Rectanglei(coord.x-radius-1, coord.y-radius-1, 2*radius+2, 2*radius+2));
      } else {
        GetWorld().ToRedrawOnScreen(Rectanglei(coord - icon.GetSize()/2, icon.GetSize()));
      }

    }
  }

  const Camera* cam = Camera::GetConstInstance();
  Point2i TopLeft = GetWorld().ground.PreviewCoordinates(cam->GetPosition());
  Point2i BottomR = GetWorld().ground.PreviewCoordinates(cam->GetPosition()+cam->GetSize());

  GetMainWindow().RectangleColor(Rectanglei(TopLeft + offset, BottomR-TopLeft),
                                 m_camera_preview_color);
  SwapWindowClip(clip);

  GetWorld().ToRedrawOnScreen(rect_preview);
}

void Interface::Draw()
{
  Surface &window  = GetMainWindow();
  bottom_bar_pos = Point2i((window.GetWidth() - GetWidth())/2,
                           window.GetHeight() - GetHeight());

  // Has the display size changed? Then reload data
  if (last_width != window.GetWidth()) {
    LoadData();
  }

  if (display_minimap)
    DrawMapPreview();

  // Position on the screen
  Point2i barPos = (window.GetSize() - weapon_strength_bar.GetSize()) * Point2d(0.5, 1)
                 - Point2i(0, game_menu.GetHeight() + MARGIN);

  // Drawing on the screen
  weapon_strength_bar.DrawXY(barPos);

  weapons_menu.Draw();

  // Display the background of both Character info and weapon info
  Rectanglei dr(bottom_bar_pos, game_menu.GetSize());
  window.Blit(game_menu, bottom_bar_pos);

  GetWorld().ToRedrawOnScreen(dr);

  // display wind, character and weapon info
  DrawWindInfo();
  DrawTimeInfo();
  DrawCharacterInfo();
  DrawTeamEnergy();
  DrawWeaponInfo();
  DrawSmallInterface();
}

int Interface::GetHeight() const
{
  if (!display) {
    int height = GetMenuHeight() - ((int)Time::GetInstance()->Read() - start_hide_display)/3;
    height = (height > 0 ? height : 0);
    return (height < GetMenuHeight() ? height : GetMenuHeight());
  } else if (start_show_display != 0) {
    int height = ((int)Time::GetInstance()->Read() - start_show_display)/3;
    height = (height < GetMenuHeight() ? height : GetMenuHeight());
    return (height < GetMenuHeight() ? height : GetMenuHeight());
  }
  return GetMenuHeight();
}

int Interface::GetMenuHeight() const
{
  return game_menu.GetHeight() + MARGIN;
}

void Interface::Show()
{
  if (display) return;
  display = true;
  if (start_show_display + 1000 < (int)Time::GetInstance()->Read())
    start_show_display = Time::GetInstance()->Read();
  else
    start_show_display = Time::GetInstance()->Read() - (1000 - ((int)Time::GetInstance()->Read() - start_show_display));
}

void Interface::Hide()
{
  if (!display) return;
  display = false;
  if (start_hide_display + 1000 < (int)Time::GetInstance()->Read())
    start_hide_display = Time::GetInstance()->Read();
  else
    start_hide_display = Time::GetInstance()->Read() - (1000 - ((int)Time::GetInstance()->Read() - start_hide_display));
}

void Interface::UpdateTimer(uint utimer, bool emergency, bool reset_anim)
{
  Sprite *prev_clock = clock;

  if (emergency) {
    clock = clock_emergency;
    timer->SetColor(primary_red_color);
  } else {
    clock = clock_normal;
    timer->SetColor(black_color);
  }

  timer->SetText(uint2str(utimer));
  remaining_turn_time = utimer;

  if (prev_clock != clock || reset_anim) {

    if (clock == clock_normal) {
      uint frame_delay = utimer * 1000 / clock->GetFrameCount();
      clock->SetFrameSpeed(frame_delay);
    }

    clock->animation.SetLoopMode(true);
    clock->SetCurrentFrame(0);
    clock->Start();
  }
}

void Interface::UpdateWindIndicator(int wind_value)
{
  wind_bar.UpdateValue(wind_value);
};

void AbsoluteDraw(const Surface &s, const Point2i& pos)
{
  Rectanglei rectSurface(pos, s.GetSize());

  if (!rectSurface.Intersect(*Camera::GetInstance()))
    return;

  GetWorld().ToRedrawOnMap(rectSurface);

  rectSurface.Clip(*Camera::GetInstance());

  Rectanglei rectSource(rectSurface.GetPosition() - pos, rectSurface.GetSize());
  Point2i ptDest = rectSurface.GetPosition() - Camera::GetInstance()->GetPosition();

  GetMainWindow().Blit(s, rectSource, ptDest);
}

static void ActionShoot(bool on)
{
  if (Game::GetInstance()->ReadState() != Game::PLAYING ||
      !ActiveTeam().IsLocalHuman() ||
      ActiveCharacter().IsDead())
    return;

  if (on) {
    // Start loading
    Action *a = new Action(Action::ACTION_WEAPON_START_SHOOTING);
    ActionHandler::GetInstance()->NewAction(a);
  } else {
    Action *a = new Action(Action::ACTION_WEAPON_STOP_SHOOTING);
    ActionHandler::GetInstance()->NewAction(a);
  }
}

bool Interface::ActionClickDown(const Point2i &mouse_pos)
{
  Surface &  window  = GetMainWindow();

  if (display) {
    Rectanglei menu_button(Point2i(), game_menu.GetSize());
    if (menu_button.Contains(mouse_pos-bottom_bar_pos)) {
      // Positions are somewhat from Interface::DrawWeaponInfo()
      Point2i bottom_right((game_menu.GetWidth() - clock_width)>>1,
                           game_menu.GetHeight());
      Point2i top_left;

      // Action that should only happen when the player is human
      if (ActiveTeam().IsLocalHuman()) {
        // Weapon icon original width is 48, and has a default scale of 0.75
        top_left.SetValues((game_menu.GetWidth()- 36)/ 2 - clock_width, 0);
        Rectanglei weapon_button(top_left, -top_left+bottom_right);

        // Check if we clicked the weapon icon: toggle weapon menu
        if (weapon_button.Contains(mouse_pos-bottom_bar_pos) && ActiveTeam().IsLocalHuman()) {
          weapons_menu.SwitchDisplay();
          return true;
        }

        // Check if we clicked the shoot icon: start firing!
        Rectanglei shoot_button(game_menu.GetWidth() - 2*MARGIN - shoot.GetWidth(),
                                (game_menu.GetHeight() - shoot.GetHeight())/2,
                                shoot.GetWidth(), shoot.GetHeight());
        if (shoot_button.Contains(mouse_pos-bottom_bar_pos)) {
          ActionShoot(true);
          return true;
        }
      }

      // Check if we clicked the clock icon: display pause menu
      Rectanglei clock_button((game_menu.GetWidth() - clock_width)/ 2, 0,
                              clock_width, game_menu.GetHeight());
      if (clock_button.Contains(mouse_pos-bottom_bar_pos)) {
        Game::GetInstance()->UserAsksForMenu();
        return true;
      }

      // Check if we clicked the character icon: center on it
      Rectanglei character_button(0, 0,
                                  (game_menu.GetWidth()- 36)/ 2 - clock_width,
                                  game_menu.GetHeight());
      if (character_button.Contains(mouse_pos-bottom_bar_pos)) {
        Camera::GetInstance()->CenterOnActiveCharacter();
        return true;
      }

      // No actual button clicked, but still swallow that click
      return true;
    } else if (ActiveTeam().IsLocalHuman() && weapons_menu.ActionClic(mouse_pos)) {
      // Process click on weapon menu before minimap as it should be
      // overlayed on top of it.
      return true;
    }

  } else {
    // Mini-interface drawn, check if we clicked on it
    if (ActiveTeam().IsLocalHuman()) {
      Rectanglei small_button((window.GetWidth() - small_background_interface.GetWidth()) / 2,
                              window.GetHeight() - small_background_interface.GetHeight(),
                              small_background_interface.GetWidth(),
                              small_background_interface.GetHeight());
      if (small_button.Contains(mouse_pos)) {
        ActionShoot(true);
        return true;
      }
    }
  }

  if (display_minimap && // We are not targetting
      Mouse::GetInstance()->GetPointer() == Mouse::POINTER_SELECT) {
    Point2i    offset(window.GetWidth() - GetWorld().ground.GetPreviewSize().x - 2*MARGIN, 2*MARGIN);
    Rectanglei rect_preview(offset, GetWorld().ground.GetPreviewSize());
    if (rect_preview.Contains(mouse_pos)) {
      offset = GetWorld().ground.FromPreviewCoordinates(mouse_pos - offset);
      Camera::GetInstance()->SetXYabs(offset - Camera::GetInstance()->GetSize()/2);
      return true;
    }
  }

  return false;
}

bool Interface::ActionClickUp(const Point2i &mouse_pos)
{
  Surface &  window  = GetMainWindow();

  if (display) {
    Rectanglei menu_button(Point2i(), game_menu.GetSize());
    if (menu_button.Contains(mouse_pos-bottom_bar_pos)) {
      // Action that should only happen when the player is human
      if (ActiveTeam().IsLocalHuman()) {
        // Check if we clicked the shoot icon: release fire!
        Rectanglei shoot_button(game_menu.GetWidth() - 2*MARGIN - shoot.GetWidth(), 0,
                                shoot.GetWidth(), game_menu.GetHeight());
        if (shoot_button.Contains(mouse_pos-bottom_bar_pos)) {
          ActionShoot(false);
          return true;
        }
      }
    }
    // No button clicked, continue
  } else {
    // Mini-interface drawn, check if we clicked on it
    if (ActiveTeam().IsLocalHuman()) {
      Rectanglei small_button((window.GetWidth() - small_background_interface.GetWidth()) / 2,
                              window.GetHeight() - small_background_interface.GetHeight(),
                              small_background_interface.GetWidth(),
                              small_background_interface.GetHeight());
      if (small_button.Contains(mouse_pos)) {
        ActionShoot(false);
        return true;
      }
    }
  }

  return false;
}

void Interface::MinimapSizeDelta(int delta)
{
  GetWorld().ground.SetPreviewSizeDelta(delta);
}


void HideGameInterface()
{
  if (Interface::GetInstance()->GetWeaponsMenu().IsDisplayed()) return;
  Mouse::GetInstance()->Hide();
  Interface::GetInstance()->Hide();
}

void ShowGameInterface()
{
  Interface::GetInstance()->Show();
}
