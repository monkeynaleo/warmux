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
#include "../game/game_loop.h"


const uint SNIPE_RIFLE_EXPLOSION_RANGE = 15;
const uint SNIPE_RIFLE_BEAM_START = 40;
const uint SNIPE_RIFLE_BULLET_SPEED = 20;
const uint SNIPE_RIFLE_MAX_BEAM_SIZE = 500;

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
  m_laser_image = new Sprite(resource_manager.LoadImage(weapons_res_profile,m_id+"_laser"));
  int size = SNIPE_RIFLE_MAX_BEAM_SIZE - SNIPE_RIFLE_BEAM_START;
  Surface * tmp = new Surface(Point2i(size,size),SDL_SWSURFACE|SDL_SRCALPHA,true);
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

void SnipeRifle::Refresh()
{
  bool tmp = projectile->is_active;
  WeaponLauncher::Refresh();
  ComputeCrossPoint(!tmp);
}

bool SnipeRifle::ComputeCrossPoint(bool force = false)
{
  // Did the current character is moving ?
  Point2i pos = ActiveCharacter().GetHandPosition();
  double angle = ActiveTeam().crosshair.GetAngleRad();
  if ( !force && last_rifle_pos == pos && last_angle == angle ) return targeting_something;
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
  ActiveTeam().crosshair.ChangeAngleVal(0);
}

// Prepare the laser beam Sprite
void SnipeRifle::PrepareLaserBeam()
{
  uint x_orig,x_dest,y_orig,y_dest;
  Point2i pos = laser_beam_start;
  if (pos.x >= targeted_point.x) {
    laser_beam_pos.x = targeted_point.x;
    x_orig = pos.x - targeted_point.x;
    x_dest = 0;
  } else {
    laser_beam_pos.x = pos.x;
    x_dest = targeted_point.x - pos.x;
    x_orig = 0;
  }
  if (pos.y >= targeted_point.y) {
    laser_beam_pos.y = targeted_point.y;
    y_orig = pos.y - targeted_point.y;
    y_dest = 0;
  } else {
    laser_beam_pos.y = pos.y;
    y_dest = targeted_point.y - pos.y;
    y_orig = 0;
  }
  (*m_laser_beam_image)[0].surface.Fill(0);
  (*m_laser_beam_image)[0].surface.LineColor(x_orig,x_dest,y_orig,y_dest,Color(255,0,0,255));
}

void SnipeRifle::Draw()
{
  if( GameLoop::GetInstance()->ReadState() != GameLoop::PLAYING || IsActive() ) return;
  ComputeCrossPoint();
  m_laser_beam_image->Draw(laser_beam_pos);
  if( targeting_something ) m_laser_image->Draw(targeted_point - (m_laser_image->GetSize()/2));      // Draw the laser impact
  WeaponLauncher::Draw();
}
