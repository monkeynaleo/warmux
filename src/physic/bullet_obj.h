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

#ifndef BULLET_OBJECT_H
#define BULLET_OBJECT_H
#include "btBulletDynamicsCommon.h"
#include "physic/physical_obj.h"
#include <map>
#include <vector>

class BulletShape;
class GameObj;
class BulletContact;

class BulletObj : public PhysicalObj
{
  
public: 
  BulletObj();
 
  virtual ~BulletObj ();

  void LoadFromXml(const xmlNode &xml_obj);

  void Activate() ;
  void Generate() ;
  void Desactivate() ;

  //-------- Position, speed and size -------

  // Set/Get position
  void SetPosition(const Point2d &position) ;
  Point2d GetPosition() const  ;
  void SetAngle(double angle_rad) ;
  double GetAngle() const ;
  
  //State
  void SetEnable(bool enable);
  bool GetEnable();
  void SetFixed(bool i_fixed) ;
  bool IsFixed() ;
  void SetRotationFixed(bool rotating) ;
  bool IsRotationFixed() const { return m_rotating; }
  void StopMovement() ;
  void SetFast(bool is_fast) ;
  bool IsFast() ;
  
 // Speed
  void SetSpeedXY(Point2d vector) ;
  void SetSpeed(double norm, double angle_rad) ;
  void SetAngularSpeed(double speed) ;
  void GetSpeed(double &norm, double &angle_rad) const ;
  Point2d GetSpeed() const ;
  double GetAngularSpeed() const;
  bool IsMoving() const ;
  bool IsSleeping() const ;
  // Shape

  void AddShape(PhysicalShape *shape,std::string name) ;
  void RemoveShape(PhysicalShape *shape) ;
  void RemoveShape(std::string name) ;
  void ClearShapes() ;
  void LoadShapeFromXml(const xmlNode &xml_config) ;

   PhysicalShape *GetShape(const std::string &name) ;
   Rectangled GetBoundingBox() ;
  //  Mass
  double GetMass() const ;
  void SetMass(double mass);
 
  // Force
  Force *AddExternForceXY (const Point2d& vector) ;
  Force *AddExternForce (double nonrm, double angle) ;
  void RemoveExternForce(Force *force) ;
  void RemoveAllExternForce() ;
  void ImpulseXY(const Point2d& vector) ;
  void Impulse(double norm, double angle) ;
  void ComputeForce(Force * force);
  // Collision

  int GetCollisionCategory();
  int GetcollisionMask();

  void ResetContacts();
  void SetCollisionMembership(CollisionCategory category, bool state);
  void SetCollisionCategory(CollisionCategory category,bool state);
  bool IsColliding() const ;
  bool IsColliding(const PhysicalObj* obj) const;
  
  PhysicalObj* CollidedObjectXY(const Point2i & position) const ;
  // Relative to current position
  PhysicalObj* CollidedObject(const Point2i & offset = Point2i(0,0)) const ;

  void SetContactListener(GameObj *listener);
  GameObj *GetContactListener();

  bool Contain(const Point2d &pos_to_check) ;
 
  //Overlapping
  void AddOverlappingObject(PhysicalObj* obj, int timeout = 0) ;
  void ClearOverlappingObject(PhysicalObj* obj) ;
  void ClearAllOverlappingObject() ;
  bool IsOverlappingObject(PhysicalObj *obj) ;
  const std::vector<PhysicalObj*> *GetOverlappingObject() const ;
  
  //Properties

  // Reset the physics constants (mass, air_resistance...) to the default values in the cfg
  void SetFrictionFactor( double value) ;
  void ResetFrictionFactor() ;
  double GetFritionFactor() ;

  void SetReboundFactor( double value) ;
  void ResetReboundFactor() ;
  double GetReboundFactor() ;

  void SetAirFrictionFactor( double value) ;
  void ResetAirFrictionFactor() ;
  double GetAirFrictionFactor() ;

  void SetWindFactor( double value) ;
  void ResetWindFactor() ;
  double GetWindFactor() ;

  void SetAutoAlignFactor( double value) ;
  void ResetAutoAlignFactor() ;
  double GetAutoAlignFactor() ;

  void SetGravityFactor( double value) ;
  void ResetGravityFactor() ;
  double GetGravityFactor() ;

  //Bullet Method
  btRigidBody* GetBody();
 #ifdef DEBUG
  void DrawShape(const Color& color) const;
 #endif

  double GetScale() const;
  void SetInWorld(bool in_world);
  bool IsInWorld();
  void SignalCollision(BulletContact * contact);

protected:
  void Reload();
  
  bool m_rotating;
  btRigidBody* m_body;
  btCompoundShape *m_root_shape;
  std::map<std::string, PhysicalShape *> m_shape_list;
  int m_collision_category;
  int m_collision_mask;
  std::vector<Force *> m_force_list;
  GameObj *m_contact_listener;
  std::vector<PhysicalObj *> m_overlapping_obj_list;
  std::vector<int> m_overlapping_time_list;
  bool m_enable;

  bool m_in_world;

};

class BulletGround : public PhysicalGround
{

public:
  BulletGround();
  ~BulletGround();

  // Set position
  void SetPosition(const Point2d &position);
  Point2d GetPosition() const;

  // Shape
  void AddShape(PhysicalShape *shape);

  #ifdef DEBUG
    void DrawShape(const Color& color) const;
  #endif

    double GetScale() const;
    btRigidBody* GetBody();
protected:
    PhysicalShape * m_shape;
    btRigidBody* m_body;
    Point2d m_position;
 };

#endif
