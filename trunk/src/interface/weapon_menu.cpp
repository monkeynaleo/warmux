/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
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
 * Interface showing various informations about the game.
 *****************************************************************************/

#include "weapon_menu.h"
#include "../weapon/weapons_list.h"

#include <sstream>
#include <math.h>
#include "interface.h"
#include "../graphic/video.h"
#include "../graphic/font.h"
#include "../game/time.h"
#include "../include/action_handler.h"
#include "../include/app.h"
#include "../interface/mouse.h"
#include "../map/camera.h"
#include "../map/map.h"
#include "../team/team.h"
#include "../team/teams_list.h"
#include "../tool/point.h"
#include "../tool/rectangle.h"
#include "../tool/string_tools.h"
#include "../tool/resource_manager.h"
#include "../tool/i18n.h"
#include "../graphic/sprite.h"
#include "../weapon/weapon.h"
#include "../weapon/weapons_list.h"


// Weapon menu
const uint ICONS_DRAW_TIME = 600; // Time to display all icons (in ms)
const uint ICON_ZOOM_TIME = 150; // Time to zoom one icon.
const uint JELLY_TIME = 300;     // Jelly time when appearing

const double DEFAULT_ICON_SCALE = 0.7;
const double MAX_ICON_SCALE = 1.1;

const int WeaponsMenu::MAX_NUMBER_OF_WEAPON = 7;


WeaponMenuItem::WeaponMenuItem(Weapon * new_weapon, const Point2d & position) :
  PolygonItem(),
  zoom(false),
  weapon(new_weapon),
  zoom_start_time(0)
{
  SetSprite(new Sprite(weapon->GetIcon()));
  SetPosition(position);
}

bool WeaponMenuItem::IsMouseOver()
{
  int nb_bullets = ActiveTeam().ReadNbAmmos(weapon->GetName());
  if(nb_bullets == 0) {
    if(zoom)
      SetZoom(false);
    return false;
  }
  Point2i mouse_pos = Mouse::GetInstance()->GetPosition();
  if(Contains(Point2d((double)mouse_pos.x, (double)mouse_pos.y))) {
    if(!zoom)
      SetZoom(true);
    return true;
  }
  if(zoom)
    SetZoom(false);
  return false;
}

void WeaponMenuItem::SetZoom(bool value)
{
  zoom = value;
  zoom_start_time = Time::GetInstance()->Read();
}

void WeaponMenuItem::Draw(Surface * dest)
{
  double scale = DEFAULT_ICON_SCALE;
  if(zoom || zoom_start_time + ICON_ZOOM_TIME > Time::GetInstance()->Read()) {
    scale = (Time::GetInstance()->Read() - zoom_start_time) / (double)ICON_ZOOM_TIME;
    if(zoom) {
      scale = DEFAULT_ICON_SCALE + (MAX_ICON_SCALE - DEFAULT_ICON_SCALE) * scale;
      scale = (scale > MAX_ICON_SCALE ? MAX_ICON_SCALE : scale);
    } else {
      scale = MAX_ICON_SCALE - (MAX_ICON_SCALE - DEFAULT_ICON_SCALE) * scale;
      scale = (scale > DEFAULT_ICON_SCALE ? scale : DEFAULT_ICON_SCALE);
    }
  }
  item->Scale(scale, scale);
  PolygonItem::Draw(dest);
  int nb_bullets = ActiveTeam().ReadNbAmmos(weapon->GetName());
  Point2i tmp = GetOffsetAlignment() + Point2i(0, item->GetWidth() - 10);
  if(nb_bullets ==  INFINITE_AMMO) {
    Interface::GetInstance()->GetWeaponsMenu().GetInfiniteSymbol()->Blit(*dest, tmp);
  } if(nb_bullets == 0) {
    tmp += Point2i(0, -Interface::GetInstance()->GetWeaponsMenu().GetCrossSymbol()->GetHeight() / 2);
    Interface::GetInstance()->GetWeaponsMenu().GetCrossSymbol()->Blit(*dest, tmp);
  } else {
    std::ostringstream txt;
    txt << nb_bullets;
    (*Font::GetInstance(Font::FONT_MEDIUM, Font::FONT_BOLD)).WriteLeft(tmp, txt.str(), gray_color);
  }
}

Weapon * WeaponMenuItem::GetWeapon() const
{
  return weapon;
}

WeaponsMenu::WeaponsMenu():
  weapons_menu(NULL),
  tools_menu(NULL),
  current_overfly_item(NULL),
  position(),
  shear(),
  rotation(),
  zoom(),
  infinite(NULL),
  cross(NULL),
  show(false),
  motion_start_time(0),
  nbr_weapon_type(0),
  nb_weapon_type(new int[MAX_NUMBER_OF_WEAPON])
{
  // Loading value from XML
  Profile *res = resource_manager.LoadXMLProfile("graphism.xml", false);
  infinite = new Sprite(resource_manager.LoadImage(res, "interface/infinite"));
  cross = new Sprite(resource_manager.LoadImage(res, "interface/cross"));
  // Polygon Size
  Point2i size = resource_manager.LoadPoint2i(res, "interface/weapons_interface_size");
  weapons_menu = PolygonGenerator::GenerateRoundedRectangle(size.x, size.y, 20);
  size = resource_manager.LoadPoint2i(res, "interface/tools_interface_size");
  tools_menu = PolygonGenerator::GenerateRoundedRectangle(size.x, size.y, 20);

  // Setting colors
  Color plane_color = resource_manager.LoadColor(res, "interface/background_color");
  Color border_color = resource_manager.LoadColor(res, "interface/border_color");
  weapons_menu->SetPlaneColor(plane_color);
  weapons_menu->SetBorderColor(border_color);
  tools_menu->SetPlaneColor(plane_color);
  tools_menu->SetBorderColor(border_color);

  // Adding label
  weapons_menu->AddItem(new Sprite(Font::GenerateSurface(_("Weapons"), gray_color, Font::FONT_BIG)),
                        weapons_menu->GetMin() + Point2d(20, 20), PolygonItem::LEFT, PolygonItem::TOP);
  tools_menu->AddItem(new Sprite(Font::GenerateSurface(_("Tools"), gray_color, Font::FONT_BIG)),
                        tools_menu->GetMin() + Point2d(20, 20), PolygonItem::LEFT, PolygonItem::TOP);

  for(int i = 0; i < MAX_NUMBER_OF_WEAPON; i++)
    nb_weapon_type[i] = 0;
  resource_manager.UnLoadXMLProfile( res);

  WeaponsList *weapons_list = WeaponsList::GetInstance();
  for (WeaponsList::weapons_list_it it=weapons_list->GetList().begin(); it != weapons_list->GetList().end(); ++it)
    AddWeapon(*it);
}

// Add a new weapon to the weapon menu.
void WeaponsMenu::AddWeapon(Weapon* new_item)
{
  Point2d position;
  Weapon::category_t num_sort = new_item->Category();
  if(num_sort < 6) {
    position = weapons_menu->GetMin() + Point2d(50 + nb_weapon_type[num_sort - 1] * 50, 80 + (num_sort - 1) * 50);
    WeaponMenuItem * item = new WeaponMenuItem(new_item, position);
    weapons_menu->AddItem(item);
  } else {
    position = tools_menu->GetMin() + Point2d(50 + (num_sort - 6) * 50, 80 + nb_weapon_type[num_sort - 1] * 50);
    WeaponMenuItem * item = new WeaponMenuItem(new_item, position);
    tools_menu->AddItem(item);
  }

  nb_weapon_type[num_sort - 1]++;
}

// Weapon menu display (init of the animation)
void WeaponsMenu::Show()
{
  ShowGameInterface();
  if(!show) {
    if(motion_start_time + ICONS_DRAW_TIME < Time::GetInstance()->Read())
      motion_start_time = Time::GetInstance()->Read();
    else
      motion_start_time = Time::GetInstance()->Read() - (ICONS_DRAW_TIME - (Time::GetInstance()->Read() - motion_start_time));
    show = true;
  }
}

void WeaponsMenu::Hide()
{
  if(show) {
    Interface::GetInstance()->SetCurrentOverflyWeapon(NULL);
    if(motion_start_time + ICONS_DRAW_TIME < Time::GetInstance()->Read())
      motion_start_time = Time::GetInstance()->Read();
    else
      motion_start_time = Time::GetInstance()->Read() - (ICONS_DRAW_TIME - (Time::GetInstance()->Read() - motion_start_time));
    show = false;
  }
}

void WeaponsMenu::Reset()
{
  Interface::GetInstance()->SetCurrentOverflyWeapon(NULL);
  motion_start_time = 0;
  show = false;
}

void WeaponsMenu::SwitchDisplay()
{
  if(show)
    Hide();
  else
    Show();
}

bool WeaponsMenu::IsDisplayed() const
{
  return show;
}

Sprite * WeaponsMenu::GetInfiniteSymbol() const
{
  return infinite;
}

Sprite * WeaponsMenu::GetCrossSymbol() const
{
  return cross;
}

AffineTransform2D WeaponsMenu::ComputeToolTransformation()
{
  // Init animation parameter
  Point2d start(AppWormux::GetInstance()->video.window.GetWidth(), 0);
  Point2i pos(AppWormux::GetInstance()->video.window.GetSize() / 2 + Point2i((int)(tools_menu->GetWidth() / 2) + 10, 0));
  Point2d end(POINT2I_2_POINT2D(pos));
  double zoom_start = 0.2, zoom_end = 1.0;
  double angle_start = M_PI * 2.0, angle_end = 0.0;
  // Define the animation
  position.SetTranslationAnimation(motion_start_time, ICONS_DRAW_TIME, Time::GetInstance()->Read(), !show, start, end);
  zoom.SetShrinkAnimation(motion_start_time, ICONS_DRAW_TIME, Time::GetInstance()->Read(), !show,
                          zoom_start, zoom_start, zoom_end, zoom_end);
  rotation.SetRotationAnimation(motion_start_time, ICONS_DRAW_TIME, Time::GetInstance()->Read(), !show, angle_start, angle_end);
  shear.SetShearAnimation(motion_start_time + ICONS_DRAW_TIME, JELLY_TIME, Time::GetInstance()->Read(), !show, 2.0, 0.2, 0.0);
  return position * shear * zoom * rotation;
}

AffineTransform2D WeaponsMenu::ComputeWeaponTransformation()
{
  // Init animation parameter
  Point2d start(0, 0);
  Point2i pos(AppWormux::GetInstance()->video.window.GetSize() / 2 - Point2i((int)(weapons_menu->GetWidth() / 2) + 10, 0));
  Point2d end(POINT2I_2_POINT2D(pos));
  double zoom_start = 0.2, zoom_end = 1.0;
  double angle_start = -M_PI * 2.0, angle_end = 0.0;
 // Define the animation
  position.SetTranslationAnimation(motion_start_time, ICONS_DRAW_TIME, Time::GetInstance()->Read(), !show, start, end);
  zoom.SetShrinkAnimation(motion_start_time, ICONS_DRAW_TIME, Time::GetInstance()->Read(), !show,
                          zoom_start, zoom_start, zoom_end, zoom_end);
  rotation.SetRotationAnimation(motion_start_time, ICONS_DRAW_TIME, Time::GetInstance()->Read(), !show, angle_start, angle_end);
  shear.SetShearAnimation(motion_start_time + ICONS_DRAW_TIME, JELLY_TIME, Time::GetInstance()->Read(), !show, 2.0, 0.2, 0.0);
  return position * shear * zoom * rotation;
}

void WeaponsMenu::Draw()
{
  if(!show && (motion_start_time == 0 || Time::GetInstance()->Read() >= motion_start_time + ICONS_DRAW_TIME))
    return;
  // Draw weapons menu
  weapons_menu->ApplyTransformation(ComputeWeaponTransformation());
  weapons_menu->DrawOnScreen();
  // Tools
  tools_menu->ApplyTransformation(ComputeToolTransformation());
  tools_menu->DrawOnScreen();
  // Update overfly weapon/tool
  if(UpdateCurrentOverflyItem(weapons_menu) == NULL)
    UpdateCurrentOverflyItem(tools_menu);
}

Weapon * WeaponsMenu::UpdateCurrentOverflyItem(Polygon * poly)
{
  std::vector<PolygonItem *> items = poly->GetItem();
  WeaponMenuItem * tmp;
  Interface::GetInstance()->SetCurrentOverflyWeapon(NULL);
  std::vector<PolygonItem *>::iterator item = items.begin();
  ++item; // Skeeping first item which is a text label
  for(; item != items.end(); item++) {
    tmp = (WeaponMenuItem *)(*item);
    if(tmp->IsMouseOver()) {
      Interface::GetInstance()->SetCurrentOverflyWeapon(tmp->GetWeapon());
      if(current_overfly_item != tmp) {
        if(current_overfly_item != NULL)
          current_overfly_item->SetZoom(false);
        current_overfly_item = tmp;
      }
      return tmp->GetWeapon();
    }
  }
  return NULL;
}

bool WeaponsMenu::ActionClic(const Point2i &mouse_pos)
{
  Weapon * tmp;
  if(!show)
    return false;
  if((tmp = UpdateCurrentOverflyItem(weapons_menu)) == NULL)
    tmp = UpdateCurrentOverflyItem(tools_menu);
  if(tmp != NULL) {
    // Check we have enough ammo
    int nb_bullets = ActiveTeam().ReadNbAmmos(tmp->GetName());
    if( nb_bullets == INFINITE_AMMO || nb_bullets > 0)
    {
      ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_PLAYER_CHANGE_WEAPON, tmp->GetType()));
      Hide();
      return true;
    }
  }
  return false;
}
