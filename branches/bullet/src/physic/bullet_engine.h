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
 * Arrow on top of the active character
 *****************************************************************************/

#ifndef _BULLET_ENGINE_H
#define	_BULLET_ENGINE_H
#include "physical_engine.h"
#include "btBulletDynamicsCommon.h"

class BulletEngine : public PhysicalEngine
{
    public:


  BulletEngine();
  ~BulletEngine();

  PhysicalObj *CreateObject(ObjectType type);
  PhysicalPolygon *CreatePolygonShape();
  PhysicalCircle *CreateCircleShape();
  PhysicalRectangle *CreateRectangleShape(double width, double height);

  void AddObject(PhysicalObj *new_obj);
  void RemoveObject(PhysicalObj *obj);
  void AddConstraint();
  void Step();
  void VirtualStep();

  void AddForce(Force * force);
  void RemoveForce(Force *force);

  void AddAirFrictionShape(PhysicalShape* shape);
  void RemoveAirFrictionShape(PhysicalShape* shape);

  void AddWindObject(PhysicalObj* i_object);
  void RemoveWindObject(PhysicalObj* i_object);

  void AddAutoAlignObject(PhysicalObj* object);
  void RemoveAutoAlignObject(PhysicalObj* object);

  void AddModifiedGravityObject(PhysicalObj* i_object);
  void RemoveModifiedGravityObject(PhysicalObj* i_object);

  void SetWindVector(const Point2d &i_wind_vector);

  static PhysicalEngine *GetInstance();
  static void SetInstance(PhysicalEngine * instance);


  double GetScale() const;
protected:
    btDiscreteDynamicsWorld *m_world;
    double m_scale;
};


#endif	/* _BULLET_ENGINE_H */

