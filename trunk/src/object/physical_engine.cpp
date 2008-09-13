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

const double PIXEL_PER_METER = 10;

PhysicalEngine::PhysicalEngine()
{
  worldAABB.lowerBound.Set(-1000.0f, -1000.0f);
  worldAABB.upperBound.Set(1000.0f, 1000.0f);
  b2Vec2 gravity(0.0f, 10.0f);
  bool doSleep = true;

  physic_world = new b2World(worldAABB, gravity, doSleep);



  frame_rate = 60;
  last_step_time = 0;
  iterations = 10;
}

PhysicalEngine::~PhysicalEngine()
{
  delete physic_world;
}

b2Body *PhysicalEngine::GetNewGroundBody()
{
  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(0.0f, 0.0f);

  return physic_world->CreateBody(&groundBodyDef);

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

  float32 timeStep = 1.0f / frame_rate;


 if ((Time::GetInstance()->Read()-last_step_time) < timeStep)
    {
      return;
    }
  MSG_DEBUG("physical.step", "Engine step");


physic_world->Step(timeStep, iterations);


  last_step_time = last_step_time-timeStep;
}

void PhysicalEngine::ClearContact()
{
  added_contact_list.clear();
  persist_contact_list.clear();
  removed_contact_list.clear();
  result_contact_list.clear();
}

void PhysicalEngine::AddContactPoint(b2ContactPoint contact,ContactType type)
{
  switch(type)
  {
  case ADD:
    added_contact_list.push_back(contact);
    break;
  case PERSIST:
    persist_contact_list.push_back(contact);
    break;
  case REMOVE:
    removed_contact_list.push_back(contact);
    break;
  }
}


void PhysicalEngine::AddContactResult(b2ContactResult contact)
{
  result_contact_list.push_back(contact);
}



ContactListener::ContactListener(PhysicalEngine *e)
{
  engine = e;
}

void ContactListener::Add(const b2ContactPoint* point)
{
  b2ContactPoint contact = *point;
  engine->AddContactPoint(contact,PhysicalEngine::ADD);
}

void ContactListener::Persist(const b2ContactPoint* point)
{
  b2ContactPoint contact = *point;
  engine->AddContactPoint(contact,PhysicalEngine::PERSIST);
}


void ContactListener::Remove(const b2ContactPoint* point)
{
  b2ContactPoint contact = *point;
  engine->AddContactPoint(contact,PhysicalEngine::REMOVE);
}

void ContactListener::Result(const b2ContactResult* point)
{
  b2ContactResult contact = *point;
  engine->AddContactResult(contact);
}



