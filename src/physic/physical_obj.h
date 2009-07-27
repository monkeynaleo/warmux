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

#ifndef PHYSICAL_OBJECT_H
#define PHYSICAL_OBJECT_H
#include <vector>
#include <string>
#include "WORMUX_point.h"
#include "WORMUX_rectangle.h"
#include "WORMUX_types.h"
#include "tool/xml_document.h"
#include "graphic/color.h"

class PhysicalShape;
class PhysicalListener;
class Force;

class PhysicalObj
{
  
public: 
  PhysicalObj();
  /* Note : The copy constructor is not implemented (and this is not a bug)
   * because we can copy directly the pointer m_overlapping_object whereas this
   * object does not own it.
   * FIXME what happen if the object is deleted meanwhile ???*/
  virtual ~PhysicalObj ();

  virtual void LoadFromXml(const xmlNode &xml_obj) = 0;

  virtual void Activate() = 0;
  virtual void Generate() = 0;
  virtual void Desactivate() = 0;

  //-------- Position, speed and size -------

  // Set/Get position
  virtual void SetPosition(const Point2d &position) = 0;
  virtual const Point2d GetPosition() const  = 0;
  virtual void SetAngle(double angle_rad) = 0;
  virtual double GetAngle() const = 0;
  
  //State
  virtual void SetFixed(bool i_fixed) = 0;
  virtual bool IsFixed() = 0;
  virtual void SetRotationFixed(bool rotating) = 0;
  virtual bool IsRotationFixed() const { return m_rotating; }
  virtual void StopMovement() = 0;
  virtual void SetFast(bool is_fast) = 0;
  virtual bool IsFast() = 0;
  
 // Speed
  virtual void SetSpeedXY(Point2d vector) = 0;
  virtual void SetSpeed(double norm, double angle_rad) = 0;
  virtual void SetAngularSpeed(double speed) = 0;
  virtual void GetSpeed(double &norm, double &angle_rad) const = 0;
  virtual Point2d GetSpeed() const = 0;
  virtual double GetAngularSpeed() const = 0;
  virtual bool IsMoving() const = 0;
  virtual bool IsSleeping() const = 0;
  // Shape
  virtual void AddShape(PhysicalShape *shape,std::string name ="") = 0;
  virtual void RemoveShape(PhysicalShape *shape) = 0;
  virtual void RemoveShape(std::string name) = 0;
  virtual void ClearShapes() = 0;
  virtual void LoadShapeFromXml(const xmlNode &xml_config) = 0;

   virtual PhysicalShape *GetShape(const std::string &name) = 0;
   virtual Rectangled GetBoundingBox() = 0;
  //  Mass
  virtual double GetMass() const = 0;
 
  // Force
  virtual Force *AddExternForceXY (const Point2d& vector) = 0;
  virtual Force *AddExternForce (double norm, double angle) = 0;
  virtual void RemoveExternForce(Force *force_index) = 0;
  virtual void RemoveAllExternForce() = 0;
  virtual void ImpulseXY(const Point2d& vector) = 0;
  virtual void Impulse(double norm, double angle) = 0;
  
  // Collision
  
    enum CollisionCategory {
        COLLISION_GROUND,
        COLLISION_CHARACTER,
        COLLISION_ITEM,
        COLLISION_PROJECTILE
    };

  virtual void SetCollisionMembership(CollisionCategory category, bool state) = 0;
  virtual void SetCollisionCategory(CollisionCategory category,bool state) = 0;
  virtual bool IsColliding() const = 0;
  
  virtual PhysicalObj* CollidedObjectXY(const Point2i & position) const = 0;
  // Relative to current position
  virtual PhysicalObj* CollidedObject(const Point2i & offset = Point2i(0,0)) const = 0;

  virtual void AddReboundListener(PhysicalListener *listener) = 0;
  virtual void AddCollisionListener(PhysicalListener *listener) = 0;

  virtual bool Contain(const Point2d &pos_to_check) = 0;
 
  //Overlapping
  virtual void AddOverlappingObject(PhysicalObj* obj, int timeout = 0) = 0;
  virtual void ClearOverlappingObject(PhysicalObj* obj) = 0;
  virtual void ClearAllOverlappingObject() = 0;
  virtual bool IsOverlappingObject(PhysicalObj *obj) = 0;
  virtual bool Overlapse(const PhysicalObj* obj) const = 0;
  virtual const std::vector<PhysicalObj*> *GetOverlappingObject() const = 0;
  
  //Properties
  enum PhysicalProperty {
      PROPERTY_FRICTION_FACTOR,
      PROPERTY_REBOUND_FACTOR,
      PROPERTY_AIR_FRICTION_FACTOR,
      PROPERTY_WIND_FACTOR,
      PROPERTY_AUTO_ALIGN_FACTOR,
      PROPERTY_GRAVITY_FACTOR
      
  };

  // Reset the physics constants (mass, air_resistance...) to the default values in the cfg
  virtual void SetFrictionFactor( double value) = 0;
  virtual void ResetFrictionFactor() = 0;
  virtual double GetFritionFactor() = 0;

  virtual void SetReboundFactor( double value) = 0;
  virtual void ResetReboundFactor() = 0;
  virtual double GetReboundFactor() = 0;

  virtual void SetAirFrictionFactor( double value) = 0;
  virtual void ResetAirFrictionFactor() = 0;
  virtual double GetAirFrictionFactor() = 0;

  virtual void SetWindFactor( double value) = 0;
  virtual void ResetWindFactor() = 0;
  virtual double GetWindFactor() = 0;

  virtual void SetAutoAlignFactor( double value) = 0;
  virtual void ResetAutoAlignFactor() = 0;
  virtual double GetAutoAlignFactor() = 0;

  virtual void SetGravityFactor( double value) = 0;
  virtual void ResetGravityFactor() = 0;
  virtual double GetGravityFactor() = 0;

  #ifdef DEBUG
    virtual void DrawShape(const Color& color) const =0;
  #endif

protected:
  virtual void SignalRebound() = 0;
  virtual void SignalCollision(const Point2d&) = 0 ;
  
  bool m_rotating;

};

#endif
