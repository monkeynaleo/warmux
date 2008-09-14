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
#include "object/force.h"
#include "object/physical_engine.h"
#include "object/physical_obj.h"
#include <iostream>

const double PIXEL_PER_METER = 20;

PhysicalEngine::PhysicalEngine()
{
  worldAABB.lowerBound.Set(-10000.0f, -10000.0f);
  worldAABB.upperBound.Set(10000.0f, 10000.0f);
  b2Vec2 gravity(0.0f, 10.0f);
  bool doSleep = true;

  physic_world = new b2World(worldAABB, gravity, doSleep);

  m_contact_listener = new ContactListener(this);
  physic_world->SetContactListener(m_contact_listener);
  frame_rate = 60;
  last_step_time = 0;
  iterations = 10;
}

PhysicalEngine::~PhysicalEngine()
{
  delete m_contact_listener;
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
  physic_world->DestroyBody(obj->GetBody());

}


void PhysicalEngine::Step()
{

  float32 timeStep = 1.0f / frame_rate;


 if ((Time::GetInstance()->Read()-last_step_time) < timeStep)
    {
      return;
    }
  MSG_DEBUG("physical.step", "Engine step");

  for(unsigned i = 0; i< m_force_list.size();i++){
    m_force_list[i]->ComputeForce();
  }

  ClearContact();
  physic_world->Step(timeStep, iterations);

  for(unsigned i = 0;i<added_contact_list.size();i++){


    b2ContactPoint contact = added_contact_list[i];

    if((objects_list.count(contact.shape1->GetBody()) == 1) && (objects_list[contact.shape1->GetBody()]!=NULL)){
      Physics  *collider =  objects_list[contact.shape1->GetBody()];

      collider->SignalRebound();

      Point2d vel = Point2d(contact.velocity.x*PIXEL_PER_METER,contact.velocity.y*PIXEL_PER_METER);

      collider->SignalCollision(vel);

      if(objects_list.count(contact.shape2->GetBody()) >0){

        collider->SignalObjectCollision(objects_list[contact.shape2->GetBody()],vel );
      }else{

        collider->SignalGroundCollision(vel);
      }


    }

  }



  last_step_time = last_step_time-timeStep;
}

void PhysicalEngine::ClearContact()
{
  added_contact_list.clear();
  persist_contact_list.clear();
  removed_contact_list.clear();
  result_contact_list.clear();
}


void PhysicalEngine::AddForce(Force * force)
{
    m_force_list.push_back(force);
}

void PhysicalEngine::RemoveForce(Force *force)
{

  std::vector<Force *>::iterator it;
  for(it = m_force_list.begin(); it!=m_force_list.end();it++){
    if(*it == force){
      m_force_list.erase(it);
      break;
    }

  }

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



ContactListener::ContactListener(PhysicalEngine *e):b2ContactListener()
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



