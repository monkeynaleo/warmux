
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

#ifndef PARTICLE_H
#define PARTICLE_H
#include "physical_obj.h"
#include "../graphic/sprite.h"
#include "../graphic/surface.h"
#include "../include/base.h"
#include "../weapon/weapon_cfg.h"

typedef enum { 
  particle_SMOKE,
  particle_FIRE,
  particle_STAR
} particle_t;

class Particle : public PhysicalObj
{
 protected:
  uint m_initial_time_to_live;
  uint m_left_time_to_live;

  uint m_time_between_scale;
  uint m_last_refresh;

  Sprite *image;
   
 public:
  Particle(GameLoop &game_loop);
  virtual void Init()=0;
  void Draw();
  void Refresh();
  void Reset(){};
  bool StillUseful();
};

class Smoke : public Particle
{
 public:
  Smoke(GameLoop &game_loop);
  void Init();
};

class StarParticle : public Particle
{
 public:
  StarParticle(GameLoop &game_loop);
  void Init();
};

class FireParticle : public Particle
{
 public: 
  Surface impact;
  FireParticle(GameLoop &game_loop);
  void Init();
  void SignalFallEnding();
};

typedef struct {
  Particle * particle;
  bool upper_objects; // if true displayed on top of characters and weapons
} drawed_particle_t;

class ParticleEngine
{
 private:
  uint m_last_refresh;
  uint m_time_between_add;

  static std::list<drawed_particle_t> lst_particles;

 public:
  ParticleEngine(uint time=100);
  void AddPeriodic(const Point2i &position, 
		   particle_t type,
		   bool upper,
		   double angle=-1, double norme=-1);

  static void AddNow(const Point2i &position, 
		     uint nb_particles, particle_t type,
		     bool upper,
		     double angle=-1, double norme=-1);

  static void Refresh();
  static void Draw(bool upper);
  static void Stop();
};

#endif
