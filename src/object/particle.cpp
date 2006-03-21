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
 * Particle Engine
 *****************************************************************************/

#include "particle.h"
#include <SDL.h>
#include <map>
#include "../game/time.h"
#include "../graphic/sprite.h"
#include "../include/app.h"
#include "../object/objects_list.h"
#include "../tool/resource_manager.h"
#include "../tool/random.h"
#include "../tool/point.h"
#include "../weapon/weapon_tools.h"

ParticleEngine global_particle_engine;

Particle::Particle() :
  PhysicalObj("Particle", 0.0)
{ 
  m_type = objUNBREAKABLE;
  m_wind_factor = 0.8;
  m_air_resist_factor = 0.2;
  m_rebounding = 0;

  m_initial_time_to_live = 20;
  m_left_time_to_live = 0;
  m_last_refresh = Time::GetInstance()->Read();
}

void Particle::Draw()
{
  if (m_left_time_to_live > 0) 
    image->Draw(GetPosition());
}

void Particle::Refresh()
{
  uint time = Time::GetInstance()->Read() - m_last_refresh; 

  UpdatePosition();

  image->Update();

  if (time >= m_time_between_scale) {  

    //assert(m_left_time_to_live > 0);
    if (m_left_time_to_live <= 0) return ;

    m_left_time_to_live--;

    float lived_time = m_initial_time_to_live - m_left_time_to_live;

    //during the 1st quarter of the time increase size of particle
    //after the 1st quarter, decrease the alpha value
    if((float)lived_time<m_initial_time_to_live/2.0)
    {
      float coeff = sin((M_PI/2.0)*((float)lived_time/((float)m_initial_time_to_live/2.0)));
      image->Scale(coeff,coeff);  
      SetSize(image->GetSize());
      image->SetAlpha(1.0);
    }
    else
    {
      float alpha = 1.0 - sin((M_PI/2.0)*((float)lived_time-((float)m_initial_time_to_live/2.0))/((float)m_initial_time_to_live/2.0));
      image->Scale(1.0,1.0);
      image->SetAlpha(alpha);
    }
    m_last_refresh = Time::GetInstance()->Read() ;
  }
}

bool Particle::StillUseful()
{
  return (m_left_time_to_live > 0);
}

Smoke::Smoke() :
  Particle()
{
  m_name="Smoke";
  SetMass(0.5);
  SetGravityFactor(-1.0);

  m_initial_time_to_live = 10;
  m_left_time_to_live = m_initial_time_to_live; 
  m_time_between_scale = 100;
}

void Smoke::Init()
{
  Profile *res = resource_manager.LoadXMLProfile( "weapons.xml", false);
  image = resource_manager.LoadSprite(res,"smoke"); 
  resource_manager.UnLoadXMLProfile( res);
   
  m_initial_time_to_live = 10;
  m_left_time_to_live = m_initial_time_to_live; 

  image->Scale(0.0,0.0);
  SetSize( Point2i(1, 1) );
}

StarParticle::StarParticle() :
  Particle()
{
  m_name="StarParticle";
  SetMass(0.5);  
  SetGravityFactor(0.0);
  m_wind_factor = 0.2;
  m_initial_time_to_live = 30;
  m_left_time_to_live = m_initial_time_to_live; 
  m_time_between_scale = 50;
}

void StarParticle::Init()
{
  Profile *res = resource_manager.LoadXMLProfile( "weapons.xml", false);
  image = resource_manager.LoadSprite(res,"star_particle"); 
  resource_manager.UnLoadXMLProfile( res);

  image->Scale(0.0, 0.0);
  SetSize( Point2i(1, 1) );
}

ExplosiveWeaponConfig fire_cfg;

FireParticle::FireParticle() :
  Particle()
{
  m_name="FireParticle";
  SetMass(0.5);
  m_type = objCLASSIC;
  m_rebounding = false;
  m_wind_factor = 0.2;

  m_initial_time_to_live = 15;
  m_left_time_to_live = m_initial_time_to_live; 
  m_time_between_scale = 50;
  fire_cfg.damage = 1;
}

void FireParticle::Init()
{
  Profile *res = resource_manager.LoadXMLProfile( "weapons.xml", false);
  image = resource_manager.LoadSprite(res,"fire_particle");
  impact = resource_manager.LoadImage(res,"fire_impact");
  resource_manager.UnLoadXMLProfile( res);

  image->Scale(0.0,0.0);
  SetSize( Point2i(1, 1) );
}

void FireParticle::SignalFallEnding()
{
  Point2i pos = GetCenter();
  AppliqueExplosion (pos, pos, impact, fire_cfg, NULL, "", false);
  
  m_left_time_to_live = 0;
}

//-----------------------------------------------------------------------------

ParticleEngine::ParticleEngine(uint time)
{
  m_time_between_add = time ;
}


void ParticleEngine::AddPeriodic(const Point2i &position, particle_t type,
				 bool upper,
				 double angle, double norme)
{
  // time spent since last refresh (in milliseconds)
  uint time = Time::GetInstance()->Read() - m_last_refresh; 
  uint tmp = Time::GetInstance()->Read();

  uint delta = uint(m_time_between_add * double(randomObj.GetLong(3,40))/10);
  if (time >= delta) {
    m_last_refresh = tmp;
    ParticleEngine::AddNow(position, 1, type, upper, angle, norme);
  }
}

//-----------------------------------------------------------------------------
// Static methods 
  
std::list<drawed_particle_t> ParticleEngine::lst_particles;

void ParticleEngine::AddNow(const Point2i &position,
			    uint nb_particles, particle_t type, 
			    bool upper,
			    double angle, double norme)
{
  Particle *particle = NULL;
  double tmp_angle, tmp_norme;

  for (uint i=0 ; i < nb_particles ; i++) {
    switch (type) {
    case particle_SMOKE : particle = new Smoke();
      break;
    case particle_FIRE : particle = new FireParticle();
      break;
    case particle_STAR : particle = new StarParticle();
      break;
    default : particle = NULL;
      break;
    }
  
    if (particle != NULL) {
      if( norme == -1 )
		  tmp_norme = double(randomObj.GetLong(0, 5000))/1000;
      else 
		  tmp_norme = norme;

      if( angle == -1 )
		  tmp_angle = - double(randomObj.GetLong(0, 3000))/1000;
      else 
		  tmp_angle = angle;
      
      drawed_particle_t p;
      p.particle = particle;
      p.upper_objects = upper;
	
      particle->Init();
      particle->SetXY(position);
      particle->SetSpeed(tmp_norme, tmp_angle);
      lst_particles.push_back(p);
    }
  }
}

void ParticleEngine::Draw(bool upper)
{
  std::list<drawed_particle_t>::iterator it;
  // draw the particles
  for (it=lst_particles.begin(); it!=lst_particles.end(); ++it){
    if ( (*it).upper_objects == upper ) {
      (*it).particle->Draw();
    }
  }

}

void ParticleEngine::Refresh()
{
  // remove old particles 
  std::list<drawed_particle_t>::iterator it=lst_particles.begin(), end=lst_particles.end(), current;
  while (it != end) {
    current = it;
    ++it;

    if (! (*current).particle->StillUseful()) {
      delete (*current).particle;
      lst_particles.erase(current);
      if (it==end) break;
    }   
  }

  // update the particles
  for(it=lst_particles.begin(); it!=lst_particles.end(); ++it) {
    (*it).particle->Refresh();
  }
}

void ParticleEngine::Stop()
{
  // remove all the particles 
  std::list<drawed_particle_t>::iterator it=lst_particles.begin(), end=lst_particles.end(), current;
  while (it != end) {
    current = it;
    ++it;
    
    delete (*current).particle;
    lst_particles.erase(current);
    if (it==end)
      break;
  }
}

