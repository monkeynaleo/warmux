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
 * Physical shape
 *****************************************************************************/

#include "object/physical_shape.h"

PhysicalShape::PhysicalShape(b2Body *body) :
  m_body(body),
  m_shape(NULL),
  m_mass(-1)
{
}

PhysicalShape::~PhysicalShape()
{
  if (m_shape) {
    m_body->DestroyShape(m_shape);
  }
}

const b2FilterData& PhysicalShape::GetFilter() const
{
  return m_filter;
}

void PhysicalShape::SetFilter(b2FilterData filter)
{
  m_filter = filter;
}

void PhysicalShape::SetMass(int mass)
{
  m_mass = mass;
}

/////////////////////////////////
// PhysicalPolygone

PhysicalPolygone::PhysicalPolygone(b2Body *body) : PhysicalShape(body)
{

}

void PhysicalPolygone::AddPoint(Point2d point)
{
  m_point_list.push_back(point);
}

void PhysicalPolygone::Generate()
{
  if (m_shape) {
    m_body->DestroyShape(m_shape);
    m_shape = NULL;
  }

  b2PolygonDef shapeDef;
  shapeDef.vertexCount = m_point_list.size();

  for (unsigned i = 0; i<m_point_list.size();i++) {
    shapeDef.vertices[i].Set(m_point_list[i].x, m_point_list[i].y);
  }

  shapeDef.density = 1.0f;
  shapeDef.friction = 0.8f;
  shapeDef.restitution = 0.1f;
  shapeDef.filter.categoryBits = m_filter.categoryBits;
  shapeDef.filter.maskBits = m_filter.maskBits;

  m_shape = m_body->CreateShape(&shapeDef);

  b2MassData massData;
  massData.mass = m_mass;
  massData.center.SetZero();
  massData.I = 0.0f;

  m_body->SetMass(&massData);
}

/////////////////////////////////
// PhysicalRectangle

PhysicalRectangle::PhysicalRectangle(b2Body *body, double width, double height) : PhysicalPolygone(body)
{
  m_width = width;
  m_height = height;

}

void PhysicalRectangle::Generate()
{

  m_point_list.clear();
  AddPoint(Point2d(0,0));
  AddPoint(Point2d(m_width,0));
  AddPoint(Point2d(m_width,m_height));
  AddPoint(Point2d(0,m_height));

  PhysicalPolygone::Generate();

}

