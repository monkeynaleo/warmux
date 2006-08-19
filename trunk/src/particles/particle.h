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
#include "../object/physical_obj.h"
#include "../graphic/sprite.h"
#include "../graphic/surface.h"
#include "../include/base.h"
#include "../weapon/weapon_cfg.h"

typedef enum {
  particle_DARK_SMOKE,
  particle_SMOKE,
  particle_FIRE,
  particle_STAR,
  particle_BULLET,
  particle_GROUND,
  particle_AIR_HAMMER,
  particle_MAGIC_STAR,
  particle_BODY_MEMBER
} particle_t;

enum particle_spr{ SMOKE_spr, EXPLOSION_SMOKE_spr, FIRE_spr, STAR_spr, MAGIC_STAR_R_spr, MAGIC_STAR_Y_spr, MAGIC_STAR_B_spr, DARK_SMOKE_spr, BULLET_spr };
const int particle_spr_nbr = 9;

class Particle : public PhysicalObj
{
 protected:
  bool on_top; // if true displayed on top of characters and weapons
  uint m_initial_time_to_live;
  uint m_left_time_to_live;

  uint m_time_between_scale;
  uint m_last_refresh;

  Sprite *image;

 public:
  Particle(const std::string &name);
  ~Particle();
  virtual void Draw();
  virtual void Refresh();
  void SetOnTop(bool b) { on_top = b; }
  bool IsOnTop() { return on_top; }
  bool StillUseful();
};

class ExplosionSmoke : public Particle
{
  uint m_initial_size, dx;
  float mvt_freq;
 public:
  ExplosionSmoke(const uint size_init);
  void Refresh();
  void Draw();
};

class Smoke : public Particle
{
 public:
  Smoke();
};

class DarkSmoke : public Particle
{
 public:
  DarkSmoke();
};

class StarParticle : public Particle
{
 public:
  StarParticle();
};

class BulletParticle : public Particle
{
 public:
  BulletParticle();
  void Refresh();
  void SignalRebound();
};

class MagicStarParticle: public Particle
{
 public:
  MagicStarParticle();
  void Refresh();
};

class FireParticle : public Particle
{
 public:
  FireParticle();
  void SignalFallEnding();
};

class GroundParticle : public Particle
{
 public:
  GroundParticle(const Point2i& size, const Point2i& position);
  void Refresh();
};

class BodyMemberParticle : public Particle
{
  int angle;
 public:
  BodyMemberParticle(Sprite* spr, const Point2i& position);
  void Refresh();
};

class ParticleEngine
{
 private:
  uint m_last_refresh;
  uint m_time_between_add;

  static Sprite* particle_sprite[particle_spr_nbr];
  static std::list<Particle *> lst_particles;

  static void AddLittleESmoke(const Point2i &pos, const uint &radius);
  static void AddBigESmoke(const Point2i &pos, const uint &radius);

 public:
  ParticleEngine(uint time=100);
  void AddPeriodic(const Point2i &position,
		   particle_t type,
		   bool upper,
		   double angle=-1, double norme=-1);

  static void Init();
  static void FreeMem();
  static Sprite* GetSprite(particle_spr type);

  static void AddNow(const Point2i &position,
		     uint nb_particles, particle_t type,
		     bool upper,
		     double angle=-1, double norme=-1);
  static void AddNow(Particle* particle);

  enum ESmokeStyle { NoESmoke, LittleESmoke, BigESmoke }; // Style of smoke explosion (quantitie of smoke)
  static void AddExplosionSmoke(const Point2i &pos, const uint &radius, ESmokeStyle &style);

  static void Refresh();
  static void Draw(bool upper);
  static void Stop();
};

#endif
