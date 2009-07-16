/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2009 Wormux Team.
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
 * Particle Engine
 *****************************************************************************/

#include "particles/fire.h"
#include "particles/particle.h"
#include "game/time.h"
#include "graphic/sprite.h"
#include "network/randomsync.h"
#include "sound/jukebox.h"
#include <WORMUX_random.h>
#include "weapon/explosion.h"
#include "weapon/weapon_cfg.h"

const uint living_time = 4000;
const uint dig_ground_time = 1000;

ExplosiveWeaponConfig fire_cfg;

FireParticle::FireParticle() :
  Particle("fire_particle"),
  creation_time(Time::GetInstance()->Read()),
  on_ground(false),
  oscil_delta(RandomLocal().GetLong(0, dig_ground_time))
{
  m_rebound_count = RandomSync().GetInt(1, 4);
  m_living_time = RandomSync().GetUint(living_time/4, living_time);
  m_vivacity = 500;
  m_left_time_to_live = 100;
  m_check_move_on_end_turn = true;
  m_is_fire = true;

  fire_cfg.damage = 2;
  fire_cfg.explosion_range = 10;
  fire_cfg.blast_range = 0;
  fire_cfg.blast_force = 0;
  fire_cfg.particle_range = 10;

  image = ParticleEngine::GetSprite(FIRE_spr);
  image->SetRotation_HotSpot(bottom_center);
  SetSphericalShape(image->GetSize().x/2, GetInitialMass());
  SetCollisionCategory(PROJECTILE);
  SetCollisionModel(true,true,true,true);

  Generate();
}

FireParticle::~FireParticle()
{
}

#include <iostream>

void FireParticle::Refresh()
{
  uint now = Time::GetInstance()->Read();
  //UpdatePosition();
  image->Update();

  if (creation_time + m_living_time < now)
  {
    m_left_time_to_live = 0;
  }

  if(now % 100 == 0)
  {
  if(RandomSync().GetInt(0,10000)< m_vivacity)
  {
    if(m_rebound_count == 0 || IsColliding())
	{
	  Split();
	  m_vivacity -= 100;
	  m_living_time += 300;
	}
  }
  }

  if (on_ground || IsColliding())
  {
    if (!on_ground) {
      JukeBox::GetInstance()->Play("default","fire/touch_ground");
    }
    on_ground = true;
  }
 

  double oscil_max =0.20; // 1 = 180°
  double oscil_period = 2000;
  std::cout<<"Now   : "<<now-m_living_time<<std::endl;
  double oscil_pourcentage = (2 * double((now+m_living_time) % (uint) oscil_period)/oscil_period) - 1; //range [-1, 1]
  if (oscil_pourcentage<0) { oscil_pourcentage *= -1;} //range [0,1]
  std::cout<<"Perc  : "<<oscil_pourcentage<<std::endl;
  std::cout<<"Cos   : "<<( (2*oscil_pourcentage - 1) * oscil_max)<<std::endl;

  double angle =  acos( (2*oscil_pourcentage - 1) * oscil_max); // move range from [0,1] to [-oscil_max,oscil_max]
  std::cout<<"Angle : "<<angle<<std::endl;
  std::cout<<"Angle : "<<360*angle/(2 * M_PI)<<std::endl;
  
  image->SetRotation_rad( angle - M_PI /2);
  


  m_last_refresh = now;
}

void FireParticle::Draw()
{
  Point2i draw_pos = GetPosition();
  draw_pos.y -=  (image->GetSize().y/2)-4;
  draw_pos.x -=  image->GetSize().x/2;

  image->Draw( draw_pos );
}

void FireParticle::SignalDrowning()
{
  m_left_time_to_live = 0;
  // JukeBox::GetInstance()->Play("default","fire/pschiit");
}

void FireParticle::SignalOutOfMap()
{
  m_left_time_to_live = 0;
}

void FireParticle::SignalGroundCollision(const Point2d&)
{

  m_rebound_count--;
  if(m_rebound_count == 0)
  {
    SetSphericalShape(image->GetSize().x/2, 0);
    SetCollisionCategory(PROJECTILE);
    SetCollisionModel(true,false,false,true);
    Generate();
  }

  if(m_rebound_count >= 0)
  {
      Point2i expl_pos = GetPosition();
	  expl_pos.y -= 4;
      ApplyExplosion(expl_pos, fire_cfg, "", false, ParticleEngine::LittleESmoke);
  }

}

void FireParticle::SignalObjectCollision(GameObj * obj,
		                                       PhysicalShape * /*shape*/,
											   const Point2d& my_speed_before)
{
  obj->SetEnergyDelta(-1);
  obj->AddSpeed(1, my_speed_before.ComputeAngle());
}

void FireParticle::Split()
{

  FireParticle * fire = new FireParticle();
  fire->SetXY(GetPosition());
  fire->SetOnTop(true);
  fire->SetVivacity(m_vivacity - 100);
  SetOverlappingObject(fire, 1000);

  double x = RandomSync().GetDouble(-10,10);
  double y = RandomSync().GetDouble(-10,-5);
  fire->SetSpeedXY(Point2d(x, y));
  //fire->SetSpeed(10,  M_PI);

  ParticleEngine::AddNow(fire);
  ParticleEngine::AddNow(GetPosition(), 2, particle_SMOKE, true, 0, 1);


}

