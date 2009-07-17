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
 * Bullet Shape
 *****************************************************************************/

#ifndef BULLET_SHAPE_H
#define BULLET_SHAPE_H

#include <physic/physical_shape.h>
#include "btBulletDynamicsCommon.h"

#ifdef DEBUG
class Color;
#endif

class BulletShape
{
public :
  BulletShape();
  btCollisionShape *GetNativeShape() { return m_native_shape; }
protected :
  btCollisionShape *m_native_shape;

};

class BulletPolygon : public PhysicalPolygon, public BulletShape
{

public:
  virtual ~BulletPolygon();
  virtual void Generate();

#ifdef DEBUG
  void DrawBorder(const Color &color) const;
#endif
};

class BulletRectangle : public PhysicalRectangle, public BulletShape
{
public:
  virtual ~BulletRectangle();
  BulletRectangle(double width, double height);
  virtual void Generate();
  virtual double Area() const;
  virtual double GetCurrentWidth() const;
  virtual double GetCurrentHeight() const;
  virtual double GetInitialWidth() const;
  virtual double GetInitialHeight() const;

  virtual double GetCurrentMinX() const;
  virtual double GetCurrentMaxX() const;
  virtual double GetCurrentMinY() const;
  virtual double GetCurrentMaxY() const;

  virtual double GetInitialMinX() const;
  virtual double GetInitialMaxX() const;
  virtual double GetInitialMinY() const;
  virtual double GetInitialMaxY() const;


#ifdef DEBUG
  void DrawBorder(const Color &color) const;
#endif
protected:
  btBoxShape m_shape;
};


class BulletCircle : public PhysicalCircle, public BulletShape
{

public:
  virtual ~BulletCircle();
  virtual void Generate();
#ifdef DEBUG
  void DrawBorder(const Color &color) const;
#endif
};



 #endif
