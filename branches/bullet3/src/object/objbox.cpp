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
 * Generic Box that falls from they sky.
 *****************************************************************************/

#include "object/objbox.h"
#include <sstream>
#include <iostream>
#include "character/character.h"
#include "game/game_mode.h"
#include "game/game.h"
#include "game/time.h"
#include "graphic/sprite.h"
#include "include/app.h"
#include "include/action.h"
#include "interface/game_msg.h"
#include "map/camera.h"
#include "map/map.h"
#include "network/randomsync.h"
#include "object/objects_list.h"
#include "physic/physical_shape.h"
#include "sound/jukebox.h"
#include "team/macro.h"
#include "team/team.h"
#include <WORMUX_debug.h>
#include "tool/resource_manager.h"
#include "weapon/explosion.h"

const uint SPEED = 5; // meter / seconde

ObjBox::ObjBox(const std::string &name)
  : GameObj(name) {
  m_allow_negative_y = true;

  parachute = true;
  SetEnergy(start_life_points);

  GetPhysic()->SetSpeed (SPEED, M_PI_2);
  GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_GROUND,true);
  GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_CHARACTER,false);
  GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_ITEM,true);
  GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_PROJECTILE,true);
  GetPhysic()->SetEnabled(true);

  JukeBox::GetInstance()->Play("default","box/falling");
}

ObjBox::~ObjBox()
{
  delete anim;
  Game::GetInstance()->SetCurrentBox(NULL);
}

void ObjBox::CloseParachute()
{
  GetPhysic()->SetAirFrictionFactor(0.0);
  Game::GetInstance()->SetCurrentBox(NULL);
  MSG_DEBUG("box", "End of the fall: parachute=%d", parachute);
  hit.Play("default", "box/hitting_ground");
  if (!parachute) return;

  MSG_DEBUG("box", "Start of the animation 'fold of the parachute'.");
  parachute = false;
  anim->SetCurrentFrame(0);
  anim->Start();
}

void ObjBox::SignalGroundCollision(const Point2d& /*my_speed_before*/)
{
  CloseParachute();
}

void ObjBox::SignalObjectCollision(const Point2d& my_speed_before,
				   GameObj * obj,
				   const Point2d& /*obj_speed_before*/)
{
  // The box has (probably) landed on an object (a barrel for instance)
  if (my_speed_before.Norm() != 0.0)
    CloseParachute();

  if (obj->GetType() ==  GAME_CHARACTER) {
    ApplyBonus((Character *)obj);
  }
}

void ObjBox::SignalDrowning()
{
  CloseParachute();
}

void ObjBox::DropBox()
{
  if (parachute) {
    GetPhysic()->SetAirFrictionFactor(0.0);
    parachute = false;
    anim->SetCurrentFrame(anim->GetFrameCount() - 1);
  } else {
    m_ignore_movements = true;
  }
}

void ObjBox::Draw()
{
  anim->SetRotation_HotSpot(Point2i(0,0));
  double angle = GetAngle();
  Point2d offset = GetCenterOffset();
  Point2d relative_position;
  relative_position.x = cos(angle) * offset.x + sin(angle) *offset.y;
  relative_position.y = (cos(angle) * offset.y - sin(angle) *offset.x);
  anim->SetRotation_rad( - angle);
  anim->Draw(GetPosition() - relative_position);
}

void ObjBox::Refresh()
{
  // If we touch a character, we remove the medkit
  FOR_ALL_LIVING_CHARACTERS(team, character)
  {
    if(GetPhysic()->IsColliding((*character)->GetPhysic())) {
      ApplyBonus(*character);
      Ghost();
      return;
    }
  }
  // Refresh animation
  if (!anim->IsFinished() && !parachute) anim->Update();
}

//Boxes can explode...
void ObjBox::Explode()
{
  ParticleEngine::AddNow(GetPhysic()->GetPosition() , 10, particle_FIRE, true);
  ApplyExplosion(GetPhysic()->GetPosition(), GameMode::GetInstance()->bonus_box_explosion_cfg); //reuse the bonus_box explosion
};

void ObjBox::SignalGhostState(bool /*was_already_dead*/)
{
  if (GetEnergy() > 0) return;
  Explode();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Static methods
int ObjBox::start_life_points = 41;