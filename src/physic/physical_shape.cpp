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
 * Physical shape
 *****************************************************************************/

#include <sstream>
#include "include/base.h"
#include "physic/physical_shape.h"
#include "tool/math_tools.h"
#include "tool/xml_document.h"

#ifdef DEBUG
#include "graphic/color.h"
#include "graphic/video.h"
#include "map/camera.h"
#endif

PhysicalShape::PhysicalShape() :
  m_body(NULL),
  m_shape(NULL),
  m_position(0,0),
  m_mass(1),
  m_friction(0.8f),
  m_density(30),
  m_name("")
{
}

PhysicalShape::~PhysicalShape()
{
  if (m_shape && m_body) {
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

void PhysicalShape::SetPosition(Point2d position)
{
  m_position = position;
}

void PhysicalShape::SetBody(b2Body *body)
{
  m_body = body;
}

void PhysicalShape::SetName(const std::string &name)
{
  m_name = name;
}

const std::string &PhysicalShape::GetName() const
{
  return m_name;
}
const b2Shape *PhysicalShape::GetShape() const
{
  return m_shape;
}

// =============================================================================
// Static method
PhysicalShape * PhysicalShape::LoadFromXml(const xmlNode* root_shape)
{
  bool r;
  const xmlNode* elem = NULL;
  PhysicalShape * shape = NULL;
  std::string shape_type;
  std::string shape_name = "noname";

  r = XmlReader::ReadStringAttr(root_shape, "type", shape_type);
  if (!r) {
    fprintf(stderr, "Fails to read shape type from XML file\n");
    return NULL;
  }

  if (shape_type != "circle" && shape_type != "polygon") {
    fprintf(stderr, "Invalid shape type -%s- from XML file\n", shape_type.c_str());
    return NULL;
  }

  XmlReader::ReadStringAttr(root_shape, "name", shape_name);

  elem = XmlReader::GetMarker(root_shape, "position");
  if (!elem) {
    fprintf(stderr, "Fails to read shape position from XML file\n");
    return NULL;
  }

  uint pos_x, pos_y;
  r = XmlReader::ReadUintAttr(elem, "x", pos_x);
  if (r)
    r = XmlReader::ReadUintAttr(elem, "y", pos_y);

  if (!r) {
    fprintf(stderr, "Fails to read shape position from XML file\n");
    return NULL;
  }

  // =============== Circle

  if (shape_type == "circle") {

    uint radius;
    r = XmlReader::ReadUint(root_shape, "radius", radius);
    if (!r) {
      fprintf(stderr, "Fails to read circle radius from XML file\n");
      return NULL;
    }

    PhysicalCircle* circle = new PhysicalCircle();
    circle->SetRadius(double(radius)/PIXEL_PER_METER);
    shape = circle;

    // =============== Polygon
  } else if (shape_type == "polygon") {

    PhysicalPolygon* polygon = new PhysicalPolygon();

    xmlNodeArray points = XmlReader::GetNamedChildren(root_shape, "point");
    xmlNodeArray::const_iterator point;
    int i = 0;

    for (point = points.begin(); point != points.end(); point++) {

      if (i > 8) {
	fprintf(stderr, "You cannot set more than 8 points for a polygon!\n");
	delete polygon;
	return NULL;
      }
      i++;

      uint x, y;
      r = XmlReader::ReadUintAttr(*point, "x", x);
      if (r)
	r = XmlReader::ReadUintAttr(*point, "y", y);

      if (!r) {
	fprintf(stderr, "Invalid point definition!\n");
	delete polygon;
	return NULL;
      }


      polygon->AddPoint(Point2d(double(x)/PIXEL_PER_METER, double(y)/PIXEL_PER_METER));
    }

    shape = polygon;
  }

  shape->SetName(shape_name);
  shape->SetPosition(Point2d(double(pos_x)/PIXEL_PER_METER, double(pos_y)/PIXEL_PER_METER));

  return shape;
}
// =============================================================================

#ifdef DEBUG
static bool is_ccw(const std::vector<Point2d> P, int i, int j, int k)
{
  bool r;

  double a = P[i].x - P[j].x;
  double b = P[i].y - P[j].y;
  double c = P[k].x - P[j].x;
  double d = P[k].y - P[j].y;

  r = (a*d - b*c <= 0); // true if points i, j, k are counterclockwise
  return r;
}

static bool check_polygon_is_convex(const std::vector<Point2d>& point_list)
{
  ASSERT(point_list.size() >= 3);

  if (point_list.size() == 3)
    return true;

  bool ccw = is_ccw(point_list, point_list.size()-1, 0, 1);
  bool ccw2;
  for (uint i = 0; i < point_list.size() - 2; i++) {
    ccw2 = is_ccw(point_list, i, i+1, i+2);
    if (ccw != ccw2)
      return false;
    ccw = ccw2;
  }
  ccw2 = is_ccw(point_list, point_list.size()-2, point_list.size()-1, 0);
  if (ccw != ccw2)
    return false;

  return true;
}
#endif

/////////////////////////////////
// PhysicalPolygon

PhysicalPolygon::PhysicalPolygon() : PhysicalShape()
{

}

void PhysicalPolygon::AddPoint(Point2d point)
{
  m_point_list.push_back(point);
}

void PhysicalPolygon::Generate()
{
  if (m_shape) {
    m_body->DestroyShape(m_shape);
    m_shape = NULL;
  }

#ifdef DEBUG
  if (!check_polygon_is_convex(m_point_list)) {
    fprintf(stderr, "ERROR: PhysicalPolygon %s is not convex!\n", GetName().c_str());
    ASSERT(false);
  }
#endif

  b2PolygonDef shapeDef;
  shapeDef.vertexCount = m_point_list.size();

  for (uint i = 0; i < m_point_list.size(); i++) {
    shapeDef.vertices[i].Set(m_point_list[i].x, m_point_list[i].y);
  }

  shapeDef.density = m_density;
  shapeDef.friction = m_friction;
  shapeDef.restitution = 0.1f;
  shapeDef.filter.categoryBits = m_filter.categoryBits;
  shapeDef.filter.maskBits = m_filter.maskBits;
  shapeDef.filter.groupIndex = m_filter.groupIndex;
  m_shape = m_body->CreateShape(&shapeDef);

  //b2MassData massData;
 // massData.mass = m_mass;
 // massData.center.SetZero();
 // massData.I = 1.0f;
 // m_shape->ComputeMass(&massData);


  //m_body->SetMass(&massData);
}

void PhysicalPolygon::Clear()
{
  m_point_list.clear();
}

double PhysicalPolygon::GetCurrentMinX() const
{
  ASSERT(m_shape);

  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;
  ASSERT(polygon->GetVertexCount() > 0);

  double minx = (polygon->GetVertices())[0].x;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    if ((polygon->GetVertices())[i].x < minx)
      minx = (polygon->GetVertices())[i].x;
  }

  return m_body->GetPosition().x + minx;
}

double PhysicalPolygon::GetCurrentMaxX() const
{
  ASSERT(m_shape);

  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

  ASSERT(polygon->GetVertexCount() > 0);

  double maxx = (polygon->GetVertices())[0].x;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    if ((polygon->GetVertices())[i].x > maxx)
      maxx = (polygon->GetVertices())[i].x;
  }

  return m_body->GetPosition().x + maxx;
}

double PhysicalPolygon::GetCurrentMinY() const
{
  ASSERT(m_shape);

  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

  ASSERT(polygon->GetVertexCount() > 0);

  double miny = (polygon->GetVertices())[0].y;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    if ((polygon->GetVertices())[i].y < miny)
      miny = (polygon->GetVertices())[i].y;
  }

  return m_body->GetPosition().y + miny;
}

double PhysicalPolygon::GetCurrentMaxY() const
{
  ASSERT(m_shape);

  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

  ASSERT(polygon->GetVertexCount() > 0);

  double maxy = (polygon->GetVertices())[0].y;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    if ((polygon->GetVertices())[i].y > maxy)
      maxy = (polygon->GetVertices())[i].y;
  }

  return m_body->GetPosition().y + maxy;
}

double PhysicalPolygon::GetCurrentWidth() const
{
  ASSERT(m_shape);

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

double PhysicalPolygon::GetCurrentHeight() const
{
  ASSERT(m_shape);

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

double PhysicalPolygon::GetInitialMinX() const
{
  ASSERT(m_point_list.size() > 0);
  double minx = m_point_list[0].x;

  for (uint i = 1; i<m_point_list.size(); i++) {

    if (m_point_list[i].x < minx)
      minx = m_point_list[i].x;
  }

  return minx;
}

double PhysicalPolygon::GetInitialMaxX() const
{
  ASSERT(m_point_list.size() > 0);
  double maxx = m_point_list[0].x;

  for (uint i = 1; i<m_point_list.size(); i++) {

    if (m_point_list[i].x > maxx)
      maxx = m_point_list[i].x;
  }

  return maxx;
}

double PhysicalPolygon::GetInitialWidth() const
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

double PhysicalPolygon::GetInitialMinY() const
{
  ASSERT(m_point_list.size() > 0);
  double miny = m_point_list[0].y;

  for (uint i = 1; i<m_point_list.size(); i++) {
    if (m_point_list[i].y < miny)
      miny = m_point_list[i].y;
  }

  return miny;
}

double PhysicalPolygon::GetInitialMaxY() const
{
  ASSERT(m_point_list.size() > 0);
  double maxy = m_point_list[0].y;

  for (uint i = 1; i<m_point_list.size(); i++) {
    if (m_point_list[i].y > maxy)
      maxy = m_point_list[i].y;
  }

  return maxy;
}

double PhysicalPolygon::GetInitialHeight() const
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
void PhysicalPolygon::DrawBorder(const Color& color) const
{
  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

  ASSERT(polygon->GetVertexCount() > 2);
  b2XForm xf = m_body->GetXForm();
  int init_x = lround(( b2Mul(xf,polygon->GetVertices()[0]).x)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x;
  int init_y = lround(( b2Mul(xf,polygon->GetVertices()[0]).y)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y;
  int prev_x = init_x;
  int prev_y = init_y;
  int x, y;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    x = lround(( b2Mul(xf,polygon->GetVertices()[i]).x)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x;
    y = lround(( b2Mul(xf,polygon->GetVertices()[i]).y)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y;

    GetMainWindow().LineColor(prev_x, x, prev_y, y, color);
    prev_x = x;
    prev_y = y;
  }

  GetMainWindow().LineColor(prev_x, init_x, prev_y, init_y, color);
}
#endif

/////////////////////////////////
// PhysicalRectangle

PhysicalRectangle::PhysicalRectangle( double width, double height) : PhysicalPolygon()
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

  PhysicalPolygon::Generate();
}

/////////////////////////////////
// PhysicalCircle

PhysicalCircle::PhysicalCircle() : PhysicalShape()
{

}

void PhysicalCircle::SetRadius(double radius)
{
  m_radius = radius;
}

void PhysicalCircle::Generate()
{
  if (m_shape) {
    m_body->DestroyShape(m_shape);
    m_shape = NULL;
  }

  b2CircleDef shapeDef;
  shapeDef.radius = m_radius;
  shapeDef.localPosition.Set(m_position.x, m_position.y);
  shapeDef.density = m_density;
  shapeDef.friction = m_friction;
  shapeDef.restitution = 0.1f;
  shapeDef.filter.categoryBits = m_filter.categoryBits;
  shapeDef.filter.maskBits = m_filter.maskBits;
  shapeDef.filter.groupIndex = m_filter.groupIndex;
  m_shape = m_body->CreateShape(&shapeDef);

  b2MassData massData;
  massData.mass = m_mass;
  massData.center.SetZero();
  massData.I = 1.0f;

  m_body->SetMass(&massData);
}

double PhysicalCircle::GetCurrentMinX() const
{
  return m_body->GetPosition().x + GetInitialMinX();
}

double PhysicalCircle::GetCurrentMaxX() const
{
  return m_body->GetPosition().x + GetInitialMaxX();
}

double PhysicalCircle::GetCurrentMinY() const
{
  return m_body->GetPosition().y + GetInitialMinY();
}

double PhysicalCircle::GetCurrentMaxY() const
{
  return m_body->GetPosition().y + GetInitialMaxY();
}

double PhysicalCircle::GetInitialMinX() const
{
  return m_position.x - m_radius;
}

double PhysicalCircle::GetInitialMaxX() const
{
  return m_position.x + m_radius;
}

double PhysicalCircle::GetInitialMinY() const
{
  return m_position.y - m_radius;
}

double PhysicalCircle::GetInitialMaxY() const
{
  return m_position.y + m_radius;
}

double PhysicalCircle::GetCurrentWidth() const
{
  return GetInitialWidth();
}

double PhysicalCircle::GetCurrentHeight() const
{
  return GetInitialHeight();
}

double PhysicalCircle::GetInitialWidth() const
{
  return m_radius*2;
}

double PhysicalCircle::GetInitialHeight() const
{
  return m_radius*2;
}

#ifdef DEBUG
void PhysicalCircle::DrawBorder(const Color& color) const
{
  int radius = m_radius * PIXEL_PER_METER;

  int x = lround((m_body->GetPosition().x + m_position.x)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x;
  int y = lround((m_body->GetPosition().y + m_position.y)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y;

  GetMainWindow().CircleColor(x, y, radius, color);
}
#endif
