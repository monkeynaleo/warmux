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
 * Air attack.
 *****************************************************************************/

#include "air_attack.h"
#include <sstream>
#include "../game/game_loop.h"
#include "../graphic/sprite.h"
#include "../include/action_handler.h"
#include "../interface/mouse.h"
#include "../map/map.h"
#include "../map/camera.h"
#include "../network/randomsync.h"
#include "../object/objects_list.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"
#include "../weapon/explosion.h"

const uint FORCE_X_MIN = 10;
const uint FORCE_X_MAX = 120;
const uint FORCE_Y_MIN = 1;
const uint FORCE_Y_MAX = 40;

const double OBUS_SPEED = 7 ;

Obus::Obus(AirAttackConfig& cfg,WeaponLauncher * p_launcher) :
  WeaponProjectile("air_attack_projectile", cfg, p_launcher)
{
  is_active = true;
  explode_colliding_character = true;
  Ready();
}

void Obus::SignalCollision()
{ 
  is_active = false; 
  lst_objects.RemoveObject(this);

  if (!IsGhost())
    Explosion();
}

//-----------------------------------------------------------------------------

Plane::Plane(AirAttackConfig &p_cfg) : 
  PhysicalObj("air_attack_plane"),
  cfg(p_cfg)
{
  SetCollisionModel(true, false, false);
  m_alive = GHOST;

  image = resource_manager.LoadSprite( weapons_res_profile, "air_attack_plane");
  SetSize(image->GetSize());
  obus_dx = 100;
  obus_dy = GetY()+GetHeight();
}

void Plane::Shoot(double speed, Point2i& target)
{
  nb_dropped_bombs = 0;
  last_dropped_bomb = NULL;

  Point2d speed_vector ;
  int dir = ActiveCharacter().GetDirection();
  cible_x = target.x;
  SetY (0);

  image->Scale(dir, 1);
   
  Ready();

  if (dir == 1)
    {
      speed_vector.SetValues( speed, 0);
      SetX (-image->GetWidth()+1);
    }
  else
    {
      speed_vector.SetValues( -speed, 0) ;
      SetX (world.GetWidth()-1);
    }

  SetSpeedXY (speed_vector);

  camera.ChangeObjSuivi (this, true, true);

  lst_objects.AddObject(this);
}

void Plane::DropBomb()
{
  Obus * instance = new Obus(cfg,dynamic_cast<WeaponLauncher *>(this));
  instance->SetXY( Point2i(GetX(), obus_dy) );
  
  Point2d speed_vector;
  
  int fx = randomSync.GetLong (FORCE_X_MIN, FORCE_X_MAX);
  fx *= GetDirection();
  int fy = randomSync.GetLong (FORCE_Y_MIN, FORCE_Y_MAX);
  
  speed_vector.SetValues( fx/30.0, fy/30.0);
  instance->SetSpeedXY (speed_vector);
  
  lst_objects.AddObject(instance);
  
  camera.ChangeObjSuivi (instance, true, true);
  
  last_dropped_bomb = instance;
  nb_dropped_bombs++;
}

void Plane::Refresh()
{
  if ( ! IsGhost() ) {

    UpdatePosition();
    
    image->Update();
    
    // First shoot !!
    if ( OnTopOfTarget() && nb_dropped_bombs == 0)
      DropBomb();
    else if (nb_dropped_bombs > 0 &&  nb_dropped_bombs < cfg.nbr_obus) {
      // Get the last rocket and check the position to be sure to not collide with it
      if ( last_dropped_bomb->GetY() > GetY()+GetHeight()+10 )
	DropBomb();
    }
  }
}

int Plane::GetDirection() const 
{ 
  float x,y;
  image->GetScaleFactors(x,y);
  return (x<0)?-1:1;
}

void Plane::Draw()
{
  if (IsGhost()) return;  
  image->Draw(GetPosition());  
}

bool Plane::OnTopOfTarget() const
{
  if (GetDirection() == 1) 
    return (cible_x <= GetX()+obus_dx);
  else
    return (GetX()+(int)image->GetWidth()-obus_dx <= cible_x);
}

void Plane::SignalGhostState (bool was_dead)
{
  lst_objects.RemoveObject(this);
}

//-----------------------------------------------------------------------------

AirAttack::AirAttack() :
  Weapon(WEAPON_AIR_ATTACK, "air_attack",new AirAttackConfig(), ALWAYS_VISIBLE),
  plane(cfg())
{  
  m_name = _("Air attack");
  can_be_used_on_closed_map = false;
}

void AirAttack::Refresh()
{
  m_is_active = false;
}

void AirAttack::ChooseTarget(Point2i mouse_pos)
{
  target = mouse_pos;
  ActiveTeam().GetWeapon().NewActionShoot();
}

bool AirAttack::p_Shoot ()
{
  plane.Shoot (cfg().speed, target);
  return true;
}

AirAttackConfig& AirAttack::cfg() 
{ return static_cast<AirAttackConfig&>(*extra_params); }

//-----------------------------------------------------------------------------

AirAttackConfig::AirAttackConfig()
{
  nbr_obus = 3;
  speed = 7;
}

void AirAttackConfig::LoadXml(xmlpp::Element *elem)
{
  ExplosiveWeaponConfig::LoadXml(elem);
  LitDocXml::LitUint (elem, "nbr_obus", nbr_obus);
  LitDocXml::LitDouble (elem, "speed", speed);
}
