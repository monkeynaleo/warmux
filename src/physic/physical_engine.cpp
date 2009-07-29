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
 * Graphical interface showing various information about the game.
 *****************************************************************************/

#include "game/time.h"
#include "physic/force.h"
#include "physic/physical_engine.h"
#include "physic/physical_obj.h"
#include "physic/physical_shape.h"
#include "tool/math_tools.h"
#include "tool/debug_physics.h"
#include <iostream>

PhysicalEngine *PhysicalEngine::g_instance = NULL;

PhysicalEngine *PhysicalEngine::GetInstance()
{
    ASSERT(g_instance);
    return g_instance;
}

void PhysicalEngine::SetInstance(PhysicalEngine * instance)
{
    g_instance = instance;
}





PhysicalEngine::PhysicalEngine() :
  m_frame_rate(60),
  m_last_step_time(0),
  m_wind_vector(0,0)
{
/*  worldAABB.lowerBound.Set(-10000.0f, -10000.0f);
  worldAABB.upperBound.Set(10000.0f, 10000.0f);
  b2Vec2 gravity(0.0f, 30.0f);
  bool doSleep = true;

  physic_world = new b2World(worldAABB, gravity, doSleep);

  m_contact_listener = new ContactListener(this);
  physic_world->SetContactListener(m_contact_listener);

  m_debug_draw = new DebugDraw();
  physic_world->SetDebugDraw(m_debug_draw);

  #ifdef DEBUG
  if (IsLOGGING("physic.slow")) {
      frame_rate = 600;
  }
  #endif
  // uint32 flags = 0;
  // flags += 0 * b2DebugDraw::e_shapeBit;
  // flags += 0 * b2DebugDraw::e_jointBit;
  // flags += 0 * b2DebugDraw::e_coreShapeBit;
  // flags += 0 * b2DebugDraw::e_aabbBit;
  // flags += 0 * b2DebugDraw::e_obbBit;
  // flags += 0 * b2DebugDraw::e_pairBit;
  // flags += 0 * b2DebugDraw::e_centerOfMassBit;
  // m_debug_draw->SetFlags(flags);
*/
}

PhysicalEngine::~PhysicalEngine()
{
//  delete m_contact_listener;
//  delete m_debug_draw;
//  delete physic_world;
}


void PhysicalEngine::SetWindVector(const Point2d & i_wind_vector)
{
  m_wind_vector = i_wind_vector;
}




/*
b2Joint* PhysicalEngine::CreateJoint(b2JointDef* i_joint_def)
{
 return physic_world->CreateJoint(i_joint_def);
}*/

void PhysicalEngine::VirtualStep()
{
    /*
  if (!m_static_step_in_progress) {
    m_static_step_in_progress = true;
    physic_world->Step(0,0);
    ComputeContacts();
    m_static_step_in_progress = false;
  }*/
}

void PhysicalEngine::AddAirFrictionShape(PhysicalShape * /*shape*/)
{
//  m_air_friction_shape_list.push_back(shape);
}

void PhysicalEngine::RemoveAirFrictionShape(PhysicalShape */*shape*/)
{
/* std::vector<PhysicalShape *>::iterator it;
  for (it = m_air_friction_shape_list.begin(); it != m_air_friction_shape_list.end(); it++){
    if (*it == shape) {
      m_air_friction_shape_list.erase(it);
      break;
    }
  }*/
}



void PhysicalEngine::AddWindObject(PhysicalObj */*i_object*/)
{
	//m_wind_object_list.push_back(i_object);
}

void PhysicalEngine::RemoveWindObject(PhysicalObj */*i_object*/)
{
 /* std::vector<GameObj *>::iterator it;
  for (it = m_wind_object_list.begin(); it != m_wind_object_list.end(); it++){
    if (*it == i_object) {
      m_wind_object_list.erase(it);
      break;
    }
  }*/
}

void PhysicalEngine::AddModifiedGravityObject(PhysicalObj */*i_object*/)
{
	//m_modified_gravity_object_list.push_back(i_object);
}

void PhysicalEngine::RemoveModifiedGravityObject(PhysicalObj */*i_object*/)
{
  /*std::vector<GameObj *>::iterator it;
  for (it = m_modified_gravity_object_list.begin(); it != m_modified_gravity_object_list.end(); it++){
    if (*it == i_object) {
      m_modified_gravity_object_list.erase(it);
      break;
    }
  }*/
}

void PhysicalEngine::AddAutoAlignObject(PhysicalObj * /*object*/)
{
    //m_auto_align_object_list.push_back(object);
}

void PhysicalEngine::RemoveAutoAlignObject(PhysicalObj */*object*/)
{
 /* std::vector<GameObj *>::iterator it;
  for (it = m_auto_align_object_list.begin(); it != m_auto_align_object_list.end(); it++){
    if (*it == object) {
      m_auto_align_object_list.erase(it);
      break;
    }
  }*/
}





