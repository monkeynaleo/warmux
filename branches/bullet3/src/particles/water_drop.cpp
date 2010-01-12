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

#include "game/time.h"
#include "graphic/sprite.h"
#include "particles/water_drop.h"
#include "particles/particle.h"
#include "tool/resource_manager.h"

const uint living_time = 5000;

WaterParticle::WaterParticle() :
  Particle("water_particle")
{
  // TODO physic SetCollisionModel(true, false, false) does not match:
  GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_GROUND,false);
  GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_CHARACTER,false);
  GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_ITEM,false);
  GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_PROJECTILE,false);
  m_left_time_to_live = 100;
  m_check_move_on_end_turn = false;

  Profile *res = GetResourceManager().LoadXMLProfile( "weapons.xml", false);
  image = GetResourceManager().LoadSprite(res, ActiveMap()->GetWaterType() + "_drop");
  GetResourceManager().UnLoadXMLProfile(res);

  image->SetRotation_HotSpot(bottom_center);
  // TODO physic
//  SetBasicShape(image->GetSize(), GetInitialMass());
}

WaterParticle::~WaterParticle()
{
}

void WaterParticle::Refresh()
{
  uint now = Time::GetInstance()->Read();
  UpdatePosition();
  image->Update();

  if (image->GetSize().x != 0 && image->GetSize().y != 0)
  {
    // TODO physic:
    //int dx = (GetWidth() - image->GetWidth()) / 2;
    //int dy = std::max(0, GetHeight() - 2);
    //SetTestRect(dx, dx, dy, 1);
  }

  double angle = GetSpeedAngle();
  image->SetRotation_rad((angle - M_PI_2));

  m_last_refresh = now;
}

void WaterParticle::Draw()
{
  Point2i draw_pos = GetPhysic()->GetPosition();
  draw_pos.y += GetSize().y/2;
  image->Draw( draw_pos );
}

void WaterParticle::SignalDrowning()
{
  m_left_time_to_live = 0;
}

void WaterParticle::SignalOutOfMap()
{
  m_left_time_to_live = 0;
}

ClearWaterParticle::ClearWaterParticle()
{
  // delete std water image
  delete image;
  image = ParticleEngine::GetSprite(CLEARWATER_spr);
}

LavaParticle::LavaParticle()
{
  // delete std water image
  delete image;
  image = ParticleEngine::GetSprite(LAVA_spr);
}

RadioactiveParticle::RadioactiveParticle()
{
  // delete std water image
  delete image;
  image = ParticleEngine::GetSprite(RADIOACTIVE_spr);
}

DirtyWaterParticle::DirtyWaterParticle()
{
  // delete std water image
  delete image;
  image = ParticleEngine::GetSprite(DIRTYWATER_spr);
}

ChocolateWaterParticle::ChocolateWaterParticle()
{
  // delete std water image
  delete image;
  image = ParticleEngine::GetSprite(CHOCOLATEWATER_spr);
}
