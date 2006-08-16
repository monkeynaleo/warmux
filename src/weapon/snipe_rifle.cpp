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
 * Snipe Rifle. Overide the Draw method in order to draw the laser beam.
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


const uint SNIPE_RIFLE_EXPLOSION_RANGE = 15;
const uint SNIPE_RIFLE_BEAM_START = 40;
const uint SNIPE_RIFLE_BULLET_SPEED = 50;
const uint SNIPE_RIFLE_MAX_BEAM_SIZE = 500 - SNIPE_RIFLE_BEAM_START;

SnipeBullet::SnipeBullet(ExplosiveWeaponConfig& cfg) :
    WeaponBullet("snipe_rifle_bullet", cfg)
{ 
  cfg.explosion_range = SNIPE_RIFLE_EXPLOSION_RANGE;
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
  last_angle = 0.0;
  projectile = new SnipeBullet(cfg());
  targeting_something = false;
  m_laser_image = new Sprite( resource_manager.LoadImage(weapons_res_profile,m_id+"_laser"));
  Surface * tmp = new Surface(Point2i(SNIPE_RIFLE_MAX_BEAM_SIZE,SNIPE_RIFLE_MAX_BEAM_SIZE),SDL_SWSURFACE,true);
  m_laser_beam_image = new Sprite(*tmp);
}

bool SnipeRifle::p_Shoot()
{
  if (m_is_active)
    return false;  

  m_is_active = true;
  projectile->Shoot (SNIPE_RIFLE_BULLET_SPEED);

  return true;
}

bool SnipeRifle::ComputeCrossPoint()
{
  // Did the current character is moving ?
  Point2i pos = ActiveCharacter().GetHandPosition();
  double angle = ActiveTeam().crosshair.GetAngleRad();
  if ( last_rifle_pos == pos && last_angle == angle ) return targeting_something;
  else {
    last_rifle_pos=pos;
    last_angle=angle;
  }

  // Equation of movement : y = ax + b
  double a, b;
  a = sin(angle) / cos(angle);
  b = pos.y - ( a * pos.x );
  Point2i delta_pos, size, start_point;
  start_point = pos;
  uint distance = 0;
  targeting_something = false;
  // While test is not finished
  while( distance < SNIPE_RIFLE_MAX_BEAM_SIZE ){
    // going upwards ( -3pi/4 < angle <-pi/4 )
    if (angle < -0.78 && angle > -2.36){
      pos.x = (int)((pos.y-b)/a);       //Calculate x
      delta_pos.y = -1;                   //Increment y
    // going downwards ( 3pi/4 > angle > pi/4 )
    } else if (angle > 0.78 && angle < 2.36){
      pos.x = (int)((pos.y-b)/a);       //Calculate x
      delta_pos.y = 1;                    //Increment y
    // else going at right or left
    } else {
      pos.y = (int)((a*pos.x) + b);   //Calculate y
      delta_pos.x = ActiveCharacter().GetDirection();   //Increment x
    }
    // start point of the laser beam
    if ( distance < SNIPE_RIFLE_BEAM_START ) laser_beam_start = pos;

    // the point is outside the map
    if ( world.EstHorsMondeX(pos.x) || world.EstHorsMondeY(pos.y) ) break;

    // is there a collision ??
    if ( distance > 30 && projectile->CollisionTest( pos )){
      targeting_something = true;
      break;
    }
    pos += delta_pos;
    distance = (int) start_point.Distance(pos);
  }
  targeted_point=pos;
  PrepareLaserBeam();
  return targeting_something;
}

// Reset crosshair when switching from a weapon to another to avoid misused
void SnipeRifle::p_Deselect()
{
  ActiveTeam().crosshair.Reset();
}

// Prepare the laser beam Sprite
void SnipeRifle::PrepareLaserBeam()
{
  Color red = Color (255,0,0,255);
  uint x1,x2,x_orig,x_dest,y1,y2,y_orig,y_dest;
  Point2i pos = laser_beam_start;
  if (pos.x >= targeted_point.x) {
    x1 = pos.x;
    laser_beam_pos.x = x2 = targeted_point.x;
    x_orig = x1 - x2;
    x_dest = 0;
  } else {
    laser_beam_pos.x = x2 = pos.x;
    x1 = targeted_point.x;
    x_dest = x1 - x2;
    x_orig = 0;
  }
  if (pos.y >= targeted_point.y) {
    y1 = pos.y;
    laser_beam_pos.y = y2 = targeted_point.y;
    y_orig = y1 - y2;
    y_dest = 0;
  } else {
    laser_beam_pos.y = y2 = pos.y;
    y1 = targeted_point.y;
    y_dest = y1 - y2;
    y_orig = 0;
  }
  Point2i size = Point2i( x1 - x2 + 1 , y1 - y2 + 1 );
  (*m_laser_beam_image)[0].surface.Fill(0);
  (*m_laser_beam_image)[0].surface.LineColor(x_orig,x_dest,y_orig,y_dest,red);
}

void SnipeRifle::Draw()
{
  if( IsActive() ) return ;
  ComputeCrossPoint();
  m_laser_beam_image->Draw(laser_beam_pos);
  if(targeting_something) m_laser_image->Draw(targeted_point);      // Draw the laser impact
  WeaponLauncher::Draw();
}
