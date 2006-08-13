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
#include "../include/app.h"


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
  last_angle = -361.0;

  projectile = new SnipeBullet(cfg());
  cross_point = new Point2i();
  targeting_something = false;
  m_laser_image = new Sprite( resource_manager.LoadImage(weapons_res_profile,m_id+"_laser"));
  m_laser_beam_image=NULL;
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
      targeting_something = false;
      break;
    }

    // is there a collision ??
    if(projectile->CollisionTest( pos ) ){
      targeting_something = true;
      break;
    }
    pos += delta_pos;
  }
  *cross_point=pos;
  PrepareLaserBeam();
  return targeting_something;
}

Point2i * SnipeRifle::GetCrossPoint()
{
  return cross_point;
}

bool SnipeRifle::isTargetingSomething()
{
  return targeting_something;
}

void SnipeRifle::PrepareLaserBeam()
{
  Color red = Color (255,0,0,255);
  uint x1,x2,x_max,x_min,x_orig,x_dest,y1,y2,y_max,y_min,y_orig,y_dest;
  Point2i pos = ActiveCharacter().GetHandPosition();
  if (pos.GetX() >= cross_point->GetX()) {
    x_max=x1=pos.GetX();
    x_min=x2=cross_point->GetX();
    x_orig=x1 - x2; x_dest=1;
  } else {
    x_min=x2=pos.GetX();
    x_max=x1=cross_point->GetX();
    x_dest=x1 - x2; x_orig=1;
  }
  if (pos.GetY() >= cross_point->GetY()) {
    y_max=y1=pos.GetY();
    y_min=y2=cross_point->GetY();
    y_orig=y1 - y2; y_dest=1;
  } else {
    y_min=y2=pos.GetY();
    y_max=y1=cross_point->GetY();
    y_dest=y1 - y2; y_orig=1;
  }
  Point2i size = Point2i( x1 - x2 + 1 , y1 - y2 + 1 );
  laser_beam_pos = Point2i(x_min,y_min);
  Surface laser_beam_surface = Surface(size,SDL_SWSURFACE,true);
  laser_beam_surface.LineColor(x_orig,x_dest,y_orig,y_dest,red);
  if ( m_laser_beam_image != NULL ) delete(m_laser_beam_image);
  m_laser_beam_image = new Sprite(laser_beam_surface);
}

void SnipeRifle::Draw()
{
  if( IsActive() ) return ;
  ComputeCrossPoint();
  m_laser_beam_image->Draw(laser_beam_pos);
  if(targeting_something) m_laser_image->Draw(*cross_point);      // Draw the laser impact
  WeaponLauncher::Draw();
}
