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
#include "../graphic/sprite.h"
#include "../weapon/weapon.h"
#include "../weapon/weapons_list.h"


// Weapon menu
const uint BUTTON_ICO_WIDTH = 58;  // Width of the button icon
const uint BUTTON_ICO_HEIGHT = 58; // Height of the button icon

const uint WEAPON_ICO_WIDTH = 48;   // Width of the weapon icon
const uint WEAPON_ICO_HEIGHT = 48;  // Height of the button icon

const uint BUTTON_ICO_GAP = 8; // Gap between buttons when a button is zoomed


const uint ICONS_DRAW_TIME = 600; // Time to display all icons (in ms)
const uint ICON_ZOOM_TIME = 150; // Time to zoom one icon.
const uint JELLY_TIME = 600;     // Jelly time when appearing

const double DEFAULT_ICON_SCALE = 0.7;
const double MAX_ICON_SCALE = 1.1;

const uint BUTTON_WIDTH = (int)(BUTTON_ICO_GAP + BUTTON_ICO_WIDTH  *
                               (DEFAULT_ICON_SCALE + MAX_ICON_SCALE)/2);

const uint BUTTON_HEIGHT = (int)(BUTTON_ICO_GAP + BUTTON_ICO_HEIGHT  *
                                (DEFAULT_ICON_SCALE + MAX_ICON_SCALE)/2);

const int WeaponsMenu::MAX_NUMBER_OF_WEAPON = 5;


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
  } else {
    zoom = false;
    zoom_start_time = Time::GetInstance()->Read();
  }
  return false;
}

void WeaponMenuItem::Draw(Surface * dest)
{
  double scale = DEFAULT_ICON_SCALE;
  if(zoom) {
    scale = (Time::GetInstance()->Read() - zoom_start_time) / (double)ICON_ZOOM_TIME;
    scale = DEFAULT_ICON_SCALE + (MAX_ICON_SCALE - DEFAULT_ICON_SCALE) * scale;
    scale = (scale > MAX_ICON_SCALE ? MAX_ICON_SCALE : scale);
  } /* else if(zoom_start_time + ICON_ZOOM_TIME < Time::GetInstance()->Read()) {
    scale = 1.0 - (Time::GetInstance()->Read() - zoom_start_time) / (double)ICON_ZOOM_TIME;
    scale = MAX_ICON_SCALE - (MAX_ICON_SCALE - DEFAULT_ICON_SCALE) * scale;
    scale = (scale < DEFAULT_ICON_SCALE ? DEFAULT_ICON_SCALE : scale);
  }*/
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
  Point2i size = resource_manager.LoadPoint2i(res, "interface/weapon_interface_size");
  background = PolygonGenerator::GenerateRoundedRectangle(size.x, size.y, 20);
  background->SetPlaneColor(resource_manager.LoadColor(res, "interface/background_color"));
  background->SetBorderColor(resource_manager.LoadColor(res, "interface/border_color"));
  weapon_over_mouse = NULL;
  nb_weapon_type = new int[MAX_NUMBER_OF_WEAPON];
  for(int i = 0; i < MAX_NUMBER_OF_WEAPON; i++)
    nb_weapon_type[i] = 0;
  resource_manager.UnLoadXMLProfile( res);
}

// Add a new weapon to the weapon menu.
void WeaponsMenu::AddWeapon(Weapon* new_item, uint num_sort)
{
  Point2d position;
  if(num_sort < 5)
    position = background->GetMin() + Point2d(10 + (num_sort * 50), 10 + nb_weapon_type[num_sort - 1] * 50);
  else
    position = background->GetMin() + Point2d(10 + nb_weapon_type[num_sort - 1] * 50, 490);
  WeaponMenuItem * item = new WeaponMenuItem(new_item, position);

  nb_weapon_type[num_sort - 1]++;
  background->AddItem(item);
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

void WeaponsMenu::Draw()
{
  if(!show && (motion_start_time == 0 || Time::GetInstance()->Read() >= motion_start_time + ICONS_DRAW_TIME))
    return;
  position.SetTranslation(-background->GetMin() + Point2d(10.0, 10.0));
  AffineTransform2D shear;
  AffineTransform2D rotation;
  AffineTransform2D zoom;
  if(Time::GetInstance()->Read() < motion_start_time + ICONS_DRAW_TIME) {
    double coef = ((Time::GetInstance()->Read() - motion_start_time) / (double)ICONS_DRAW_TIME);
    if(show) {
      zoom.SetShrink(coef, coef);
      rotation.SetRotation(coef * M_PI * 2.0);
    } else {
      zoom.SetShrink(1.0 - coef, 1.0 - coef);
      rotation.SetRotation(2 * M_PI - coef * M_PI * 2);
    }
  } /*else if(Time::GetInstance()->Read() < motion_start_time + ICONS_DRAW_TIME + JELLY_TIME) {
    if(show) {
      shear.SetShear(sin((Time::GetInstance()->Read() - motion_start_time + ICONS_DRAW_TIME) / 5.0), 0);
    }
  }*/
  // Draw background
  background->ApplyTransformation(position * zoom * rotation * shear);
  background->DrawOnScreen();
  UpdateCurrentOverflyWeapon();
}

Weapon * WeaponsMenu::UpdateCurrentOverflyWeapon() const
{
  std::vector<PolygonItem *> items = background->GetItem();
  WeaponMenuItem * tmp;
  Interface::GetInstance()->SetCurrentOverflyWeapon(NULL);
  for(std::vector<PolygonItem *>::iterator item = items.begin(); item != items.end(); item++) {
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
  tmp = UpdateCurrentOverflyWeapon();
  if(tmp != NULL) {
    ActionHandler::GetInstance()->NewAction(new Action(Action::ACTION_PLAYER_CHANGE_WEAPON, tmp->GetType()));
    Hide();
    return true;
  }
  return false;
}
