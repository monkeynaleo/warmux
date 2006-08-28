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

#include "fire.h"
#include "particle.h"
#include "../network/randomsync.h"

ExplosiveWeaponConfig fire_cfg;

FireParticle::FireParticle() :
  Particle("fire_particle")
{
  SetCollisionModel(false, false, false);
  m_initial_time_to_live = 15;
  m_left_time_to_live = m_initial_time_to_live;
  m_time_between_scale = 50;
  fire_cfg.damage = 1;
  fire_cfg.explosion_range = 5;
  direction = randomSync.GetBool() ? -1 : 1;
  image = ParticleEngine::GetSprite(FIRE_spr);
  SetSize( Point2i(1, 1) );
}

void FireParticle::SignalFallEnding()
{
//  Point2i pos = GetCenter();
//  ApplyExplosion (pos, fire_cfg, "", false, ParticleEngine::NoESmoke);

  m_left_time_to_live = 0;
}

void FireParticle::Refresh()
{
  Particle::Refresh();
  double angle = GetSpeedAngle() * 180/M_PI ;
  image->SetRotation_deg( angle * direction);
  image->Scale(direction, 1.0);
}
