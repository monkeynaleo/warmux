/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
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
 * Physical Shape
 *****************************************************************************/

#ifndef PHYSICAL_SHAPE_H
#define PHYSICAL_SHAPE_H

#include "tool/point.h"
#include <vector>

#include <Box2D.h>

extern const double PIXEL_PER_METER;
#ifdef DEBUG
class Color;
#endif

class PhysicalShape
{
public:
  PhysicalShape();
  virtual ~PhysicalShape();
  virtual void Generate() = 0;

  const b2FilterData& GetFilter() const;
  void SetFilter(b2FilterData filter);
  void SetFriction(double friction);
  void SetMass(int mass);
  void SetPosition(Point2d position);

  void SetBody(b2Body *body);
  // returns current max width (taking angle into account)
  virtual double GetCurrentWidth() const = 0;

  // returns current max height (taking angle into account)
  virtual double GetCurrentHeight() const = 0;

  virtual double GetCurrentMinX() const = 0;
  virtual double GetCurrentMaxX() const = 0;
  virtual double GetCurrentMinY() const = 0;
  virtual double GetCurrentMaxY() const = 0;

  virtual double GetInitialWidth() const = 0;
  virtual double GetInitialHeight() const = 0;

#ifdef DEBUG
  virtual void DrawBorder(const Color &color) const = 0;
#endif

protected:
  b2FilterData m_filter;
  b2Body *m_body;
  b2Shape *m_shape;
  Point2d m_position;
  int m_mass;
  double m_friction;
};


class PhysicalPolygon : public PhysicalShape
{
public:
  PhysicalPolygon();
  void AddPoint(Point2d point);
  void Clear();
  virtual void Generate();
  virtual double GetCurrentWidth() const;
  virtual double GetCurrentHeight() const;
  virtual double GetInitialWidth() const;
  virtual double GetInitialHeight() const;

  virtual double GetCurrentMinX() const;
  virtual double GetCurrentMaxX() const;
  virtual double GetCurrentMinY() const;
  virtual double GetCurrentMaxY() const;

#ifdef DEBUG
  void DrawBorder(const Color &color) const;
#endif
protected:
  std::vector<Point2d> m_point_list;
};

class PhysicalRectangle : public PhysicalPolygon
{
public:
  PhysicalRectangle( double width, double height);
  virtual void Generate();
protected:
  double m_width;
  double m_height;
};


class PhysicalCircle : public PhysicalShape
{
public:
  PhysicalCircle();
  void SetRadius(double radius);
  virtual void Generate();
  virtual double GetCurrentWidth() const;
  virtual double GetCurrentHeight() const;
  virtual double GetInitialWidth() const;
  virtual double GetInitialHeight() const;

  virtual double GetCurrentMinX() const;
  virtual double GetCurrentMaxX() const;
  virtual double GetCurrentMinY() const;
  virtual double GetCurrentMaxY() const;

#ifdef DEBUG
  void DrawBorder(const Color &color) const;
#endif
protected:
  double m_radius;
};



 #endif
