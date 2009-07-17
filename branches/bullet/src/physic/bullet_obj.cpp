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
#include "physic/physical_engine.h"
#include "bullet_obj.h"
#include <iostream>

BulletObj::BulletObj() : PhysicalObj() {
    /// Create Dynamic Object
    btTransform startTransform;
    startTransform.setIdentity();

    btScalar mass(0.f);
    btVector3 localInertia(0, 0, 0);


    m_root_shape = new btCompoundShape(true);

    /*startTransform.setOrigin(btVector3(0, 0, 0));
    btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
    colShape->calculateLocalInertia(mass,localInertia);
*/
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

    btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, m_root_shape, localInertia);
    m_body = new btRigidBody(rbInfo);
    m_body->setActivationState(ISLAND_SLEEPING);

}



BulletObj::~BulletObj()
{

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
       current_transform.setOrigin(btVector3(position.x,position.y,0));
       m_body->setWorldTransform(current_transform);
  }
 
  const Point2d BulletObj::GetPosition() const
  {
    btTransform current_transform = m_body->getWorldTransform();
    return Point2d(current_transform.getOrigin().getX(),current_transform.getOrigin().getY());
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
    return m_body->getWorldTransform().getRotation().getZ();
  }

  //State
  void BulletObj::SetFixed(bool /*i_fixed*/){
      //TODO
  }

  bool BulletObj::IsFixed()
  {
    return m_body->isStaticObject();
  }

  void BulletObj::SetRotationFixed(bool /*rotating*/) {}
  void BulletObj::StopMovement(){}
  void BulletObj::SetFast(bool /*is_fast*/){}
  bool BulletObj::IsFast() { return false;}

 // Speed
  void BulletObj::SetSpeedXY(Point2d vector)
  {
    m_body->setLinearVelocity(btVector3(vector.x,vector.y,0));
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
      return Point2d(m_body->getLinearVelocity().getX(),m_body->getLinearVelocity().getY());
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
    if(name !=""){
      m_shape_list[name] = reinterpret_cast<BulletShape *>(shape);
    }

    shape->SetParent(this);
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(shape->GetPosition().x,shape->GetPosition().y,0));
    BulletShape * native_shape = dynamic_cast<BulletShape *>(shape);
    ASSERT(native_shape->GetNativeShape());
    m_root_shape->addChildShape(startTransform,native_shape->GetNativeShape());

  }
  void BulletObj::RemoveShape(PhysicalShape *shape)
  {
    std::map<std::string,BulletShape *>::iterator it;

    for(it = m_shape_list.begin() ; it != m_shape_list.end(); it++)
    {
      if(it->second == reinterpret_cast<BulletShape *>(shape))
      {
        m_shape_list.erase(it);

      }
    }

    BulletShape * native_shape = reinterpret_cast<BulletShape *>(shape);
    m_root_shape->removeChildShape(native_shape->GetNativeShape());
    delete shape;
  }
  void BulletObj::RemoveShape(std::string name)
  {
    std::map<std::string,BulletShape *>::iterator it;

    for(it = m_shape_list.begin() ; it != m_shape_list.end(); it++)
    {
      if(it->first == name)
      {
        BulletShape * native_shape = reinterpret_cast<BulletShape *>(it->second);
        m_root_shape->removeChildShape(native_shape->GetNativeShape());
        m_shape_list.erase(it);
        delete native_shape;
      }
    }
  }
  void BulletObj::ClearShapes()
  {
    std::map<std::string,BulletShape *>::iterator it;

    for(it = m_shape_list.begin() ; it != m_shape_list.end(); it++)
    {
      BulletShape * native_shape = reinterpret_cast<BulletShape *>(it->second);
      m_root_shape->removeChildShape(native_shape->GetNativeShape());
      delete native_shape;
    }
    m_shape_list.clear();
  }
  void BulletObj::LoadShapeFromXml(const xmlNode &/*xml_config*/) {}

   PhysicalShape *BulletObj::GetShape(const std::string &/*name*/){ 
      //TODO : modify this horror
       return PhysicalEngine::GetInstance()->CreateRectangleShape(10,10);
   }
   Rectangled BulletObj::GetBoundingBox()
   {
       Rectangled rect;
       rect.SetPosition(GetPosition());
       rect.SetSize(Point2d(10,10));
       return rect;
   }
  //  Mass
  double BulletObj::GetMass() const{ return 10;}

  // Force
  uint BulletObj::AddExternForceXY (const Point2d& /*vector*/){ return 0;}
  uint BulletObj::AddExternForce (double /*norm*/, double /*angle*/) { return 0;}
  void BulletObj::RemoveExternForce(unsigned /*force_index*/) {}
  void BulletObj::RemoveAllExternForce() {}
  void BulletObj::ImpulseXY(const Point2d& /*vector*/){}
  void BulletObj::Impulse(double /*norm*/, double /*angle*/) {}

  // Collision

  void BulletObj::SetCollisionMembership(CollisionCategory /*category*/, bool /*state*/){}
  void BulletObj::SetCollisionCategory(CollisionCategory /*category*/,bool /*state*/){}
  bool BulletObj::IsColliding() const {
    return false;
    //return m_body->hasContactResponse();
  }

  PhysicalObj* BulletObj::CollidedObjectXY(const Point2i & /*position*/) const { return NULL;}
  // Relative to current position
  PhysicalObj* BulletObj::CollidedObject(const Point2i & /*offset*/) const { return NULL;}

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
