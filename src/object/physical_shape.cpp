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

#include "include/base.h"
#include "object/physical_shape.h"

#ifdef DEBUG
#include "graphic/color.h"
#include "graphic/video.h"
#include "map/camera.h"
#endif

PhysicalShape::PhysicalShape(b2Body *body) :
  m_body(body),
  m_shape(NULL),
  m_mass(-1),
  m_friction(0.8f)
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

void PhysicalShape::SetFriction(double friction)
{
  m_friction = friction;
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

  for (uint i = 0; i<m_point_list.size();i++) {
    shapeDef.vertices[i].Set(m_point_list[i].x, m_point_list[i].y);
  }

  shapeDef.density = 1.0f;
  shapeDef.friction = m_friction;
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

void PhysicalPolygone::Clear()
{
  m_point_list.clear();
}

double PhysicalPolygone::GetCurrentWidth() const
{
  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

  ASSERT(polygon->GetVertexCount() > 0);

  double width = 0;
  double minx = (polygon->GetVertices())[0].x;
  double maxx = (polygon->GetVertices())[0].x;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    if ((polygon->GetVertices())[i].x > maxx)
      maxx = (polygon->GetVertices())[i].x;

    if ((polygon->GetVertices())[i].x < minx)
      minx = (polygon->GetVertices())[i].x;
  }
  width = maxx - minx;
  return width;
}

double PhysicalPolygone::GetCurrentHeight() const
{
  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

  ASSERT(polygon->GetVertexCount() > 0);

  double height = 0;
  double miny = (polygon->GetVertices())[0].y;
  double maxy = (polygon->GetVertices())[0].y;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    if ((polygon->GetVertices())[i].y > maxy)
      maxy = (polygon->GetVertices())[i].y;

    if ((polygon->GetVertices())[i].y < miny)
      miny = (polygon->GetVertices())[i].y;
  }
  height = maxy - miny;
  return height;
}

double PhysicalPolygone::GetInitialWidth() const
{
  ASSERT(m_point_list.size() > 0);

  double width = 0;
  double minx = m_point_list[0].x;
  double maxx = m_point_list[0].x;

  for (uint i = 1; i<m_point_list.size(); i++) {

    if (m_point_list[i].x > maxx)
      maxx = m_point_list[i].x;

    if (m_point_list[i].x < minx)
      minx = m_point_list[i].x;
  }
  width = maxx - minx;
  return width;
}

double PhysicalPolygone::GetInitialHeight() const
{
  ASSERT(m_point_list.size() > 0);

  double height = 0;
  double miny = m_point_list[0].y;
  double maxy = m_point_list[0].y;

  for (uint i = 1; i<m_point_list.size(); i++) {

    if (m_point_list[i].y > maxy)
      maxy = m_point_list[i].y;

    if (m_point_list[i].y < miny)
      miny = m_point_list[i].y;
  }
  height = maxy - miny;
  return height;
}

#ifdef DEBUG
void PhysicalPolygone::DrawBorder(const Color& color) const
{
  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

  ASSERT(polygon->GetVertexCount() > 2);

  int init_x = (m_body->GetPosition().x + (polygon->GetVertices())[0].x)*PIXEL_PER_METER - Camera::GetInstance()->GetPosition().x;
  int init_y = (m_body->GetPosition().y + (polygon->GetVertices())[0].y)*PIXEL_PER_METER - Camera::GetInstance()->GetPosition().y;
  int prev_x = init_x;
  int prev_y = init_y;
  int x, y;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    x = (m_body->GetPosition().x + (polygon->GetVertices())[i].x)*PIXEL_PER_METER - Camera::GetInstance()->GetPosition().x;
    y = (m_body->GetPosition().y + (polygon->GetVertices())[i].y)*PIXEL_PER_METER - Camera::GetInstance()->GetPosition().y;

    GetMainWindow().LineColor(prev_x, x, prev_y, y, color);
    prev_x = x;
    prev_y = y;
  }

  GetMainWindow().LineColor(prev_x, init_x, prev_y, init_y, color);
}
#endif

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

