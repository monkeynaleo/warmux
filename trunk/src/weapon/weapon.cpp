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
 * Virtual class to handle weapon in wormux.
 * Weapon projectile are handled in WeaponLauncher (see launcher.cpp and launcher.h).
 *****************************************************************************/

#include "weapon.h"
#include <SDL.h>
#include <SDL_rotozoom.h>
#include <SDL_gfxPrimitives.h>
#include <iostream>
#include <sstream>
#include "explosion.h"
#include "../character/move.h"
#include "../game/time.h"
#include "../game/game_loop.h"
#include "../graphic/video.h"
#include "../graphic/sprite.h"
#include "../gui/progress_bar.h"
#include "../include/app.h"
#include "../include/action_handler.h"
#include "../graphic/font.h"
#include "../interface/interface.h"
#include "../map/camera.h"
#include "../object/objects_list.h"
#include "../team/macro.h"
#include "../team/team.h"
#include "../tool/debug.h"
#include "../tool/i18n.h"
#include "../tool/math_tools.h"
#include "../tool/point.h"
#include "../tool/resource_manager.h"
#include "../tool/xml_document.h"

const int INFINITE_AMMO = -1;
const uint MAX_TIME_LOADING = 2000;

const uint UNIT_BOX_WIDTH = 22;
const uint UNIT_BOX_HEIGHT = 20;
const uint UNIT_BOX_GAP = 6;

const uint WEAPON_BOX_BUTTON_DX = 20;
const uint WEAPON_BOX_BUTTON_DY = 50;

const uint ANIM_DISPLAY_TIME = 400;

extern WeaponStrengthBar weapon_strength_bar;


Weapon::Weapon(Weapon_type type,
	       const std::string &id,
	       EmptyWeaponConfig * params,
	       weapon_visibility_t visibility)
{
  m_type = type;
  m_id = id;

  m_is_active = false;

  m_time_anim_begin = Time::GetInstance()->Read();
  m_initial_nb_ammo = INFINITE_AMMO;
  m_initial_nb_unit_per_ammo = 1;
  use_unit_on_first_shoot = true;
  can_be_used_on_closed_map = true;

  m_strength = 0;
  m_first_time_loading = 0;
  m_last_fire_time = 0;
  m_fire_remanence_time = 100;
  max_strength = min_angle = max_angle = 0;
  use_flipping = true;

  origin = weapon_origin_HAND;

  m_can_change_weapon = false;

  m_visibility = visibility;
  m_unit_visibility = ALWAYS_VISIBLE;

  m_image = NULL;
  m_weapon_fire = NULL;

  channel_load = -1;

  if (!use_flipping && !EgalZero(min_angle - max_angle))
    use_flipping = true;

  extra_params = params;

  if (m_visibility != NEVER_VISIBLE)
  {
    m_image = new Sprite( resource_manager.LoadImage(weapons_res_profile, m_id));
    if(!EgalZero(min_angle - max_angle))
      m_image->cache.EnableLastFrameCache();
  }

  icon = new Sprite(resource_manager.LoadImage(weapons_res_profile,m_id+"_ico"));

  mouse_character_selection = true;

  xmlpp::Element *elem = resource_manager.GetElement(weapons_res_profile, "position", m_id);
  if (elem != NULL) {
    // E.g. <position name="my_weapon_id" origin="hand" x="-1" y="0" />
    std::string origin_xml;
    XmlReader::ReadIntAttr (elem, "x", position.x);
    XmlReader::ReadIntAttr (elem, "y", position.y);
    if(!XmlReader::ReadStringAttr (elem, "origin", origin_xml))
    {
      std::cerr << "No \"origin\" flag found for weapon %s" << m_id <<std::endl;
      assert(false);
    }
    if (origin_xml == "over")
      origin = weapon_origin_OVER;
    else
      origin = weapon_origin_HAND;
  }
  else
  {
    std::cerr << "No \"position\" flag found for weapon %s" << m_id <<std::endl;
    assert(false);
  }

  elem = resource_manager.GetElement(weapons_res_profile, "hole", m_id);
  if (elem != NULL) {
    // E.g. <hole name="my_weapon_id" dx="-1" dy="0" />
    XmlReader::ReadIntAttr(elem, "dx", hole_delta.x);
    XmlReader::ReadIntAttr(elem, "dy", hole_delta.y);
  }

}

Weapon::~Weapon()
{
  if(extra_params) delete extra_params;
  if(icon) delete icon;
}

void Weapon::p_Select ()
{
  m_last_fire_time = 0;
}
void Weapon::p_Deselect () {}

void Weapon::Select()
{
  MSG_DEBUG("weapon.change", "Select %s", m_name.c_str());

  m_time_anim_begin = Time::GetInstance()->Read();
  m_is_active = false;
  m_strength = 0;
  ActiveTeam().ResetNbUnits();

  ActiveCharacter().SetWeaponClothe();

  // is there a crosshair ?
  if (!EgalZero(min_angle - max_angle))
    ActiveTeam().crosshair.enable = true;

  p_Select();

  // be sure that angle is correct
  ActiveCharacter().SetFiringAngle(ActiveCharacter().GetAbsFiringAngle());

  if (max_strength == 0) return ;

  // prepare the strength bar
  weapon_strength_bar.InitVal (0, 0, uint(max_strength*100));

  // init stamp on the stength_bar
  double val = ActiveCharacter().previous_strength;
  weapon_strength_bar.ResetTag();
  if (0 < val && val < max_strength)
  weapon_strength_bar.AddTag (uint(val*100), primary_red_color);
}

void Weapon::Deselect()
{
  ActiveTeam().crosshair.enable = false;
  MSG_DEBUG("weapon.change", "Deselect %s", m_name.c_str());
  p_Deselect();
}

void Weapon::Manage()
{
  if (!m_is_active)
    return ;

  Refresh();

  GameLoop * game_loop = GameLoop::GetInstance();

  if (game_loop->ReadState() != GameLoop::PLAYING)
    return;

  if ( (ActiveTeam().ReadNbUnits() == 0) )
    {
      Deselect();

      if (m_can_change_weapon)
	Select();
      else
	game_loop->SetState(GameLoop::HAS_PLAYED);
    }
}

bool Weapon::CanChangeWeapon() const
{
  if ( !ActiveTeam().IsLocal() )
    return false;

  if ( (ActiveTeam().ReadNbUnits() != m_initial_nb_unit_per_ammo) &&
       (m_can_change_weapon == false))
    return false;

  return true;
}

void Weapon::NewActionWeaponShoot() const
{
  assert(ActiveTeam().IsLocal() || ActiveTeam().IsLocalAI());

  Action* a_shoot = new Action(Action::ACTION_WEAPON_SHOOT,
			       m_strength,
			       ActiveCharacter().GetAbsFiringAngle());
  ActionHandler::GetInstance()->NewActionActiveCharacter(a_shoot);
}

void Weapon::NewActionWeaponStopUse() const
{
  assert(ActiveTeam().IsLocal() || ActiveTeam().IsLocalAI());

  Action* a = new Action(Action::ACTION_WEAPON_STOP_USE);
  ActionHandler::GetInstance()->NewActionActiveCharacter(a);
}

void Weapon::PrepareShoot(double strength, double angle)
{
  MSG_DEBUG("weapon.shoot", "Try to shoot with strength:%f, angle:%f", 
	    strength, angle);
  ActiveCharacter().SetFiringAngle(angle);
  m_strength = strength;
  StopLoading();

  ActiveCharacter().PrepareShoot();
}

bool Weapon::Shoot()
{
  MSG_DEBUG("weapon.shoot", "Enough ammo ? %d", EnoughAmmo() );
  MSG_DEBUG("weapon.shoot", "Enough ammo unit ? %d", EnoughAmmoUnit() );
  MSG_DEBUG("weapon.shoot", "Use unit on 1st shoot ? %d", use_unit_on_first_shoot );


  {
    // WARNING: The following commented code is wrong! Please see explanation following
    //   if (!EnoughAmmo()
    //       || (use_unit_on_first_shoot && !EnoughAmmoUnit()))
    //     return false;


    // Gentildemon : YES the following code seems strange!
    // BUT when have only one ammo left, you shoot, then nb_ammo == 0
    // then you need to be able to use the left ammo units

    if (use_unit_on_first_shoot && !EnoughAmmoUnit())
      return false;

    if (!EnoughAmmo())
      if ( ! (ActiveTeam().ReadNbAmmos() == 0
	      && use_unit_on_first_shoot && EnoughAmmoUnit()) )
	return false;
  }

  MSG_DEBUG("weapon.shoot", "Enough ammo");

  MSG_DEBUG("weapon.shoot", "%s Shooting at position:%d,%d (hand: %d,%d)", 
	    ActiveCharacter().GetName().c_str(),
	    ActiveCharacter().GetX(),
	    ActiveCharacter().GetY(),
	    ActiveCharacter().GetHandPosition().GetX(),
	    ActiveCharacter().GetHandPosition().GetY());
  ActiveCharacter().body->DebugState();
  if (!p_Shoot()) return false;
  m_last_fire_time = Time::GetInstance()->Read();

  MSG_DEBUG("weapon.shoot", "shoot!");

  // Is this the first shoot for this ammo use ?
  if (ActiveTeam().ReadNbUnits() == m_initial_nb_unit_per_ammo) {
    UseAmmo();
  }

  if (use_unit_on_first_shoot){
    UseAmmoUnit();
  }

  m_is_active = true;

  if (max_strength != 0) ActiveCharacter().previous_strength = m_strength;

  GameLoop::GetInstance()->character_already_chosen = true;

  return true;
}

// Calcule la position de l'image de l'arme
void Weapon::PosXY (int &x, int &y) const
{
  if(origin == weapon_origin_HAND)
  {
    Point2i handPos = ActiveCharacter().GetHandPosition();
    y = handPos.y + position.y;
    if (ActiveCharacter().GetDirection() == 1)
      x = handPos.x + position.x;
    else
      x = handPos.x - position.x;

    if(ActiveCharacter().GetDirection()==-1)
      x -= m_image->GetWidth();
  }
  else
  if(origin == weapon_origin_OVER)
  {
    x = ActiveCharacter().GetCenterX() - m_image->GetWidth() / 2 + position.x;
    y = ActiveCharacter().GetY()       - m_image->GetHeight()    + position.y;
  }
  else
    assert(false);
}

const Point2i Weapon::GetGunHolePosition()
{
  const Point2i &pos = ActiveCharacter().GetHandPosition();
  Point2i hole(pos +  hole_delta);
  double dst = pos.Distance(hole);
  double angle = pos.ComputeAngle(hole);
  return pos + Point2i(static_cast<int>(dst * cos(angle + ActiveCharacter().GetFiringAngle())),
                       static_cast<int>(dst * sin(angle + ActiveCharacter().GetFiringAngle())));
}

bool Weapon::EnoughAmmo() const
{
  int ammo = ActiveTeam().ReadNbAmmos();
  return ((ammo == INFINITE_AMMO) || (0 < ammo));
}

void Weapon::UseAmmo()
{
  // Use one ammo...
  int *ammo = &ActiveTeam().AccessNbAmmos();
  if (*ammo != INFINITE_AMMO) (*ammo)--;

  assert (*ammo >= 0 || *ammo == INFINITE_AMMO);
}

bool Weapon::EnoughAmmoUnit() const
{
  int unit = ActiveTeam().ReadNbUnits();
  return (unit > 0);
}

void Weapon::UseAmmoUnit()
{
  // Use one ammo unit.
  int *unit = &ActiveTeam().AccessNbUnits();
  (*unit)--;

  assert (unit >= 0);
}

int Weapon::ReadInitialNbAmmo() const{
  return m_initial_nb_ammo;
}

int Weapon::ReadInitialNbUnit() const{
  return m_initial_nb_unit_per_ammo;
}

bool Weapon::CanBeUsedOnClosedMap() const{
  return can_be_used_on_closed_map;
}

const std::string& Weapon::GetName() const {
  assert (!m_name.empty());
  return m_name;
}

const std::string& Weapon::GetID() const {
  assert (!m_name.empty());
  return m_id;
}

Weapon::Weapon_type Weapon::GetType() const {
  return m_type;
}

void Weapon::UpdateStrength(){
  if( max_strength == 0 || m_first_time_loading == 0 )
    return ;

  uint time = Time::GetInstance()->Read() - m_first_time_loading;
  double val = (max_strength * time) / MAX_TIME_LOADING;

  m_strength = BorneDouble (val, 0.0, max_strength);

  weapon_strength_bar.UpdateValue ((int)(m_strength*100));
}

bool Weapon::IsReady() const{
  return EnoughAmmo() ;
}

void Weapon::InitLoading(){
  // no loading for weapon with max_strength = 0
  if (max_strength == 0)
    return ;

  channel_load = jukebox.Play("share","weapon/load");

  m_first_time_loading = Time::GetInstance()->Read();

  m_strength = 0;

  GameLoop::GetInstance()->character_already_chosen = true;
}

void Weapon::StopLoading(){
  m_first_time_loading = 0 ;

  jukebox.Stop(channel_load);
}

void Weapon::Draw(){
  if(GameLoop::GetInstance()->ReadState() != GameLoop::PLAYING &&
     m_last_fire_time + 100 < Time::GetInstance()->Read())
    return;

  if (m_last_fire_time + m_fire_remanence_time > Time::GetInstance()->Read())
    DrawWeaponFire();
  weapon_strength_bar.visible = false;

  switch (m_unit_visibility)
    {
    case VISIBLE_ONLY_WHEN_ACTIVE:
      if (!m_is_active)
	break;
      
    case NEVER_VISIBLE:
      break;
      
    default:
      if (m_initial_nb_unit_per_ammo > 1)
	DrawUnit(ActiveTeam().ReadNbUnits());
    }

  // Do we need to draw strength_bar ? (real draw is done by class Interface)
  // We do not draw on the network
  if (max_strength != 0 && IsReady() && !m_is_active &&
      (ActiveTeam().IsLocal() || ActiveTeam().IsLocalAI()))
    weapon_strength_bar.visible = true;

  switch (m_visibility)
    {
      case ALWAYS_VISIBLE:
	break ;

      case NEVER_VISIBLE:
	return ;

      case VISIBLE_ONLY_WHEN_ACTIVE:
	if (!m_is_active)
	  return ;
	break;

      case VISIBLE_ONLY_WHEN_INACTIVE:
	if (m_is_active)
	  return ;
	break;

      default:
	printf ("Hum... there is a problem !!!\n");
	break;
    }

  if(ActiveCharacter().IsGhost()
  || ActiveCharacter().IsDrowned()
  || ActiveCharacter().IsDead())
    return;

  // Reset the Sprite:
  m_image->SetRotation_rad(0.0);
  m_image->Scale(1.0,1.0);

  // rotate weapon if needed
  if (!EgalZero(min_angle - max_angle))
  {
    if(ActiveCharacter().GetDirection() == 1)
      m_image->SetRotation_rad (ActiveCharacter().GetFiringAngle());
    else
      m_image->SetRotation_rad (ActiveCharacter().GetFiringAngle() - M_PI);
  }

  // flip image if needed
  if (use_flipping)
  {
    m_image->Scale(ActiveCharacter().GetDirection(), 1.0);
  }

  // Calculate position of the image
  int x,y;
  PosXY (x, y);

  // Animate the display of the weapon:
  if( m_time_anim_begin + ANIM_DISPLAY_TIME > Time::GetInstance()->Read())
  {
    if (!EgalZero(min_angle - max_angle))
    {
      double angle = m_image->GetRotation_rad();
      angle += sin( M_PI_2 * double(Time::GetInstance()->Read() - m_time_anim_begin) /(double) ANIM_DISPLAY_TIME) * 2 * M_PI;
      m_image->SetRotation_rad (angle);
    }
    else
    {
      float scale = sin( 1.5 * M_PI_2 * double(Time::GetInstance()->Read() - m_time_anim_begin) /(double) ANIM_DISPLAY_TIME) / sin(1.5 * M_PI_2);
      m_image->Scale(ActiveCharacter().GetDirection() * scale,scale);

      if(origin == weapon_origin_OVER) PosXY(x,y); //Recompute position to get the icon centered over the skin
    }
  }

  if ( m_image )
    m_image->Blit( AppWormux::GetInstance()->video.window, Point2i(x, y) - camera.GetPosition());
}

// Draw the weapon fire when firing
void Weapon::DrawWeaponFire()
{
  if (m_weapon_fire == NULL) return;
  Point2i size = m_weapon_fire->GetSize();
  size.x = (ActiveCharacter().GetDirection() == Body::DIRECTION_RIGHT ? 0 : size.x);
  size.y /= 2;
  m_weapon_fire->SetRotation_rad (ActiveCharacter().GetFiringAngle());
  m_weapon_fire->Draw( GetGunHolePosition() - size );
}

void Weapon::DrawUnit(int unit) const
{
  Rectanglei rect;

  std::ostringstream ss;

  ss << unit;

  DrawTmpBoxText(*Font::GetInstance(Font::FONT_SMALL),
		 Point2i( ActiveCharacter().GetCenterX(), 
			  ActiveCharacter().GetY() - UNIT_BOX_HEIGHT / 2 - UNIT_BOX_GAP )
		 - camera.GetPosition(),
		 ss.str());
}

Sprite & Weapon::GetIcon() const
{
  return *icon;
}

bool Weapon::LoadXml(xmlpp::Element * weapon)
{
  xmlpp::Element *elem = XmlReader::GetMarker(weapon, m_id);
  if (elem == NULL)
  {
      std::cout << Format(_("No element <%s> found in the xml config file!"),
                          m_id.c_str())
                << std::endl;
    return false;
  }

  XmlReader::ReadInt(elem, "nb_ammo", m_initial_nb_ammo);
  XmlReader::ReadInt(elem, "unit_per_ammo", m_initial_nb_unit_per_ammo);

  // max strength
  // if max_strength = 0, no strength_bar !
  XmlReader::ReadDouble(elem, "max_strength", max_strength);

  // change weapon after ? (for the ninja cord = true)
  XmlReader::ReadBool(elem, "change_weapon", m_can_change_weapon);

  // angle of weapon when drawing
  // if (min_angle == max_angle) no cross_hair !
  // between -90 to 90 degrees
  int min_angle_deg = 0, max_angle_deg = 0;
  XmlReader::ReadInt(elem, "min_angle", min_angle_deg);
  XmlReader::ReadInt(elem, "max_angle", max_angle_deg);
  min_angle = static_cast<double>(min_angle_deg) * M_PI / 180.0;
  max_angle = static_cast<double>(max_angle_deg) * M_PI / 180.0;

  // Load extra parameters if existing
  if (extra_params != NULL) extra_params->LoadXml(elem);

  if (m_visibility != NEVER_VISIBLE && origin == weapon_origin_HAND)
    m_image->SetRotation_HotSpot(-position);

  return true;
}

bool Weapon::IsInUse() const{
  return m_is_active;
}

const double Weapon::ReadStrength() const{
  return m_strength;
}

bool Weapon::IsLoading() const{
  return m_first_time_loading;
}

void Weapon::ChooseTarget(Point2i mouse_pos){
}

void Weapon::SignalTurnEnd(){
}

void Weapon::ActionStopUse()
{
  assert(false);
}
  
// Handle keyboard events

// #################### SHOOT
void Weapon::HandleKeyPressed_Shoot()
{  
  if(ActiveCharacter().IsPreparingShoot())
    return;

  if (max_strength == 0)
    NewActionWeaponShoot();
  else if ( IsReady() )
    InitLoading();
}

void Weapon::HandleKeyRefreshed_Shoot()
{  
  if(ActiveCharacter().IsPreparingShoot())
    return;
  if ( !IsLoading() ) 
    return;

  // Strength == max strength -> Fire !!!
  if (ReadStrength() >= max_strength) {
    NewActionWeaponShoot();
  } else {
    // still pressing the Space key
    UpdateStrength();
  }
}

void Weapon::HandleKeyReleased_Shoot()
{  
  if(ActiveCharacter().IsPreparingShoot())
    return;
  if ( !IsLoading()) 
    return;

  NewActionWeaponShoot();
}

void Weapon::HandleKeyPressed_MoveRight()
{
  ActiveCharacter().HandleKeyPressed_MoveRight();
}

void Weapon::HandleKeyRefreshed_MoveRight()
{
  ActiveCharacter().HandleKeyRefreshed_MoveRight();
}

void Weapon::HandleKeyReleased_MoveRight()
{
  ActiveCharacter().HandleKeyReleased_MoveRight();
}

void Weapon::HandleKeyPressed_MoveLeft()
{
  ActiveCharacter().HandleKeyPressed_MoveLeft();
}

void Weapon::HandleKeyRefreshed_MoveLeft()
{
  ActiveCharacter().HandleKeyRefreshed_MoveLeft();
}

void Weapon::HandleKeyReleased_MoveLeft()
{
  ActiveCharacter().HandleKeyReleased_MoveLeft();
}

void Weapon::HandleKeyPressed_Up()
{
  ActiveCharacter().HandleKeyPressed_Up();
}

void Weapon::HandleKeyRefreshed_Up()
{
  ActiveCharacter().HandleKeyRefreshed_Up();
}

void Weapon::HandleKeyReleased_Up()
{
  ActiveCharacter().HandleKeyReleased_Up();
}

void Weapon::HandleKeyPressed_Down()
{
  ActiveCharacter().HandleKeyPressed_Down();
}

void Weapon::HandleKeyRefreshed_Down()
{
  ActiveCharacter().HandleKeyRefreshed_Down();
}

void Weapon::HandleKeyReleased_Down()
{
  ActiveCharacter().HandleKeyReleased_Down();
}

void Weapon::HandleKeyPressed_Jump()
{
  ActiveCharacter().HandleKeyPressed_Jump();
}

void Weapon::HandleKeyRefreshed_Jump()
{
  ActiveCharacter().HandleKeyRefreshed_Jump();
}

void Weapon::HandleKeyReleased_Jump()
{
  ActiveCharacter().HandleKeyReleased_Jump();
}

void Weapon::HandleKeyPressed_HighJump()
{
  ActiveCharacter().HandleKeyPressed_HighJump();
}

void Weapon::HandleKeyRefreshed_HighJump()
{
  ActiveCharacter().HandleKeyRefreshed_HighJump();
}

void Weapon::HandleKeyReleased_HighJump()
{
  ActiveCharacter().HandleKeyReleased_HighJump();
}

void Weapon::HandleKeyPressed_BackJump()
{
  ActiveCharacter().HandleKeyPressed_BackJump();
}

void Weapon::HandleKeyRefreshed_BackJump()
{
  ActiveCharacter().HandleKeyRefreshed_BackJump();
}

void Weapon::HandleKeyReleased_BackJump()
{
  ActiveCharacter().HandleKeyReleased_BackJump();
}
