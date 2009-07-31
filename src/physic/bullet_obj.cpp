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
 * Abstract class used for physical object (object with a size, mass,
 * etc.). This object can have differents state : ready, is moving, or ghost
 * (is outside of the world).
 *
 * You can : make the object move (with collision test), change state, etc.
 * If the object go outside of the world, it become a ghost.
 *****************************************************************************/
#include "physic/bullet_obj.h"
#include "physic/bullet_shape.h"
#include "physic/bullet_engine.h"
#include "physic/force.h"
#include "bullet_obj.h"
#include <iostream>
#include <stdio.h>


#define BULLET_SPEED_FACTOR 0.04
#define BULLET_IMPULSE_FACTOR 3

BulletObj::BulletObj() : PhysicalObj() {
    /// Create Dynamic Object
    btTransform startTransform;
    startTransform.setIdentity();

    btScalar mass(100.0f);
    btVector3 localInertia(0, 0, 0);


    m_root_shape = new btCompoundShape(true);

    startTransform.setOrigin(btVector3(0, 0, 0));
    btCollisionShape* colShape = new btBoxShape(btVector3(10/GetScale(),10/GetScale(),10/GetScale()));
    colShape->calculateLocalInertia(mass,localInertia);

    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

    btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, m_root_shape, localInertia);
    m_body = new btRigidBody(rbInfo);
    m_body->setActivationState(ISLAND_SLEEPING);
    m_body->setLinearFactor(btVector3(1,1,0));
    m_body->setAngularFactor(btVector3(0,0,0));
    m_body->setDamping(0.1,0.5);
    m_body->setRestitution(0.1);

    m_body->setCollisionFlags(m_body->getCollisionFlags() |
        btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

    m_in_world = false;
   // m_root_shape->addChildShape(startTransform,colShape);

}



BulletObj::~BulletObj()
{
  PhysicalEngine::GetInstance()->RemoveObject(this);
  ClearShapes();
}

 void BulletObj::LoadFromXml(const xmlNode &/*xml_obj*/){}

  void BulletObj::Activate() {}
  void BulletObj::Generate() {}
  void BulletObj::Desactivate() {}

  //-------- Position, speed and size -------

  // Set/Get position
  void BulletObj::SetPosition(const Point2d &position)
  {
       btTransform current_transform = m_body->getWorldTransform();
       current_transform.setOrigin(btVector3(position.x/GetScale(),position.y/GetScale(),0));
       m_body->setWorldTransform(current_transform);
  }

  double BulletObj::GetScale() const
  {
   return (reinterpret_cast<BulletEngine *>(PhysicalEngine::GetInstance()))->GetScale();
  }
 
  Point2d BulletObj::GetPosition() const
  {
    /*btTransform current_transform;
    m_body->getMotionState()->getWorldTransform(current_transform);
    ASSERT(current_transform.getOrigin().getZ() == 0);
    return Point2d(current_transform.getOrigin().getX()*GetScale(),current_transform.getOrigin().getY()*GetScale());*/
    btTransform current_transform = m_body->getWorldTransform();
    return Point2d(current_transform.getOrigin().getX()*GetScale(),current_transform.getOrigin().getY()*GetScale());

  }

  void BulletObj::SetAngle(double angle_rad)
  {
    btTransform current_transform = m_body->getWorldTransform();
    btQuaternion rot = current_transform.getRotation();
    rot.setEuler(0,0,angle_rad);
    current_transform.setRotation(rot);
    m_body->setWorldTransform(current_transform);
  }

  double BulletObj::GetAngle() const{
    btTransform current_transform;
    m_body->getMotionState()->getWorldTransform(current_transform);
    return current_transform.getRotation().getZ();
  }

  //State
  void BulletObj::SetFixed(bool i_fixed){
    if(i_fixed){
      m_body->setLinearFactor(btVector3(0,0,0));
    }else{
      m_body->setLinearFactor(btVector3(1,1,0));
    }
  }

  bool BulletObj::IsFixed()
  {
    return m_body->isStaticObject();
  }

  void BulletObj::SetRotationFixed(bool rotating)
  {
    if(rotating){
      m_body->setAngularFactor(btVector3(0,0,0));
    }else{
      m_body->setAngularFactor(btVector3(0,0,0));
    }
  }
  void BulletObj::StopMovement(){
    SetSpeedXY(Point2d(0,0));
  }
  void BulletObj::SetFast(bool /*is_fast*/){

  }
  bool BulletObj::IsFast() { return false;}

 // Speed
  void BulletObj::SetSpeedXY(Point2d vector)
  {
    m_body->setLinearVelocity(btVector3(vector.x/(GetScale()*BULLET_SPEED_FACTOR),vector.y/(GetScale()*BULLET_SPEED_FACTOR),0));
    m_body->setActivationState(ACTIVE_TAG);
  }

  void BulletObj::SetSpeed(double norm, double angle_rad)
  {
    SetSpeedXY(Point2d::FromPolarCoordinates(norm, angle_rad));

  }

  void BulletObj::SetAngularSpeed(double /*speed*/){}


  void BulletObj::GetSpeed(double &norm, double &angle_rad) const
  {
    Point2d speed ;

    speed = GetSpeed();
    norm = speed.Norm();
    angle_rad = speed.ComputeAngle();

  }

Point2d BulletObj::GetSpeed() const
  {
      return Point2d(m_body->getLinearVelocity().getX()*GetScale()*BULLET_SPEED_FACTOR,m_body->getLinearVelocity().getY()*GetScale()*BULLET_SPEED_FACTOR);
  }

  double BulletObj::GetAngularSpeed() const
  {
	  return m_body->getAngularVelocity().getZ();
  }

  bool BulletObj::IsMoving() const
  {
	  return (GetSpeed().x == 0 && GetSpeed().y == 0);
  }

  bool BulletObj::IsSleeping() const {
	  return !m_body->isActive();
  }
  // Shape

  void BulletObj::AddShape(PhysicalShape *shape,std::string name){

    if(name =="")
      {
      if(shape->GetName() == "")
      {
        char buffer [50];
        snprintf(buffer, 50, "%u", (unsigned int) m_shape_list.size());
        name = "unamed_shape_"+std::string(buffer);
      }
      else
      {
        name = shape->GetName();
      }
    }

    m_shape_list[name] = shape;

    shape->SetParent(this);
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(shape->GetPosition().x/GetScale(),shape->GetPosition().y/GetScale(),0));
    BulletShape * native_shape = dynamic_cast<BulletShape *>(shape);
    ASSERT(native_shape->GetNativeShape());
    m_root_shape->addChildShape(startTransform,native_shape->GetNativeShape());

  }
  void BulletObj::RemoveShape(PhysicalShape *shape)
  {
    std::map<std::string,PhysicalShape *>::iterator it;

    for(it = m_shape_list.begin() ; it != m_shape_list.end(); it++)
    {
      if(it->second == shape)
      {
        m_shape_list.erase(it);

      }
    }

    BulletShape * native_shape = dynamic_cast<BulletShape *>(shape);
    m_root_shape->removeChildShape(native_shape->GetNativeShape());
    //delete shape;
  }
  void BulletObj::RemoveShape(std::string name)
  {
    std::map<std::string,PhysicalShape *>::iterator it;

    for(it = m_shape_list.begin() ; it != m_shape_list.end(); it++)
    {
      if(it->first == name)
      {
        BulletShape * native_shape = dynamic_cast<BulletShape *>(it->second);
        m_root_shape->removeChildShape(native_shape->GetNativeShape());
        m_shape_list.erase(it);
        //delete native_shape;
      }
    }
  }
  void BulletObj::ClearShapes()
  {
    std::map<std::string,PhysicalShape *>::iterator it;

    for(it = m_shape_list.begin() ; it != m_shape_list.end(); it++)
    {
      BulletShape * native_shape = dynamic_cast<BulletShape *>(it->second);
      m_root_shape->removeChildShape(native_shape->GetNativeShape());
      //delete native_shape;
    }
    m_shape_list.clear();
  }
  void BulletObj::LoadShapeFromXml(const xmlNode &/*xml_config*/)
  {

  }

   PhysicalShape *BulletObj::GetShape(const std::string &name){
     if(m_shape_list.count(name))
     {
       return m_shape_list[name];
     }
     else
       {
       return NULL;
     }

   }
   Rectangled BulletObj::GetBoundingBox()
   {
       Rectangled rect;
       rect.SetPosition(GetPosition());
       rect.SetSize(Point2d(10,10));
       return rect;
   }
  //  Mass
  double BulletObj::GetMass() const
  {
	  return 1/m_body->getInvMass();
  }

  void BulletObj::SetMass(double mass)
  {
    m_body->setMassProps(mass,btVector3(0,0,0));
  }

  // Force
  Force *BulletObj::AddExternForceXY (const Point2d& vector)
  {
	  Force * force = new Force(this,vector);
	  PhysicalEngine::GetInstance()->AddForce(force);
	  m_force_list.push_back(force);
	  return force;
  }

  Force *BulletObj::AddExternForce (double norm, double angle_rad) {
	  Point2d force = Point2d::FromPolarCoordinates(norm, angle_rad);
	  return AddExternForce(force.x,force.y);
  }

  void BulletObj::RemoveExternForce(Force *force)
  {
	  std::vector<Force *>::iterator it;
	  for(it = m_force_list.begin(); it != m_force_list.end(); it++){
		  if(*it == force){
			  m_force_list.erase(it);
			  break;
		  }
	  }
	  PhysicalEngine::GetInstance()->RemoveForce(force);
	  delete force;
  }
  void BulletObj::RemoveAllExternForce()
  {
	  std::vector<Force *>::iterator it;
	  for(it = m_force_list.begin(); it != m_force_list.end(); it++){
		PhysicalEngine::GetInstance()->RemoveForce(*it);
		delete *it;
	  }
	  m_force_list.clear();
  }
  void BulletObj::ImpulseXY(const Point2d& vector)
  {
    std::cout<<"Impulse"<<std::endl;
    m_body->internalApplyImpulse(btVector3(vector.x/(GetScale()*BULLET_IMPULSE_FACTOR), vector.y/(GetScale()*BULLET_IMPULSE_FACTOR),0),btVector3(0,0,0),1);
    m_body->setActivationState(ACTIVE_TAG);
  }
  void BulletObj::Impulse(double norm, double angle)
  {
    ImpulseXY(Point2d::FromPolarCoordinates(norm, angle));

  }

  void BulletObj::ComputeForce(Force * force)
  {
    m_body->applyCentralForce(btVector3(force->m_force.x/(GetScale()),force->m_force.y/(GetScale()),0));
    m_body->setActivationState(ACTIVE_TAG);
  }


  // Collision
  int BulletObj::GetCollisionCategory(){
    return m_collision_category;
  }
  int BulletObj::GetcollisionMask(){
    return m_collision_mask;
  }
  void BulletObj::SetCollisionMembership(CollisionCategory category, bool state){
    int bit = 0;
    switch(category){
      case COLLISION_CHARACTER:
        bit = 0x01;
        break;
      case COLLISION_GROUND:
        bit = 0x02;
        break;
      case COLLISION_ITEM:
        bit = 0x04;
        break;
      case COLLISION_PROJECTILE:
        bit = 0x08;
        break;
    }

    if(state){
      m_collision_category |= bit;
    }else{
      m_collision_category &= ~bit;
    }

    Reload();


  }
  void BulletObj::SetCollisionCategory(CollisionCategory category,bool state){
    int bit = 0;
    switch(category){
      case COLLISION_CHARACTER:
        bit = 0x01;
        break;
      case COLLISION_GROUND:
        bit = 0x02;
        break;
      case COLLISION_ITEM:
        bit = 0x04;
        break;
      case COLLISION_PROJECTILE:
        bit = 0x08;
        break;
    }

    if(state){
      m_collision_mask |= bit;
    }else{
      m_collision_mask &= ~bit;
    }
    Reload();
  }

  bool BulletObj::IsColliding() const {
    return false;
    //return m_body->hasContactResponse();
  }

  PhysicalObj* BulletObj::CollidedObjectXY(const Point2i & /*position*/) const { return NULL;}
  // Relative to current position
  PhysicalObj* BulletObj::CollidedObject(const Point2i & /*offset*/) const { return NULL;}

  void BulletObj::ResetContacts()
  {
    std::map<std::string,PhysicalShape *>::iterator it;

       for(it = m_shape_list.begin() ; it != m_shape_list.end(); it++)
       {
         BulletShape * native_shape = dynamic_cast<BulletShape *>(it->second);
         ASSERT(native_shape);
         native_shape->ResetContacts();
       }

  }


  void BulletObj::AddReboundListener(PhysicalListener */*listener*/) {}
  void BulletObj::AddCollisionListener(PhysicalListener */*listener*/) {}

  bool BulletObj::Contain(const Point2d &/*pos_to_check*/){ return false;}

  //Overlapping
  void BulletObj::AddOverlappingObject(PhysicalObj* /*obj*/, int /*timeout*/) {}
  void BulletObj::ClearOverlappingObject(PhysicalObj* /*obj*/) {}
  void BulletObj::ClearAllOverlappingObject() {}
  bool BulletObj::IsOverlappingObject(PhysicalObj */*obj*/) { return false;}
  bool BulletObj::Overlapse(const PhysicalObj* /*obj*/) const {return false;}
  const std::vector<PhysicalObj*> *BulletObj::GetOverlappingObject() const{ return NULL;}

  //Properties

  void BulletObj::SetFrictionFactor( double value)
  {
    m_body->setFriction(value);
  }

  void BulletObj::ResetFrictionFactor() {}

  double BulletObj::GetFritionFactor()
  {
    return m_body->getFriction();
  }

  void BulletObj::SetReboundFactor( double value){
    m_body->setRestitution(value);
  }

  void BulletObj::ResetReboundFactor(){}

  double BulletObj::GetReboundFactor(){
    return m_body->getRestitution();
  }

  void BulletObj::SetAirFrictionFactor( double value)
  {
    m_body->setDamping(value,0.01);
  }

  void BulletObj::ResetAirFrictionFactor(){}

  double BulletObj::GetAirFrictionFactor()
  {
    return m_body->getLinearDamping();
  }


  void BulletObj::SetWindFactor( double /*value*/){}
  void BulletObj::ResetWindFactor(){}
  double BulletObj::GetWindFactor(){ return 0;}

  void BulletObj::SetAutoAlignFactor( double /*value*/){}
  void BulletObj::ResetAutoAlignFactor(){}
  double BulletObj::GetAutoAlignFactor(){ return 0;}

  void BulletObj::SetGravityFactor( double /*value*/){}
  void BulletObj::ResetGravityFactor(){}
  double BulletObj::GetGravityFactor(){ return 0;}


  void BulletObj::SignalRebound() {}
  void BulletObj::SignalCollision(const Point2d&){}
  btRigidBody* BulletObj::GetBody()
  {
      return m_body;
  }


  void BulletObj::Reload(){
    if(IsInWorld()){
      PhysicalEngine::GetInstance()->RemoveObject(this);
      PhysicalEngine::GetInstance()->AddObject(this);
    }
  }

  void BulletObj::SetInWorld(bool in_world){
    m_in_world = in_world;
  }

  bool BulletObj::IsInWorld()
  {
    return m_in_world;
  }

  #ifdef DEBUG
  void BulletObj::DrawShape(const Color& color) const
  {
     std::map<std::string,PhysicalShape *>::const_iterator it;

     for(it = m_shape_list.begin() ; it != m_shape_list.end(); it++)
     {
      PhysicalShape * native_shape = it->second;
     native_shape->DrawBorder(color);
    }
  }
 #endif



    BulletGround::BulletGround(){

      // ground




  }

    void BulletGround::SetPosition(const Point2d &position)
    {
        m_position = position;
    }

    Point2d BulletGround::GetPosition() const{
      return m_position;
    }

    void BulletGround::AddShape(PhysicalShape *shape){

      m_shape_list.push_back(shape);
      shape->SetParent(this);

      BulletShape * native_shape = dynamic_cast<BulletShape *>(shape);
      ASSERT(native_shape->GetNativeShape());


      btTransform startTransform;
      startTransform.setIdentity();

      btScalar mass(0.f);
      btVector3 localInertia(0, 0, 0);

      startTransform.setOrigin(btVector3(m_position.x/GetScale(),m_position.y/GetScale(),0));

       btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
      btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, native_shape->GetNativeShape(), localInertia);
      m_body = new btRigidBody(rbInfo);

    }

    double BulletGround::GetScale() const
    {
     return (reinterpret_cast<BulletEngine *>(PhysicalEngine::GetInstance()))->GetScale();
    }

    #ifdef DEBUG
      void BulletGround::DrawShape(const Color& color) const{
        std::vector<PhysicalShape *>::const_iterator it;

        for(it = m_shape_list.begin() ; it != m_shape_list.end(); it++)
        {
         PhysicalShape * native_shape = *it;
        native_shape->DrawBorder(color);
       }
     }
    #endif

      btRigidBody* BulletGround::GetBody()
      {
          return m_body;
      }

