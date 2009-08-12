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
 * Physical Engine using Bullet Physic library.
 *****************************************************************************/
#include "physic/bullet_engine.h"
#include "physic/bullet_obj.h"
#include "physic/bullet_shape.h"
#include "physic/force.h"
#include "physic/bullet_contact.h"
#include "game/time.h"
#include "physical_engine.h"
#include "bullet_engine.h"

#include <iostream>


extern ContactAddedCallback  gContactAddedCallback;
extern ContactProcessedCallback gContactProcessedCallback;
extern ContactDestroyedCallback  gContactDestroyedCallback;


BulletEngine::BulletEngine() : PhysicalEngine() {
    m_scale = 100.0;
    m_frame_rate = 60;
    ///collision configuration contains default setup for memory, collision setup
    btDefaultCollisionConfiguration *collision_configuration = new btDefaultCollisionConfiguration();
    //m_collisionConfiguration->setConvexConvexMultipointIterations();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collision_configuration);

    btDbvtBroadphase *broadphase = new btDbvtBroadphase();

    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

    m_world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collision_configuration);
    //m_world = new btContinuousDynamicsWorld(dispatcher, broadphase, solver, collision_configuration);

    m_world->setGravity(btVector3(0, 10, 0));

    gContactAddedCallback = BulletEngine::ContactAddedCallback;
    gContactProcessedCallback = BulletEngine::ContactProcessedCallback;
    gContactDestroyedCallback= BulletEngine::ContactDestroyedCallback;



}

BulletEngine::~BulletEngine()
{

}

PhysicalGround *BulletEngine::CreateGround()
{

    return new BulletGround();
}

PhysicalObj *BulletEngine::CreateObject(PhysicalEngine::ObjectType /*type*/)
{

    return new BulletObj();
}

PhysicalPolygon *BulletEngine::CreatePolygonShape()
{
    return new BulletPolygon();
}
PhysicalCircle *BulletEngine::CreateCircleShape()
{
    return new BulletCircle();
}
PhysicalRectangle *BulletEngine::CreateRectangleShape(double width, double height)
{
    return new BulletRectangle( width,  height);
}


void BulletEngine::AddObject(PhysicalObj *new_obj)
{
    BulletObj *obj = reinterpret_cast<BulletObj *>(new_obj);
    ASSERT(!obj->IsInWorld());
    obj->GetBody()->setActivationState(ISLAND_SLEEPING);
    m_world->addRigidBody(obj->GetBody(), obj->GetCollisionCategory(),obj->GetcollisionMask());
    //m_world->addRigidBody(obj->GetBody());
    //obj->GetBody()->setActivationState(ACTIVE_TAG);
    m_object_list.push_back(obj);
   // std::cout<<"Add "<<new_obj<<" x="<<new_obj->GetPosition().x<<" y="<<new_obj->GetPosition().y<<std::endl;
    obj->SetInWorld(true);

/* b2Body * body = physic_world->CreateBody(new_obj->GetBodyDef());
  objects_list[body] = new_obj;
  return body;*/
}

void BulletEngine::AddGround(PhysicalGround *new_obj)
{
    BulletGround *obj = reinterpret_cast<BulletGround *>(new_obj);
    obj->GetBody()->setActivationState(ISLAND_SLEEPING);
    m_world->addRigidBody(obj->GetBody(),0x0002,0xFFFF);
    obj->GetBody()->setRestitution(0.5);
}
void BulletEngine::RemoveObject(PhysicalObj *obj)
{
  BulletObj *bobj = reinterpret_cast<BulletObj *>(obj);
  ASSERT(bobj->IsInWorld());
  std::vector<BulletObj *>::iterator it;
  for(it = m_object_list.begin(); it != m_object_list.end();it++){
   if(*it == bobj){
     m_object_list.erase(it);
     break;
   }
  }
  bobj->SetInWorld(false);
  m_world->removeRigidBody(bobj->GetBody());
//  std::cout<<"Remove "<<obj<<std::endl;
}

void BulletEngine::RemoveGround(PhysicalGround *obj)
{
  BulletGround *bobj = reinterpret_cast<BulletGround *>(obj);
  m_world->removeRigidBody(bobj->GetBody());
}

void BulletEngine::Step()
{

  btScalar timeStep = 1.0f / m_frame_rate;

  if ((Time::GetInstance()->Read()-m_last_step_time) < (uint)lround(timeStep)) {
    return;
  }

  //ResetContacts();

  for (uint i = 0; i< m_force_list.size();i++) {
        m_force_list[i]->m_target->ComputeForce(m_force_list[i]);
      }

 MSG_DEBUG("physical.step", "Engine step");
  m_world->stepSimulation(timeStep);
  m_last_step_time = m_last_step_time +lround(timeStep);

  

  /*
  for (uint i = 0; i< m_air_friction_shape_list.size(); i++){
    m_air_friction_shape_list[i]->ComputeAirFriction();
  }
  for (uint i = 0; i< m_auto_align_object_list.size(); i++){
    m_auto_align_object_list[i]->ComputeAutoAlign();
  }

  ComputeWind();
  ComputeModifiedGravity();

  physic_world->Step(timeStep, iterations);

  ComputeContacts();

  */
}

double BulletEngine::GetScale() const
{
  return m_scale;
}


// Force
void BulletEngine::AddForce(Force *force)
{
  m_force_list.push_back(force);
}

void BulletEngine::RemoveForce(Force *force)
{
  std::vector<Force *>::iterator it;
  for (it = m_force_list.begin(); it != m_force_list.end(); it++)
    {
      if (*it == force)
        {
          m_force_list.erase(it);
          break;
        }
    }
}

void BulletEngine::ResetContacts()
{
  std::vector<BulletObj *>::iterator it;
  for(it = m_object_list.begin(); it != m_object_list.end();it++){
    (*it)->ResetContacts();
  }
}
//Contact Callback

bool BulletEngine::ContactAddedCallback(btManifoldPoint& cp,const btCollisionObject* colObj0, int /*partId0*/, int /*index0*/, const btCollisionObject* colObj1, int /*partId1*/, int /*index1*/)
{
 // std::cout<<"ContactAdded"<<std::endl;
  if(cp.m_userPersistentData == NULL){
    BulletContact * contact = new BulletContact();
    cp.m_userPersistentData = contact;

    if(colObj0->getCollisionShape()->getUserPointer()){

      //const btCompoundShape *cshape = reinterpret_cast<const btCompoundShape *>(colObj0->getCollisionShape());
      //const btCollisionShape *shape = cshape->getChildShape(partId0);
      const btCollisionShape *shape = colObj0->getCollisionShape();
      BulletShape *bshape = reinterpret_cast<BulletShape *>(shape->getUserPointer());
      contact->SetShapeA(bshape);
      double scale = reinterpret_cast<BulletEngine *>(PhysicalEngine::GetInstance())->GetScale();
      Point2d position(cp.getPositionWorldOnA().getX() * scale,cp.getPositionWorldOnA().getY() * scale);
      contact->SetPositionA(position);
    }else{

      contact->SetShapeA(NULL);
    }

    if(colObj1->getCollisionShape()->getUserPointer()){

      //const btCompoundShape *cshape = reinterpret_cast<const btCompoundShape *>(colObj1->getCollisionShape());
      //const btCollisionShape *shape = cshape->getChildShape(partId1);
      const btCollisionShape *shape = colObj1->getCollisionShape();
      BulletShape *bshape = reinterpret_cast<BulletShape *>(shape->getUserPointer());
      contact->SetShapeB(bshape);
      double scale = reinterpret_cast<BulletEngine *>(PhysicalEngine::GetInstance())->GetScale();
      Point2d position(cp.getPositionWorldOnB().getX() * scale,cp.getPositionWorldOnB().getY() * scale);
      contact->SetPositionB(position);
    }else{

         contact->SetShapeB(NULL);
     }

    if(contact->GetBulletShapeA()){

      contact->GetBulletShapeA()->AddContact(contact);
    }
    if(contact->GetBulletShapeB()){

      contact->GetBulletShapeB()->AddContact(contact);
    }

  }
  return false;
}

bool BulletEngine::ContactProcessedCallback(btManifoldPoint& /*cp*/,void* /*colObj0*/, void* /*colObj1*/)
{
  return false;
}


bool BulletEngine::ContactDestroyedCallback(void* userPersistentData)
{


  BulletContact *contact = reinterpret_cast<BulletContact  *>(userPersistentData);
  if(contact->GetBulletShapeA())
  {
    contact->GetBulletShapeA()->RemoveContact(contact);
  }

  if(contact->GetBulletShapeB())
  {
    contact->GetBulletShapeB()->RemoveContact(contact);
  }

  delete contact;
  //  shape->RemoveContact();
  return false;
}
