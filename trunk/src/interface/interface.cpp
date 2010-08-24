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

const Point2i BORDER_POSITION(5, 5);

const uint MARGIN = 4;

Interface::Interface()
  : display(true)
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
  int      width = AppWormux::GetInstance()->video->window.GetWidth();
  Profile *res   = GetResourceManager().LoadXMLProfile("graphism.xml", false);
  Surface  tmp   = GetResourceManager().LoadImage(res, "interface/background_interface");
  if (width < tmp.GetWidth()+20) {
    Double zoom = width / Double(tmp.GetWidth()+20);
    game_menu = tmp.RotoZoom(0.0, zoom, zoom);
  }
  else
    game_menu = tmp;
  small_background_interface = GetResourceManager().LoadImage(res, "interface/small_background_interface");
  clock_background = GetResourceManager().LoadImage(res, "interface/clock_background");
  clock_normal = GetResourceManager().LoadSprite(res, "interface/clock_normal");
  clock_emergency = GetResourceManager().LoadSprite(res, "interface/clock_emergency");
  wind_icon = GetResourceManager().LoadImage(res, "interface/wind");
  wind_indicator = GetResourceManager().LoadImage(res, "interface/wind_indicator");

  // styled box
  rounding_style[1][2] = GetResourceManager().LoadImage(res, "interface/rounding_bottom");
  rounding_style[0][2] = GetResourceManager().LoadImage(res, "interface/rounding_bottom_left");
  rounding_style[2][2] = GetResourceManager().LoadImage(res, "interface/rounding_bottom_right");
  rounding_style[1][0] = GetResourceManager().LoadImage(res, "interface/rounding_top");
  rounding_style[0][0] = GetResourceManager().LoadImage(res, "interface/rounding_top_left");
  rounding_style[2][0] = GetResourceManager().LoadImage(res, "interface/rounding_top_right");
  rounding_style[0][1] = GetResourceManager().LoadImage(res, "interface/rounding_left");
  rounding_style[2][1] = GetResourceManager().LoadImage(res, "interface/rounding_right");
  rounding_style[1][1] = GetResourceManager().LoadImage(res, "interface/rounding_center");
  for (int j=0; j<3; j++) {
    for (int i=0; i<3; i++) {
      rounding_style[j][i].SetAlpha(0, 0);
    }
  }

  // energy bar
  energy_bar = new EnergyBar(0, 0, 120, 15,
                             0, 0,
                             GameMode::GetInstance()->character.init_energy);

  // wind bar
  wind_bar.InitPos(0, 0, wind_indicator.GetWidth() - 4, wind_indicator.GetHeight() - 4);
  wind_bar.SetMinMaxValueColor(GetResourceManager().LoadColor(res, "interface/wind_color_min"),
                               GetResourceManager().LoadColor(res, "interface/wind_color_max"));
  wind_bar.InitVal(0, -100, 100);

  wind_bar.border_color.SetColor(0, 0, 0, 0);
  wind_bar.background_color.SetColor(0, 0, 0, 0);
  //wind_bar.value_color = c_red;

  wind_bar.SetReferenceValue (true, 0);

  // strength bar initialisation
  weapon_strength_bar.InitPos (0, 0, 300, 15);
  weapon_strength_bar.InitVal (0, 0, 100);

  weapon_strength_bar.SetValueColor(GetResourceManager().LoadColor(res, "interface/weapon_strength_bar_value"));
  weapon_strength_bar.SetBorderColor(GetResourceManager().LoadColor(res, "interface/weapon_strength_bar_border"));
  weapon_strength_bar.SetBackgroundColor(GetResourceManager().LoadColor(res, "interface/weapon_strength_bar_background"));

  Color text_color = GetResourceManager().LoadColor(res, "interface/text_color");
  Color energy_text_color = GetResourceManager().LoadColor(res, "interface/energy_text_color");

  m_camera_preview_color = GetResourceManager().LoadColor(res, "interface/camera_preview_color");

  m_playing_character_preview_color = GetResourceManager().LoadColor(res, "interface/playing_character_preview_color");


  // XXX Unused !?
  // Color turn_timer_text_color = GetResourceManager().LoadColor(res, "interface/turn_timer_text_color");
  // Color global_clock_text_color = GetResourceManager().LoadColor(res, "interface/global_clock_text_color");

  global_timer = new Text("0", gray_color, Font::FONT_BIG, Font::FONT_BOLD, false);
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
  AppWormux * app = AppWormux::GetInstance();
  // XXX Not used !?
  // Point2i pos = (app->video->window.GetSize() - GetSize()) * Point2d(0.5, 1);

  // Get the character
  if (character_under_cursor == NULL) character_under_cursor = &ActiveCharacter();

  // Display energy bar
  Point2i energy_bar_offset = BORDER_POSITION + Point2i(MARGIN + character_under_cursor->GetTeam().GetFlag().GetWidth(),
                                                        character_under_cursor->GetTeam().GetFlag().GetHeight() / 2);
  energy_bar->DrawXY(bottom_bar_pos + energy_bar_offset);

  // Display team logo
  if (energy_bar->GetCurrentValue() == energy_bar->GetMinValue())
    app->video->window.Blit(character_under_cursor->GetTeam().GetDeathFlag(), bottom_bar_pos + BORDER_POSITION);
  else
    app->video->window.Blit(character_under_cursor->GetTeam().GetFlag(), bottom_bar_pos + BORDER_POSITION);

  // Display team name
  t_team_name->SetText(character_under_cursor->GetTeam().GetName());
  Point2i team_name_offset = energy_bar_offset + Point2i(energy_bar->GetWidth() / 2, energy_bar->GetHeight() + t_team_name->GetHeight() / 2);
  t_team_name->DrawCenter(bottom_bar_pos + team_name_offset);

  // Display character's name
  t_character_name->SetText(character_under_cursor->GetName());
  Point2i character_name_offset = energy_bar_offset + Point2i((energy_bar->GetWidth() > t_character_name->GetWidth() ? energy_bar->GetWidth() : t_character_name->GetWidth()) / 2, -t_character_name->GetHeight() / 2);
  t_character_name->DrawCenter(bottom_bar_pos + character_name_offset);

  // Display player's name
  t_player_name->SetText(_("Head commander") + std::string(": ") + character_under_cursor->GetTeam().GetPlayerName());
  Point2i player_name_offset = energy_bar_offset + Point2i(energy_bar->GetWidth() / 2, t_team_name->GetHeight() + t_player_name->GetHeight() + MARGIN);
  t_player_name->DrawCenter(bottom_bar_pos + player_name_offset);

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
  int offset = (game_menu.GetWidth() - clock_background.GetWidth()) / 2;
  t_weapon_name->SetText(weapon->GetName());
  Point2i weapon_name_offset(offset - t_weapon_name->GetWidth() - MARGIN, 0);
  t_weapon_name->DrawLeftTop(bottom_bar_pos + weapon_name_offset);

  // Display number of ammo
  t_weapon_stock->SetText((nbr_munition ==  INFINITE_AMMO ? _("(unlimited)") : _("Stock:") + Format("%i", nbr_munition)));
  Point2i weapon_stock_offset(offset - t_weapon_stock->GetWidth() - MARGIN,
                              t_weapon_name->GetHeight());
  t_weapon_stock->DrawLeftTop(bottom_bar_pos + weapon_stock_offset);

  // Draw weapon icon
  Sprite& icon = weapon->GetIcon();
  icon.Scale(icon_scale_factor, 0.75);
  Point2i weapon_icon_offset = (game_menu.GetSize() - icon.GetSize()) / 2
                             + Point2i(- clock_background.GetWidth(), MARGIN);
  icon.DrawXY(bottom_bar_pos + weapon_icon_offset);
}

void Interface::DrawTimeInfo() const
{
  AppWormux * app = AppWormux::GetInstance();
  Point2i turn_time_pos = (app->video->window.GetSize() - clock_background.GetSize()) * Point2d(0.5, 1) +
      Point2i(0, - GetHeight() + clock_background.GetHeight());
  Rectanglei dr(turn_time_pos, clock_background.GetSize());

  // Draw background interface
  app->video->window.Blit(clock_background, turn_time_pos);
  GetWorld().ToRedrawOnScreen(dr);
  DrawClock(turn_time_pos + clock_background.GetSize() / 2);
}

// display time left in a turn
void Interface::DrawClock(const Point2i &time_pos) const
{
  // Draw turn time
  if (display_timer)
    timer->DrawCenter(time_pos - Point2i(0, clock_background.GetHeight()/3));

  // Draw clock
  Point2i tmp_point = time_pos - clock->GetSize() / 2;
  clock->Update();
  clock->DrawXY(tmp_point);

  // Draw global timer
  std::string tmp(Time::GetInstance()->GetString());
  global_timer->SetText(tmp);
  global_timer->DrawCenter(time_pos + Point2i(0, clock_background.GetHeight()/3));
}

// draw wind indicator
void Interface::DrawWindIndicator(const Point2i &wind_bar_pos, const bool draw_icon) const
{
  AppWormux * app = AppWormux::GetInstance();
  int height;

  // draw wind icon
  if (draw_icon) {
    app->video->window.Blit(wind_icon, wind_bar_pos);
    GetWorld().ToRedrawOnScreen(Rectanglei(wind_bar_pos, wind_icon.GetSize()));
    height = wind_icon.GetHeight() - wind_indicator.GetHeight();
  } else {
    height = MARGIN;
  }

  // draw wind indicator
  Point2i wind_bar_offset = Point2i(0, height);
  Point2i tmp = wind_bar_pos + wind_bar_offset + Point2i(2, 2);
  app->video->window.Blit(wind_indicator, wind_bar_pos + wind_bar_offset);
  wind_bar.DrawXY(tmp);
  GetWorld().ToRedrawOnScreen(Rectanglei(wind_bar_pos + wind_bar_offset, wind_indicator.GetSize()));
}

// display wind info
void Interface::DrawWindInfo() const
{
  Point2i wind_pos_offset = Point2i(game_menu.GetWidth() / 2 + clock_background.GetWidth() / 2 + MARGIN, game_menu.GetHeight() / 2 - wind_icon.GetHeight() / 2);
  DrawWindIndicator(bottom_bar_pos + wind_pos_offset, true);
}

// draw mini info when hidding interface
void Interface::DrawSmallInterface() const
{
  if (display) return;
  AppWormux * app = AppWormux::GetInstance();
  int height;
  height = ((int)Time::GetInstance()->Read() - start_hide_display - 1000) / 3 - 30;
  height = (height > 0 ? height : 0);
  height = (height < small_background_interface.GetHeight() ? height : small_background_interface.GetHeight());
  Point2i small_interface_position = Point2i(app->video->window.GetWidth() / 2 - small_background_interface.GetWidth() / 2, app->video->window.GetHeight() - height);
  app->video->window.Blit(small_background_interface,small_interface_position);
  GetWorld().ToRedrawOnScreen(Rectanglei(small_interface_position,small_background_interface.GetSize()));
  DrawWindIndicator(small_interface_position + Point2i(MARGIN, 0), false);
  if (display_timer)
    timer->DrawLeftTop(small_interface_position + Point2i(MARGIN * 2 + wind_bar.GetWidth(), MARGIN));
}

// draw team energy
void Interface::DrawTeamEnergy() const
{
  Point2i team_bar_offset = Point2i(game_menu.GetWidth() / 2 + clock_background.GetWidth() / 2 + wind_icon.GetWidth() + MARGIN, MARGIN);
  FOR_EACH_TEAM(tmp_team) {
    if (!display) // Fix bug #7753 (Team energy bar visible when the interface is hidden)
      (**tmp_team).GetEnergyBar().FinalizeMove();
    (**tmp_team).DrawEnergy(bottom_bar_pos + team_bar_offset);
  }
}

// Draw map preview
void Interface::DrawMapPreview()
{
  Surface &  window  = GetMainWindow();
  Point2i    offset(window.GetWidth() - GetWorld().ground.GetPreviewSize().x - 2*MARGIN,
                    2*MARGIN);
  Rectanglei rect_preview(offset, GetWorld().ground.GetPreviewSize());

  if (minimap == NULL ||
      GetWorld().ground.GetLastPreviewRedrawTime() > m_last_minimap_redraw ||
      GetWorld().water.GetLastPreviewRedrawTime() > m_last_minimap_redraw) {

    m_last_minimap_redraw = Time::GetInstance()->Read();

    // Check whether the whole minimap must be updated
    if (m_last_preview_size != GetWorld().ground.GetPreviewSize()) {
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
      minimap = new Surface(GetWorld().ground.GetPreviewSize(), SDL_SWSURFACE, true);
    Point2i mergePos = -GetWorld().ground.GetPreviewRect().GetPosition();

    minimap->Blit(*GetWorld().ground.GetPreview(), mergePos);

    // Draw water
    if (GetWorld().water.IsActive()) {
      const Color * color = GetWorld().water.GetColor();
      ASSERT(color);

      // Scale water height according to preview size
      int y = GetWorld().GetSize().GetY() - GetWorld().water.GetSelfHeight();
      int h = GetWorld().ground.PreviewCoordinates(Point2i(0, y)).GetY();

      Surface water_surf(Point2i(rect_preview.GetSizeX(), rect_preview.GetSizeY()-h),
                         SDL_SWSURFACE, false);
      water_surf.SetAlpha(SDL_SRCALPHA|SDL_RLEACCEL, 200);
      water_surf.Fill(*color);

      // Draw box with color according to water type
      minimap->Blit(water_surf, Point2i(0, h));
    }
    GenerateStyledBox();
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

void Interface::GenerateStyledBox()
{
  if (!mask) {
    m_last_preview_size = GetWorld().ground.GetPreviewSize();
    mask = new Surface(m_last_preview_size, SDL_HWSURFACE, true);

    Rectanglei temp_rect;
    temp_rect.SetPosition(Point2i(0,0));
    temp_rect.SetSize(minimap->GetSize());

    Point2i temp_position;

    temp_position = temp_rect.GetPosition();
    mask->Blit(rounding_style[0][0], temp_position);

    temp_position = temp_rect.GetPosition();
    temp_position.x += temp_rect.GetSize().x - rounding_style[2][0].GetSize().x;
    mask->Blit(rounding_style[2][0],temp_position);

    temp_position = temp_rect.GetPosition();
    temp_position.y += temp_rect.GetSize().y - rounding_style[0][2].GetSize().y;
    mask->Blit(rounding_style[0][2],temp_position);

    temp_position = temp_rect.GetPosition();
    temp_position.x += temp_rect.GetSize().x - rounding_style[2][2].GetSize().x;
    temp_position.y += temp_rect.GetSize().y - rounding_style[2][2].GetSize().y;
    mask->Blit(rounding_style[2][2],temp_position);

    for (int i = rounding_style[0][0].GetSize().x;
        i < (temp_rect.GetSize().x - rounding_style[2][0].GetSize().x);
        ++i) {
      temp_position = temp_rect.GetPosition();
      temp_position.x += i;
      mask->Blit(rounding_style[1][0],temp_position);

      temp_position.y += temp_rect.GetSize().y - rounding_style[1][2].GetSize().y;
      mask->Blit(rounding_style[1][2],temp_position);
    }

    for (int i = rounding_style[0][0].GetSize().y;
        i< (temp_rect.GetSize().y - rounding_style[0][2].GetSize().y);
        ++i) {
      temp_position = temp_rect.GetPosition();
      temp_position.y += i;
      mask->Blit(rounding_style[0][1],temp_position);

      temp_position.x += temp_rect.GetSize().x - rounding_style[2][1].GetSize().x;
      mask->Blit(rounding_style[2][1],temp_position);
    }

    for (int i = rounding_style[0][0].GetSize().x;
        i < (temp_rect.GetSize().x - rounding_style[2][0].GetSize().x);
        ++i) {

      for (int j = rounding_style[0][0].GetSize().y; j< (temp_rect.GetSize().y - rounding_style[0][2].GetSize().y);j++){
        temp_position = temp_rect.GetPosition() + Point2i(i,j);
        mask->Blit(rounding_style[1][1],temp_position);
      }
    }
    mask->SetAlpha(0, 0);
    //printf("Recreated mask\n");
  }

  // Recreate the scratch buffer
  if (!scratch) {
    scratch = new Surface(GetWorld().ground.GetPreviewSize(), SDL_SWSURFACE, true);
    scratch->SetAlpha(0, 0);
  }

  // Compose
  scratch->Blit(*mask);
  scratch->Blit(*minimap);

  // Save result
  minimap->Blit(*scratch);
}

void Interface::Draw()
{
  AppWormux * app = AppWormux::GetInstance();
  bottom_bar_pos = (app->video->window.GetSize() - GetSize()) * Point2d(0.5, 1);

  if (display_minimap)
    DrawMapPreview();


  // Position on the screen
  Point2i barPos = (app->video->window.GetSize() - weapon_strength_bar.GetSize()) * Point2d(0.5, 1)
      - Point2i(0, game_menu.GetHeight() + MARGIN);

  // Drawing on the screen
  weapon_strength_bar.DrawXY(barPos);

  weapons_menu.Draw();

  // Display the background of both Character info and weapon info
  Rectanglei dr(bottom_bar_pos, game_menu.GetSize());
  app->video->window.Blit(game_menu, bottom_bar_pos);

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

Point2i Interface::GetSize() const
{
  return Point2i(GetWidth(), GetHeight());
}

Point2i Interface::GetMenuPosition() const
{
  return bottom_bar_pos;
}

void Interface::EnableDisplay(bool _display)
{
  display = _display;
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

bool Interface::ActionClick(const Point2i &mouse_pos)
{
  // From Interface::DrawWeaponInfo()
  Weapon* weapon;
  Double icon_scale_factor = 0.75;

  if (!display)
    return false;

  // Get the weapon
  if (!weapon_under_cursor) {
    weapon = &ActiveTeam().AccessWeapon();
  } else {
    weapon = weapon_under_cursor;
    icon_scale_factor = cos((Double)Time::GetInstance()->Read()/1000 * PI)
                      * Double(0.9);
  }

  weapon->GetIcon().Scale(icon_scale_factor, 0.75);
  Point2i top_left = Point2i(game_menu.GetWidth() / 2 - clock_background.GetWidth() - weapon->GetIcon().GetWidth()/ 2 , 0);
  Point2i bottom_right = Point2i(game_menu.GetWidth() / 2 - clock_background.GetWidth() / 2, game_menu.GetHeight());
  Rectanglei weapon_button = Rectanglei(top_left, -top_left+bottom_right);

  top_left = Point2i((game_menu.GetWidth() - clock_background.GetWidth())/ 2, 0);
  Rectanglei clock_button = Rectanglei(top_left, clock_background.GetSize());

  Rectanglei character_button = Rectanglei(Point2i(0,0), Point2i(weapon_button.GetPositionX(), game_menu.GetHeight()));

  if (weapon_button.Contains(mouse_pos-bottom_bar_pos) && ActiveTeam().IsLocalHuman()) {
    weapons_menu.SwitchDisplay();
    return true;
  } else if (clock_button.Contains(mouse_pos-bottom_bar_pos)) {
    Game::GetInstance()->UserAsksForMenu();
    return true;
  } else if (character_button.Contains(mouse_pos-bottom_bar_pos)) {
    Camera::GetInstance()->CenterOnActiveCharacter();
    return true;
  } else if (weapons_menu.ActionClic(mouse_pos)) {
    // Process click on weapon menu before minimap as it should be
    // overlayed on top of it.
    return true;
  } else if (display_minimap) {
    Surface &  window  = GetMainWindow();
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

void Interface::MinimapSizeDelta(int delta)
{
  GetWorld().ground.SetPreviewSizeDelta(delta);
}
