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

#ifndef PHYSICAL_ENGINE_H
#define PHYSICAL_ENGINE_H
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

extern const double PIXEL_PER_METER;

class PhysicalObj;
class Force;
class ContactListener;
class DebugDraw;
class PhysicalShape;
class PhysicalCircle;
class PhysicalPolygon;
class PhysicalRectangle;

#include "WORMUX_point.h"
#include "WORMUX_types.h"
class PhysicalEngine
{
public:

    enum ObjectType { RIGID_BODY, SOFT_BODY };

  PhysicalEngine();
  virtual ~PhysicalEngine();

  virtual PhysicalObj *CreateObject(ObjectType type) = 0;
  virtual PhysicalPolygon *CreatePolygonShape() = 0;
  virtual PhysicalCircle *CreateCircleShape() = 0;
  virtual PhysicalRectangle *CreateRectangleShape(double width, double height) = 0;

  virtual void AddObject(PhysicalObj *new_obj) =0;
  virtual void RemoveObject(PhysicalObj *obj) =0;
  void AddConstraint();
  virtual void Step() = 0;
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

protected:
  uint m_frame_rate;
  uint m_last_step_time;
  Point2d m_wind_vector;
  static PhysicalEngine *g_instance;
};


//-----------------------------------------------------------------------------
#endif
