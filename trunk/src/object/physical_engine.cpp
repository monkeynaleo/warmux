/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
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
 * Graphical interface showing various information about the game.
 *****************************************************************************/

#include "game/time.h"
#include "object/physical_engine.h"
#include "object/physical_obj.h"
#include <iostream>

PhysicalEngine::PhysicalEngine()
{
  worldAABB.lowerBound.Set(-1000.0f, -1000.0f);
  worldAABB.upperBound.Set(1000.0f, 1000.0f);
  b2Vec2 gravity(0.0f, 10.0f);
  bool doSleep = true;

  physic_world = new b2World(worldAABB, gravity, doSleep);

  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(0.0f, 40.0f);
  b2Body* ground;

  ground= physic_world->CreateBody(&groundBodyDef);
  b2PolygonDef groundShapeDef1;
  groundShapeDef1.SetAsBox(500.0f, 10.0f);
  groundShapeDef1.friction = 0.8f;
  groundShapeDef1.restitution = 0.1f;
  ground->CreateShape(&groundShapeDef1);

  frame_rate = 60;
  last_step_time = 0;
  iterations = 10;
}

PhysicalEngine::~PhysicalEngine()
{
  delete physic_world;
}

b2Body *PhysicalEngine::AddObject(Physics *new_obj)
{
  b2Body * body = physic_world->CreateBody(new_obj->GetBodyDef());
  objects_list[body] = new_obj;
  return body;
}

void PhysicalEngine::RemoveObject(Physics *obj)
{
  objects_list.erase(obj->GetBody());
}


void PhysicalEngine::Step()
{
  MSG_DEBUG("fred", "step ?");
  float32 timeStep = 1.0f / frame_rate;

  if ((Time::GetInstance()->Read()-last_step_time) < timeStep)
    {
      return;
    }
  MSG_DEBUG("fred", "Engine step");

  physic_world->Step(timeStep, iterations);
  last_step_time = last_step_time-timeStep;
}



