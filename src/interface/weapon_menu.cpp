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
PolygonItem()
{
  weapon = new_weapon;
  SetSprite(new Sprite(weapon->GetIcon()));
  SetPosition(position);
  zoom = false;
  zoom_start_time = 0;
}

bool WeaponMenuItem::IsMouseOver()
{
  Point2i mouse_pos = Mouse::GetInstance()->GetPosition();
  if(Contains(Point2d((double)mouse_pos.x, (double)mouse_pos.y))) {
    if(!zoom) {
      zoom = true;
      zoom_start_time = Time::GetInstance()->Read();
    }
    return true;
  }
  if(zoom) {
    zoom = false;
    zoom_start_time = Time::GetInstance()->Read();
  }
  return false;
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
  } else {
    std::ostringstream txt;
    txt << nb_bullets;
    (*Font::GetInstance(Font::FONT_TINY)).WriteLeftBottom(tmp, txt.str(), white_color);
  }
}

Weapon * WeaponMenuItem::GetWeapon() const
{
  return weapon;
}

WeaponsMenu::WeaponsMenu()
{
  show = false;
  nbr_weapon_type = 0;
  motion_start_time = 0;

  // Loading value from XML
  Profile *res = resource_manager.LoadXMLProfile("graphism.xml", false);
  infinite = new Sprite(resource_manager.LoadImage(res, "interface/infinite"));
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

  nb_weapon_type = new int[MAX_NUMBER_OF_WEAPON];
  for(int i = 0; i < MAX_NUMBER_OF_WEAPON; i++)
    nb_weapon_type[i] = 0;
  resource_manager.UnLoadXMLProfile( res);
}

// Add a new weapon to the weapon menu.
void WeaponsMenu::AddWeapon(Weapon* new_item, uint num_sort)
{
  Point2d position;
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
    motion_start_time = Time::GetInstance()->Read();
    show = true;
  }
}

void WeaponsMenu::Hide()
{
  if(show) {
    Interface::GetInstance()->SetCurrentOverflyWeapon(NULL);
    motion_start_time = Time::GetInstance()->Read();
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

AffineTransform2D WeaponsMenu::ComputeToolTransformation()
{
  double coef;
  Point2i translate = (AppWormux::GetInstance()->video.window.GetSize() / 2);
  position.Init();
  shear.Init();
  rotation.Init();
  zoom.Init();
  position.SetTranslation(translate);
  if(Time::GetInstance()->Read() < motion_start_time + ICONS_DRAW_TIME) {
    coef = ((Time::GetInstance()->Read() - motion_start_time) / (double)ICONS_DRAW_TIME);
    if(show) {
      position.SetTranslation(POINT2I_2_POINT2D(translate) + POINT2I_2_POINT2D(translate) * Point2d(1.0 - coef, 1.0 - coef));
      zoom.SetShrink(coef, coef);
      rotation.SetRotation(coef * M_PI * 2.0);
    } else {
      position.SetTranslation(POINT2I_2_POINT2D(translate) + POINT2I_2_POINT2D(translate) * Point2d(coef, coef));
      zoom.SetShrink(1.0 - coef, 1.0 - coef);
      rotation.SetRotation(2 * M_PI - coef * M_PI * 2);
    }
  } else if(Time::GetInstance()->Read() < motion_start_time + ICONS_DRAW_TIME + JELLY_TIME) {
    coef = 1.0 - ((double)Time::GetInstance()->Read() - (motion_start_time + ICONS_DRAW_TIME)) / (double)JELLY_TIME;
    coef = -(cos((1.0 - coef) * M_PI * 4) * coef) / 5;
    shear.SetShear(coef, 0);
  }
  return position * shear * zoom * rotation;
}

AffineTransform2D WeaponsMenu::ComputeWeaponTransformation()
{
  double coef;
  Point2i translate = (AppWormux::GetInstance()->video.window.GetSize() / 2) - Point2i((int)(weapons_menu->GetWidth() / 2), 0);
  position.Init();
  shear.Init();
  rotation.Init();
  zoom.Init();
  position.SetTranslation(translate);
  if(Time::GetInstance()->Read() < motion_start_time + ICONS_DRAW_TIME) {
    coef = ((Time::GetInstance()->Read() - motion_start_time) / (double)ICONS_DRAW_TIME);
    if(show) {
      position.SetTranslation(POINT2I_2_POINT2D(translate) * Point2d(coef, coef));
      zoom.SetShrink(coef, coef);
      rotation.SetRotation(coef * M_PI * 2.0);
    } else {
      position.SetTranslation(POINT2I_2_POINT2D(translate) * Point2d(1.0 - coef, 1.0 - coef));
      zoom.SetShrink(1.0 - coef, 1.0 - coef);
      rotation.SetRotation(2 * M_PI - coef * M_PI * 2);
    }
  } else if(Time::GetInstance()->Read() < motion_start_time + ICONS_DRAW_TIME + JELLY_TIME) {
    coef = 1.0 - ((double)Time::GetInstance()->Read() - (motion_start_time + ICONS_DRAW_TIME)) / (double)JELLY_TIME;
    coef = -(cos((1.0 - coef) * M_PI * 4) * coef) / 5;
    shear.SetShear(coef, 0);
  }
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

Weapon * WeaponsMenu::UpdateCurrentOverflyItem(Polygon * poly) const
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
    ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_PLAYER_CHANGE_WEAPON, tmp->GetType()));
    Hide();
    return true;
  }
  return false;
}
