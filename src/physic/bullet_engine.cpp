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
#include "physic/game_obj.h"
#include "physic/bullet_contact.h"
#include "game/time.h"
#include "physical_engine.h"
#include "bullet_engine.h"


#include <iostream>


static const double GROUND_RESTITUTION = 0.5;
static const double GROUND_FRICTION = 0.5;

extern ContactAddedCallback  gContactAddedCallback;
extern ContactProcessedCallback gContactProcessedCallback;
extern ContactDestroyedCallback  gContactDestroyedCallback;


struct BulletEngineFilterCallback : public btOverlapFilterCallback
{
         // return true when pairs need collision
         virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0,btBroadphaseProxy* proxy1) const
         {
                  bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
                  collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);

                  btCollisionObject* colObj0 = (btCollisionObject*)proxy0->m_clientObject;
                  btCollisionObject* colObj1 = (btCollisionObject*)proxy1->m_clientObject;

                  BulletObj *bulObj0 = NULL;
                  BulletObj *bulObj1 = NULL;

                  if(colObj0->getUserPointer()){
                    bulObj0 = reinterpret_cast<BulletObj *>(colObj0->getUserPointer());
                    if(!bulObj0->IsEnabled()){
                      return false;
                    }
                  }

                  if(colObj1->getUserPointer()){
                    bulObj1 = reinterpret_cast<BulletObj *>(colObj1->getUserPointer());
                    if(!bulObj1->IsEnabled()){
                      return false;
                    }
                  }


                  if(!bulObj0 || !bulObj1){
                    return collides;
                  }



                  if(!bulObj0->IsEnabled() || !bulObj1->IsEnabled()){
                    return false;
                  }

                  if(bulObj0->IsOverlappingObject(bulObj1)){
                    return false;
                  }


                  return collides;
         }
};


BulletEngine::BulletEngine() : PhysicalEngine() {
    m_scale = 100.0;
    m_frame_rate = 60;
    m_is_in_step = false;
    m_gravity = 10;
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

    btOverlapFilterCallback * filterCallback = new BulletEngineFilterCallback();
    m_world->getPairCache()->setOverlapFilterCallback(filterCallback);


    m_world->setGravity(btVector3(0, m_gravity, 0));
    m_world->setSynchronizeAllMotionStates(false);
    m_world->setForceUpdateAllAabbs(false);


    gContactAddedCallback = BulletEngine::ContactAddedCallback;
    gContactProcessedCallback = BulletEngine::ContactProcessedCallback;
    gContactDestroyedCallback= BulletEngine::ContactDestroyedCallback;



}

BulletEngine::~BulletEngine()
{

}

double BulletEngine::GetGravity() const
{
  return m_gravity;
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
    m_object_list.push_back(obj);
    obj->SetInWorld(true);

    if (obj->IsSpecialObj() )
    {
      AddSpecialObject(obj);
    }

}

void BulletEngine::AddGround(PhysicalGround *new_obj)
{
    BulletGround *obj = reinterpret_cast<BulletGround *>(new_obj);
    obj->GetBody()->setActivationState(ISLAND_SLEEPING);
    obj->GetBody()->setRestitution(0.5);

    if(m_is_in_step){
       m_add_list.push_back(obj);
    }else{
      m_world->addRigidBody(obj->GetBody(),0x0002,0xFFFF);

    }
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
  RemoveSpecialObject(bobj);
}

void BulletEngine::RemoveGround(PhysicalGround *obj)
{
  BulletGround *bobj = reinterpret_cast<BulletGround *>(obj);

  std::vector<BulletGround *>::iterator it;
  for(it = m_add_list.begin(); it != m_add_list.end();it++){
    if(*it == bobj){
      m_add_list.erase(it);
      return;
    }
  }

  m_garbage_list.push_back(bobj);

}

void BulletEngine::Step()
{

  btScalar timeStep = 1.0f / m_frame_rate;

  if ((Time::GetInstance()->Read()-m_last_step_time) < (uint)lround(timeStep))
  {
    return;
  }

  for (uint i = 0; i< m_special_object_list.size(); i++){
    m_special_object_list[i]->ComputeAutoAlign();
    m_special_object_list[i]->ComputeAirFriction();
    m_special_object_list[i]->ComputeModifiedGravity();
    m_special_object_list[i]->ComputeWind(m_wind_vector);
  }

  for (uint i = 0; i< m_force_list.size();i++)
  {
        m_force_list[i]->m_target->ComputeForce(m_force_list[i]);
  }

 MSG_DEBUG("physical.step", "Engine step");
  m_is_in_step = true;
  m_world->stepSimulation(timeStep,60,btScalar(1.)/btScalar(180.));
  ApplyContacts();
  m_is_in_step = false;
  m_last_step_time = m_last_step_time +lround(timeStep);

  CleanGarbage();
  PerformAddList();

}

void BulletEngine::VirtualStep()
{
  if(m_is_in_step){
    return;
  }

 MSG_DEBUG("physical.step", "Engine  virtual step");
  m_is_in_step = true;
  m_world->performDiscreteCollisionDetection();
  ApplyContacts();
  m_is_in_step = false;

  CleanGarbage();
  PerformAddList();


}

void BulletEngine::PerformAddList(){
  std::vector<BulletGround *>::iterator it;
    for(it = m_add_list.begin(); it != m_add_list.end();it++){
      m_world->addRigidBody((*it)->GetBody(),0x0002,0xFFFF);
    }
    m_add_list.clear();
}

void BulletEngine::CleanGarbage(){
  std::vector<BulletGround *>::iterator it;
    for(it = m_garbage_list.begin(); it != m_garbage_list.end();it++){
      m_world->removeRigidBody((*it)->GetBody());
      delete (*it);
     }
    m_garbage_list.clear();
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

void BulletEngine::ApplyContacts()
{
  std::vector<BulletContact *>::iterator it;
  BulletContact *contact;

  for(it = m_contact_add_list.begin(); it != m_contact_add_list.end();it++)
  {
    contact = *it;
    MSG_DEBUG("physic.contact", "Contact created between \"%s.%s\" and \"%s.%s\" ",
         (contact->GetBulletShapeA() ? contact->GetBulletShapeA()->GetBulletParent()->GetContactListener()->GetName().c_str() : "ground"),
         (contact->GetBulletShapeA() ? contact->GetShapeA()->GetName().c_str() : "ground"),
         (contact->GetBulletShapeB() ? contact->GetBulletShapeB()->GetBulletParent()->GetContactListener()->GetName().c_str() : "ground"),
         (contact->GetBulletShapeB() ? contact->GetShapeB()->GetName().c_str() : "ground"));

    if(!contact->IsSignaled())
    {
      contact->Signal();
      MSG_DEBUG("physic.contact", "Contact signaled");
    }


  }

  m_contact_add_list.clear();

  for(it = m_contact_remove_list.begin(); it != m_contact_remove_list.end();it++)
  {
    contact = *it;

    if(contact->GetBulletShapeA())
    {
     contact->GetBulletShapeA()->RemoveContact(contact);
    }

    if(contact->GetBulletShapeB())
    {
     contact->GetBulletShapeB()->RemoveContact(contact);
    }

    delete contact;
  }

  m_contact_remove_list.clear();

}
//Contact Callback

bool BulletEngine::ContactAddedCallback(btManifoldPoint& cp,const btCollisionObject* colObj0, int /*partId0*/, int /*index0*/, const btCollisionObject* colObj1, int /*partId1*/, int /*index1*/)
{
  double friction0 = GROUND_FRICTION;
  double friction1 = GROUND_FRICTION;
  double restitution0 = GROUND_RESTITUTION;
  double restitution1 = GROUND_RESTITUTION;

  if(cp.m_userPersistentData == NULL)
  {
    BulletContact * contact = new BulletContact();
    cp.m_userPersistentData = contact;

    if(colObj0->getCollisionShape()->getUserPointer())
    {
      const btCollisionShape *shape = colObj0->getCollisionShape();
      BulletShape *bshape = reinterpret_cast<BulletShape *>(shape->getUserPointer());
      contact->SetShapeA(bshape);
      double scale = reinterpret_cast<BulletEngine *>(PhysicalEngine::GetInstance())->GetScale();
      Point2d position(cp.getPositionWorldOnA().getX() * scale,cp.getPositionWorldOnA().getY() * scale);
      contact->SetPositionA(position);
      friction0 = bshape->GetPublicShape()->GetFriction();
      restitution0 = bshape->GetPublicShape()->GetRestitution();
    }
    else
    {
      contact->SetShapeA(NULL);
      contact->SetPositionA(Point2d(0,0));
      contact->SetSpeedXYA(Point2d(0,0));
    }

    if(colObj1->getCollisionShape()->getUserPointer())
    {
      const btCollisionShape *shape = colObj1->getCollisionShape();
      BulletShape *bshape = reinterpret_cast<BulletShape *>(shape->getUserPointer());
      contact->SetShapeB(bshape);
      double scale = reinterpret_cast<BulletEngine *>(PhysicalEngine::GetInstance())->GetScale();
      Point2d position(cp.getPositionWorldOnB().getX() * scale,cp.getPositionWorldOnB().getY() * scale);
      contact->SetPositionB(position);

      friction1 = bshape->GetPublicShape()->GetFriction();
      restitution1 = bshape->GetPublicShape()->GetRestitution();
    }
    else
    {
         contact->SetShapeB(NULL);
         contact->SetPositionB(Point2d(0,0));
         contact->SetSpeedXYB(Point2d(0,0));
    }
    bool exist = false;

    if(contact->GetBulletShapeA())
    {
      exist = exist || !contact->GetBulletShapeA()->AddContact(contact);
    }
    if(contact->GetBulletShapeB())
    {
      exist = exist || !contact->GetBulletShapeB()->AddContact(contact);
    }

    if(exist)
    {
      cp.m_userPersistentData = NULL;
      delete contact;
    }
    else
    {
      reinterpret_cast<BulletEngine *>(PhysicalEngine::GetInstance())->m_contact_add_list.push_back(contact);
    }


  }


  //Compute friction and restitution
  cp.m_combinedFriction = friction0 * friction1;
  cp.m_combinedRestitution = restitution0 * restitution1;




  return true;
}

bool BulletEngine::ContactProcessedCallback(btManifoldPoint& /*cp*/,void* /*colObj0*/, void* /*colObj1*/)
{
  return false;
}


bool BulletEngine::ContactDestroyedCallback(void* userPersistentData)
{

  BulletContact *contact = reinterpret_cast<BulletContact  *>(userPersistentData);
  reinterpret_cast<BulletEngine *>(PhysicalEngine::GetInstance())->m_contact_remove_list.push_back(contact);

  return false;
}

void BulletEngine::AddSpecialObject(BulletObj* i_object)
{
  RemoveSpecialObject(i_object);
  m_special_object_list.push_back(i_object);
}

void BulletEngine::RemoveSpecialObject(BulletObj* i_object)
{
  std::vector<BulletObj *>::iterator it;
  for (it = m_special_object_list.begin(); it != m_special_object_list.end(); it++)
  {
    if (*it == i_object)
      {
        m_special_object_list.erase(it);
        break;
      }
  }
}


