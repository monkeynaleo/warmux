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
#include "physic/physical_engine.h"
#include "physic/physical_shape.h"
#include "tool/math_tools.h"
#include "tool/xml_document.h"

#ifdef DEBUG
#include "graphic/color.h"
#include "graphic/video.h"
#include "map/camera.h"
#endif

PhysicalShape::PhysicalShape() :
  m_parent(NULL),
  m_position(0,0),
  m_force_application_point(0,0),
  m_friction(0.8f),
  m_air_friction(0),
  m_rebound_factor(0.1f),
  m_density(30),
  m_name("")
{
}

PhysicalShape::~PhysicalShape()
{
/*  PhysicalEngine::GetInstance()->RemoveAirFrictionShape(this);
  if (m_shape && m_body) {
    m_body->DestroyShape(m_shape);
  }*/

}


void PhysicalShape::SetParent(PhysicalObj *parent)
{
  ASSERT(parent);
  m_parent = parent;
}

void PhysicalShape::SetMass(double mass)
{
  // Compute density from mass and area
  m_density = mass / Area();
}

void PhysicalShape::SetFriction(double /*friction*/)
{
  //m_friction = friction;
}

void PhysicalShape::SetReboundFactor(double rebound_factor)
{
  m_rebound_factor = rebound_factor;
}

Point2d PhysicalShape::GetPosition() const
{
  return m_position;
 }

void PhysicalShape::SetPosition(Point2d position)
{
  m_position = position;
}

void PhysicalShape::SetName(const std::string &name)
{
  m_name = name;
}

const std::string &PhysicalShape::GetName() const
{
  return m_name;
}

// TODO: REMOVE IT IN NEAR FUTURE
double PhysicalShape::GetMass() const
{
  return m_density * Area();
}
/*
Point2d PhysicalShape::PosWithRotation(const b2Vec2& point) const
{

  if (m_body->GetAngle() == 0)
    return Point2d(point.x + m_body->GetPosition().x,
		   point.y + m_body->GetPosition().y);

  Point2d p;

  p.x = point.x * cos(-m_body->GetAngle())
    + point.y  * sin(-m_body->GetAngle())
    + m_body->GetPosition().x;

  p.y =  -point.x * sin(-m_body->GetAngle())
    + point.y * cos(-m_body->GetAngle())
    + m_body->GetPosition().y;

  MSG_DEBUG("shape.rotate", "Body pos %s: %f\t %f - angle: %f\n"
	    "original point: %f \t %f\n"
	    "point rotated: %f \t %f\n\n",
	    GetName().c_str(),
	    m_body->GetPosition().x, m_body->GetPosition().y, m_body->GetAngle(),
	    point.x, point.y,
	    p.x, p.y);

  return p;
}
*/

void PhysicalShape::ComputeAirFriction()
{
  //if(m_body){
//  b2Vec2 force(m_body->GetLinearVelocity().x * m_air_friction *-1,m_body->GetLinearVelocity().y * m_air_friction *-1);
/*  m_body->ApplyForce( force,
                      m_body->GetWorldPoint(b2Vec2( m_position.x+m_force_application_point.x,
                                                    m_position.y+m_force_application_point.y)));*/
//  }
}


void PhysicalShape::SetForceApplicationPoint(Point2d point)
{
  m_force_application_point = point;
}

void PhysicalShape::SetAirFriction(double air_friction)
{
  if(air_friction == 0 && m_air_friction != 0)
  {
    PhysicalEngine::GetInstance()->RemoveAirFrictionShape(this);
  } else if( air_friction > 0 && m_air_friction == 0)
  {
    PhysicalEngine::GetInstance()->AddAirFrictionShape(this);
  }
  m_air_friction = air_friction;
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
    fprintf(stderr, "Fails to read shape (%s) position from XML file\n", shape_name.c_str());
    return NULL;
  }

  uint pos_x, pos_y;
  r = XmlReader::ReadUintAttr(elem, "x", pos_x);
  if (r)
    r = XmlReader::ReadUintAttr(elem, "y", pos_y);

  if (!r) {
    fprintf(stderr, "Fails to read shape (%s) position from XML file\n", shape_name.c_str());
    return NULL;
  }


  elem = XmlReader::GetMarker(root_shape, "application_point");
   uint force_x, force_y;
  if(elem){
    r = XmlReader::ReadUintAttr(elem, "x", force_x);
    if (r)
      r = XmlReader::ReadUintAttr(elem, "y", force_y);

    if (!r) {
      fprintf(stderr, "Fails to read shape (%s) force application point from XML file\n", shape_name.c_str());
      return NULL;
    }
  }


  double mass;
  r = XmlReader::ReadDouble(root_shape
  , "mass", mass);
  if (!r) {
    fprintf(stderr, "Fails to read shape  (%s) mass from XML file\n", shape_name.c_str());
    return NULL;
  }

  double air_friction = 0;
  r = XmlReader::ReadDouble(root_shape, "air_friction", air_friction);

  double friction = 0.8;
   r = XmlReader::ReadDouble(root_shape, "friction", friction);
  
  // =============== Circle

  if (shape_type == "circle") {

    uint radius;
    r = XmlReader::ReadUint(root_shape, "radius", radius);
    if (!r) {
      fprintf(stderr, "Fails to read circle  (%s) radius from XML file\n", shape_name.c_str());
      return NULL;
    }

    PhysicalCircle* circle = PhysicalEngine::GetInstance()->CreateCircleShape();
    circle->SetRadius(double(radius));
    shape = circle;

    // =============== Polygon
  } else if (shape_type == "polygon") {

    PhysicalPolygon* polygon = PhysicalEngine::GetInstance()->CreatePolygonShape();

    xmlNodeArray points = XmlReader::GetNamedChildren(root_shape, "point");
    xmlNodeArray::const_iterator point;
    int i = 0;

    for (point = points.begin(); point != points.end(); point++) {

      if (i > 8) {
	fprintf(stderr, "You cannot set more than 8 points for a polygon (%s) !\n", shape_name.c_str());
	ASSERT(false);

	delete polygon;
	return NULL;
      }
      i++;

      uint x, y;
      r = XmlReader::ReadUintAttr(*point, "x", x);
      if (r)
	r = XmlReader::ReadUintAttr(*point, "y", y);

      if (!r) {
	fprintf(stderr, "Invalid point definition (%s) !\n", shape_name.c_str());
	ASSERT(false);

	delete polygon;
	return NULL;
      }


      polygon->AddPoint(Point2d(double(x), double(y)));
    }

    if (!polygon->IsConvex()) {
      fprintf(stderr, "ERROR: PhysicalPolygon %s is not convex!\n", shape_name.c_str());
      ASSERT(false);

      delete polygon;
      return NULL;
    }

    shape = polygon;
  }


  shape->SetMass(mass);
  shape->SetName(shape_name);
  shape->SetPosition(Point2d(double(pos_x), double(pos_y)));
  shape->SetAirFriction(air_friction);
  shape->SetForceApplicationPoint(Point2d(force_x, force_y));
  shape->SetFriction(friction);

  return shape;
}


// =============================================================================

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

/////////////////////////////////
// PhysicalPolygon

PhysicalPolygon::PhysicalPolygon() : PhysicalShape()
{

}

void PhysicalPolygon::AddPoint(Point2d point)
{
  m_point_list.push_back(point);
}

bool PhysicalPolygon::IsConvex()
{
  ASSERT(m_point_list.size() >= 3);

  if (m_point_list.size() == 3)
    return true;

  bool ccw = is_ccw(m_point_list, m_point_list.size()-1, 0, 1);
  bool ccw2;
  for (uint i = 0; i < m_point_list.size() - 2; i++) {
    ccw2 = is_ccw(m_point_list, i, i+1, i+2);
    if (ccw != ccw2)
      return false;
    ccw = ccw2;
  }
  ccw2 = is_ccw(m_point_list, m_point_list.size()-2, m_point_list.size()-1, 0);
  if (ccw != ccw2)
    return false;

  return true;
}

void PhysicalPolygon::Generate()
{
/*  if (m_shape) {
    m_body->DestroyShape(m_shape);
    m_shape = NULL;
  }*/

#ifdef DEBUG
  if (!IsConvex()) {
    fprintf(stderr, "ERROR: PhysicalPolygon %s is not convex!\n", GetName().c_str());
    ASSERT(false);
  }
#endif
/*
  b2PolygonDef shapeDef;
  shapeDef.vertexCount = m_point_list.size();

  for (uint i = 0; i < m_point_list.size(); i++) {
    shapeDef.vertices[i].Set(m_point_list[i].x, m_point_list[i].y);
  }

  shapeDef.density = m_density;
  shapeDef.friction = m_friction;
  shapeDef.restitution = m_rebound_factor;
  shapeDef.filter.categoryBits = m_filter.categoryBits;
  shapeDef.filter.maskBits = m_filter.maskBits;
  shapeDef.filter.groupIndex = m_filter.groupIndex;
  m_shape = m_body->CreateShape(&shapeDef);

  //b2MassData massData;
 // massData.mass = m_mass;
 // massData.center.SetZero();
 // massData.I = 1.0f;
 // m_shape->ComputeMass(&massData);


  //m_body->SetMass(&massData);*/
}

double PhysicalPolygon::Area() const
{
  /*if (m_shape) {
    b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

    float32 area = 0.0f;

    for (int32 i = 0; i < polygon->GetVertexCount(); i++) {
      // Triangle vertices.
      b2Vec2 p1 = polygon->GetVertices()[i];
      b2Vec2 p2 = i + 1 < polygon->GetVertexCount() ? polygon->GetVertices()[i+1] : polygon->GetVertices()[0];

      float32 D = b2Cross(p1, p2);

      float32 triangleArea = 0.5f * D;
      area += triangleArea;
    }
    return area;

  }

  Point2d pRef(0, 0);
  double area = 0;
  for (uint i = 0; i < m_point_list.size(); i++) {
    Point2d p1 = m_point_list[i];
    Point2d p2 = i + 1 < m_point_list.size() ? m_point_list[i+1] : m_point_list[0];
    float32 D = b2Cross(b2Vec2(p1.x, p1.y), b2Vec2(p2.x, p2.y));
    float32 triangleArea = 0.5f * D;
    area += triangleArea;
  }
  return area;*/
    return 0;
}

void PhysicalPolygon::Clear()
{
  m_point_list.clear();
}

double PhysicalPolygon::GetCurrentMinX() const
{
 /* ASSERT(m_shape);

  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;
  ASSERT(polygon->GetVertexCount() > 0);

  double minx = PosWithRotation(polygon->GetVertices()[0]).x;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    double _x = PosWithRotation(polygon->GetVertices()[i]).x;
    if (_x < minx)
      minx = _x;
  }

  return minx;*/
    return 0;
}

double PhysicalPolygon::GetCurrentMaxX() const
{
/*  ASSERT(m_shape);

  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

  ASSERT(polygon->GetVertexCount() > 0);

  double maxx = PosWithRotation(polygon->GetVertices()[0]).x;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    double _x = PosWithRotation(polygon->GetVertices()[i]).x;
    if (_x > maxx)
      maxx = _x;
  }

  return maxx;*/
    return 0;
}

double PhysicalPolygon::GetCurrentMinY() const
{
/*  ASSERT(m_shape);

  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

  ASSERT(polygon->GetVertexCount() > 0);

  double miny = PosWithRotation(polygon->GetVertices()[0]).y;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    double _y = PosWithRotation(polygon->GetVertices()[i]).y;
    if (_y < miny)
      miny = _y;
  }

  return miny;*/
    return 0;
}

double PhysicalPolygon::GetCurrentMaxY() const
{
/*  ASSERT(m_shape);

  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

  ASSERT(polygon->GetVertexCount() > 0);

  double maxy = PosWithRotation(polygon->GetVertices()[0]).y;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    double _y = PosWithRotation(polygon->GetVertices()[i]).y;
    if (_y > maxy)
      maxy = _y;
  }

  return maxy;*/
    return 0;
}

double PhysicalPolygon::GetCurrentWidth() const
{
/*  ASSERT(m_shape);

  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

  ASSERT(polygon->GetVertexCount() > 0);

  double width = 0;
  double minx = PosWithRotation(polygon->GetVertices()[0]).x;
  double maxx = minx;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    double _x = PosWithRotation(polygon->GetVertices()[i]).x;

    if (_x > maxx)
      maxx = _x;

    if (_x < minx)
      minx = _x;
  }
  width = maxx - minx;
  return width;*/
    return 0;
}

double PhysicalPolygon::GetCurrentHeight() const
{
 /* ASSERT(m_shape);

  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

  ASSERT(polygon->GetVertexCount() > 0);

  double height = 0;
  double miny = PosWithRotation(polygon->GetVertices()[0]).y;
  double maxy = miny;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    double _y = PosWithRotation(polygon->GetVertices()[i]).y;

    if (_y > maxy)
      maxy = _y;

    if (_y < miny)
      miny = _y;
  }
  height = maxy - miny;
  return height;*/
    return 0;
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
    return 0;
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
    return 0;
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
    return 0;
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
    return 0;
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
    return 0;
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
    return 0;
}

#ifdef DEBUG
void PhysicalPolygon::DrawBorder(const Color& /*color*/) const
{



/*  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

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

  GetMainWindow().LineColor(prev_x, init_x, prev_y, init_y, color);*/
}
#endif

/////////////////////////////////
// PhysicalRectangle

PhysicalRectangle::PhysicalRectangle( double width, double height) : PhysicalShape()
{
  m_width = width;
  m_height = height;
}










void PhysicalRectangle::Generate()
{
 /* m_point_list.clear();
  AddPoint(Point2d(0,0));
  AddPoint(Point2d(m_width,0));
  AddPoint(Point2d(m_width,m_height));
  AddPoint(Point2d(0,m_height));

  PhysicalPolygon::Generate();*/
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

double PhysicalCircle::Area() const
{
 // return b2_pi * m_radius * m_radius;
    return 0;
}

void PhysicalCircle::Generate()
{
/*  if (m_shape) {
    m_body->DestroyShape(m_shape);
    m_shape = NULL;
  }

  if(m_radius > 0.01)
  {
  b2CircleDef shapeDef;
  shapeDef.radius = m_radius;
  shapeDef.localPosition.Set(m_position.x, m_position.y);
  shapeDef.density = m_density;
  shapeDef.friction = m_friction;
  shapeDef.restitution = m_rebound_factor;
  shapeDef.filter.categoryBits = m_filter.categoryBits;
  shapeDef.filter.maskBits = m_filter.maskBits;
  shapeDef.filter.groupIndex = m_filter.groupIndex;
  m_shape = m_body->CreateShape(&shapeDef);
  }*/
}

double PhysicalCircle::GetCurrentMinX() const
{
 // return m_body->GetPosition().x + GetInitialMinX();
    return 0;
}

double PhysicalCircle::GetCurrentMaxX() const
{
  //return m_body->GetPosition().x + GetInitialMaxX();
    return 0;
}

double PhysicalCircle::GetCurrentMinY() const
{
 // return m_body->GetPosition().y + GetInitialMinY();
    return 0;
}

double PhysicalCircle::GetCurrentMaxY() const
{
  //return m_body->GetPosition().y + GetInitialMaxY();
    return 0;
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
void PhysicalCircle::DrawBorder(const Color& /*color*/) const
{
/*  int radius = m_radius * PIXEL_PER_METER;
b2Vec2 center = m_body->GetWorldPoint(b2Vec2(m_position.x, m_position.y));
  int x = lround(( center.x)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x;
  int y = lround(( center.y)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y;

  GetMainWindow().CircleColor(x, y, radius, color);*/
}
#endif
