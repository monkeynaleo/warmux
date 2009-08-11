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
 * Physical Shape
 *****************************************************************************/

#ifndef PHYSICAL_SHAPE_H
#define PHYSICAL_SHAPE_H

#include <WORMUX_point.h>
#include <vector>
#include <string>
#include "tool/xml_document.h"
#include "physical_obj.h"

#ifdef DEBUG
class Color;
#endif

class PhysicalShape
{
protected:
  PhysicalEntity *m_parent;

  Point2d m_position;
  Point2d m_force_application_point;
  double m_friction;
  double m_air_friction;
  double m_rebound_factor;
  double m_density;
  std::string m_name;
  Point2d PosWithRotation(const Point2d& point) const;

public:
  PhysicalShape();
  virtual ~PhysicalShape();
  virtual void Generate() = 0;
  virtual double Area() const = 0;

  void SetFriction(double friction);
  void SetReboundFactor(double rebound_factor);
  void SetMass(double mass); // compute the density from mass and area
  void SetPosition(Point2d position);
  void SetName(const std::string &name);
  void SetAirFriction(double air_friction);
  void SetForceApplicationPoint(Point2d point);

  void SetParent(PhysicalEntity *parent);


  virtual bool IsColliding() const = 0;

  const std::string &GetName() const;
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

  virtual double GetInitialMinX() const = 0;
  virtual double GetInitialMaxX() const = 0;
  virtual double GetInitialMinY() const = 0;
  virtual double GetInitialMaxY() const = 0;

  Point2d GetPosition() const;

  void ComputeAirFriction();

  // TODO: REMOVE IT IN NEAR FUTURE
  double GetMass() const;

  static PhysicalShape* LoadFromXml(const xmlNode* root_shape);

#ifdef DEBUG
  virtual void DrawBorder(const Color &color) const = 0;
#endif
};


class PhysicalPolygon : public PhysicalShape
{
protected:
  std::vector<Point2d> m_point_list;

public:
  PhysicalPolygon();
  void AddPoint(Point2d point);
  void Clear();

  // Box2D support only convex polygons
  bool IsConvex();

  virtual void Generate() = 0;
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
};

class PhysicalRectangle : public PhysicalShape
{
protected:
  double m_width;
  double m_height;

public:
  PhysicalRectangle(double width, double height);
  virtual void Generate() = 0;



  virtual double Area() const = 0;

  virtual double GetCurrentWidth() const = 0;
   virtual double GetCurrentHeight() const = 0;
   virtual double GetInitialWidth() const = 0;
   virtual double GetInitialHeight() const = 0;

   virtual double GetCurrentMinX() const = 0;
   virtual double GetCurrentMaxX() const = 0;
   virtual double GetCurrentMinY() const = 0;
   virtual double GetCurrentMaxY() const = 0;

   virtual double GetInitialMinX() const = 0;
   virtual double GetInitialMaxX() const = 0;
   virtual double GetInitialMinY() const = 0;
   virtual double GetInitialMaxY() const = 0;

  #ifdef DEBUG
    virtual void DrawBorder(const Color &color) const = 0;
  #endif
};


class PhysicalCircle : public PhysicalShape
{
protected:
  double m_radius;

public:
  PhysicalCircle();
  void SetRadius(double radius);

  virtual void Generate() = 0;
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
};



 #endif
