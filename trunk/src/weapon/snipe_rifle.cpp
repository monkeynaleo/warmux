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
 * Snipe Rifle
 *****************************************************************************/

#include "snipe_rifle.h"
#include <sstream>
#include "explosion.h"
#include "../game/time.h"
#include "../interface/game_msg.h"
#include "../map/map.h"
#include "../object/objects_list.h"
#include "../team/teams_list.h"
#include "../tool/i18n.h"


const double SOUFFLE_BALLE = 2;
const double MIN_TIME_BETWEEN_SHOOT = 1000; // in milliseconds

SnipeBullet::SnipeBullet(ExplosiveWeaponConfig& cfg) :
    WeaponBullet("snipe_rifle_bullet", cfg)
{ 
  cfg.explosion_range = 15;
}

void SnipeBullet::ShootSound()
{
  jukebox.Play("share","weapon/gun");
}

//-----------------------------------------------------------------------------

SnipeRifle::SnipeRifle() : WeaponLauncher(WEAPON_SNIPE_RIFLE,"snipe_rifle", new ExplosiveWeaponConfig())
{
  m_name = _("Snipe Rifle");
  override_keys = true ;

  m_first_shoot = 0;
  last_angle = 0.0;

  projectile = new SnipeBullet(cfg());
  cross_point = new Point2i();
  targeting_something = false;
  m_laser_image = new Sprite( resource_manager.LoadImage(weapons_res_profile,m_id+"_laser"));
}

bool SnipeRifle::p_Shoot()
{
  if (m_is_active)
    return false;  

  m_is_active = true;
  projectile->Shoot (20);

  return true;
}

bool SnipeRifle::ComputeCrossPoint()
{
  // Did the current character is moving ?
  Point2i pos = ActiveCharacter().GetHandPosition();
  double angle = ActiveTeam().crosshair.GetAngleRad();
  if ( (last_rifle_pos == pos) && (last_angle == angle)) return targeting_something;
  else {
    last_rifle_pos=pos;
    last_angle=angle;
  }

  // Get the target point
  // Set the initial position.
  // Equation of movement : y = ax + b
  double a, b;
  a = sin(angle)/cos(angle);
  b = pos.y - ( a * pos.x ) ;
  Point2i delta_pos;
  // While test is not finished
  while( true ){
    // going upwards ( -3pi/4 < angle <-pi/4 ) 
    if (angle < -0.78 && angle > -2.36){ 
      pos.x = (int)((pos.y-b)/a);       //Calculate x
      delta_pos.y=-1;                   //Increment y
    // going downwards ( 3pi/4 > angle > pi/4 )
    } else if (angle > 0.78 && angle < 2.36){ 
      pos.x = (int)((pos.y-b)/a);       //Calculate x
      delta_pos.y=1;                    //Increment y
    // else going at right or left
    } else {
      pos.y = (int)((a*pos.x) + b);   //Calculate y
      delta_pos.x=ActiveCharacter().GetDirection();   //Increment x
    }

    // the point is outside the map
    if ( ( world.EstHorsMondeX(pos.x) ) || ( world.EstHorsMondeY(pos.y) )) {
      *cross_point=pos;
      return targeting_something = false;
    }

    // is there a collision ??
    if(projectile->CollisionTest( pos ) ){
      *cross_point=pos;
      return targeting_something = true;
    }
    pos += delta_pos;
  }
  return targeting_something = false;
}

Point2i * SnipeRifle::GetCrossPoint()
{
  return cross_point;
}

bool SnipeRifle::isTargetingSomething()
{
  return targeting_something;
}

void SnipeRifle::Draw()
{
  if( IsActive() ) return ;
  ComputeCrossPoint();
  if(targeting_something) m_laser_image->Draw(*GetCrossPoint());      // Draw the laser impact
  WeaponLauncher::Draw();
}
