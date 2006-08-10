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
  //m_laser_image = NULL ;
  m_laser_image = new Sprite( resource_manager.LoadImage(weapons_res_profile,m_id+"_laser"));
}

void SnipeRifle::RepeatShoot()
{  
  uint time = Time::GetInstance()->Read() - m_first_shoot; 
  uint tmp = Time::GetInstance()->Read();

  if (time >= MIN_TIME_BETWEEN_SHOOT)
  {
    m_is_active = false;
    NewActionShoot();
    m_first_shoot = tmp;
  }
}

bool SnipeRifle::p_Shoot()
{
  jukebox.Play("share", "weapon/uzi"); // TODO : Change this sound for another ?
  
  // Calculate movement of the bullet
  // Set the initial position.
  Point2i pos = ActiveCharacter().GetHandPosition();

  // Equation of movement : y = ax + b
  double angle, a, b;
  angle = ActiveTeam().crosshair.GetAngleRad();
  a = sin(angle)/cos(angle);
  b = pos.y - ( a * pos.x ) ;
  Point2i delta_pos ;

  // Add a particle for the empty bullet
  particle.AddNow(pos, 1, particle_BULLET, true, angle + M_PI + ActiveCharacter().GetDirection() * M_PI_4,
                  5.0 + (Time::GetInstance()->Read() % 6));

  // If we already shooted in this direction reuse the last trajectorie of the bullet
  if(m_first_shoot!=0 && last_angle==angle && ActiveCharacter().GetHandPosition()==last_rifle_pos)
    pos = last_bullet_pos;

  // Move the bullet !!
  projectile->SetXY( pos );
  projectile->Ready();
  projectile->is_active = true;
  
  while( projectile->is_active ){
	
	// shooting upwards  ( -3pi/4 < angle <-pi/4 ) 
	if (angle < -0.78 && angle > -2.36){ 
		pos.x = (int)((pos.y-b)/a);  	//Calculate x
		delta_pos.y=-1;			//Increment y
	//shooting downwards ( 3pi/4 > angle > pi/4 )
	}else if (angle > 0.78 && angle < 2.36){ 
		pos.x = (int)((pos.y-b)/a);	//Calculate x
		delta_pos.y=1;			//Increment y
	// else shooting at right or left 
	}else{ 
    		pos.y = (int)((a*pos.x) + b);	//Calculate y
		delta_pos.x=ActiveCharacter().GetDirection();	//Increment x
	}

    projectile->SetXY( pos );

    // the bullet in gone outside the map
   if ( ( world.EstHorsMondeX(projectile->GetX()) ) || ( world.EstHorsMondeY(projectile->GetY()) )) {  //IsGhost does not check the Y side.
      projectile->is_active=false;
      last_angle = angle;
      last_bullet_pos = pos;
      last_rifle_pos = ActiveCharacter().GetHandPosition();
      return true;
    }

    // is there a collision ??
    if(projectile->CollisionTest( pos ) ){
      projectile->is_active=false;
      last_angle = angle;
      last_bullet_pos = pos;
      last_rifle_pos = ActiveCharacter().GetHandPosition();
      projectile->Explosion();
      return true;
    }
  pos += delta_pos;
  }

  return true;
}

bool SnipeRifle::ComputeCrossPoint()
{
  // Get the target point
  // Set the initial position.
  Point2i pos = ActiveCharacter().GetHandPosition();

  // Equation of movement : y = ax + b
  double angle, a, b;
  angle = ActiveTeam().crosshair.GetAngleRad();
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
      return false;
    }

    // is there a collision ??
    if(projectile->CollisionTest( pos ) ){
      *cross_point=pos;
      printf("%d %d\n",cross_point->GetX(),cross_point->GetY());
      return true;
    }
    pos += delta_pos;
  }
  return false;
}

Point2i * SnipeRifle::GetCrossPoint()
{
  if (ComputeCrossPoint()) return cross_point;
  else return NULL;
}

void SnipeRifle::HandleKeyEvent(int action, int event_type)
{
  GetCrossPoint();
  switch (action) {    

  case ACTION_SHOOT:
    if (event_type == KEY_REFRESH)
      RepeatShoot();

    if (event_type == KEY_RELEASED)
      m_is_active = false;

    break;
  default:
    break;
  };
}

void SnipeRifle::Draw()
{
  WeaponLauncher::Draw();
  if(GetCrossPoint() != NULL)
    m_laser_image->Draw(*GetCrossPoint());
}
