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
#include "physic/physical_obj.h"
#include "tool/xml_document.h"
#include "game/game_mode.h"


PhysicalObj::PhysicalObj()
{
    
}
PhysicalObj::~PhysicalObj()
{

}

void PhysicalObj::LoadShapeFromXml(const xmlNode &/*xml_shapes*/)
{
 // xmlNodeArray::const_iterator shape_it;
/*
  for (shape_it = shapes.begin(); shape_it != shapes.end(); shape_it++) {
    PhysicalShape* shape = PhysicalShape::LoadFromXml(*shape_it);
    ASSERT(shape);

    b2FilterData filter_data;
    filter_data.categoryBits = 0x0001;
    filter_data.maskBits = 0x0000;
    filter_data.maskBits = 0;
    shape->SetFilter(filter_data);

    m_shapes.push_back(shape);
  }
*/
 /* if (m_shapes.empty()) {
    double mass = 1.0;
    XmlReader::ReadDouble(elem, "mass", mass);
    SetBasicShape(Point2i(1,1), mass);
    return;
  } else {
    double mass;
    bool r = XmlReader::ReadDouble(elem, "mass", mass);
    if (r)
      Error(Format("Body (%s) mass defition is forbidden when you define shape(s)", m_name.c_str()));

    ASSERT(!r);
  }*/

  Generate();
}

void PhysicalObj::Activate()
{
  /*if (m_body)
    return;

  m_body = PhysicalEngine::GetInstance()->AddObject(this);

  if(m_wind_factor != 0)
  {
    PhysicalEngine::GetInstance()->AddWindObject(this);
  }

  if(m_gravity_factor != 1)
  {
    PhysicalEngine::GetInstance()->AddModifiedGravityObject(this);
  }

  SetSpeedXY(m_initial_speed);
  Generate();*/
}

void PhysicalObj::Generate()
{
 /* if (!m_body)
    return;

  m_body->SetBullet(m_is_bullet);

  std::list<PhysicalShape*>::iterator it;
  for (it = m_shapes.begin(); it != m_shapes.end(); it++) {
    (*it)->SetBody(m_body);
    if (m_rebounding)
      (*it)->SetReboundFactor(m_rebound_factor);
    else
      (*it)->SetReboundFactor(0.0);
    (*it)->Generate();
  }
  GenerateMass();
*/
}

void PhysicalObj::Desactivate()
{
 /* if (!m_body)
    return;

  PhysicalEngine::GetInstance()->RemoveModifiedGravityObject(this);
  PhysicalEngine::GetInstance()->RemoveWindObject(this);
  RemoveAllExternForce();
  PhysicalEngine::GetInstance()->RemoveObject(this);
  m_body = NULL;
*/
}

/*
void GameObj::SetBasicShape(const Point2i &newSize, double mass)
{
  double phys_width = double(newSize.x)/PIXEL_PER_METER;
  double phys_height = double(newSize.y)/PIXEL_PER_METER;

  // Shape position is relative to body
  PhysicalPolygon *shape = new PhysicalPolygon();

  shape->AddPoint(Point2d(0 , 0));
  shape->AddPoint(Point2d(phys_width, 0));
  shape->AddPoint(Point2d(phys_width, phys_height));
  shape->AddPoint(Point2d(0 , phys_height));
  shape->SetMass(mass);

  b2FilterData filter_data;
  filter_data.categoryBits = 0x0001;
  filter_data.maskBits = 0x0000;
  filter_data.maskBits = 0;
  if (m_shapes.size() > 0) {
    filter_data = GetCollisionFilter();
  }
  shape->SetFilter(filter_data);

  ClearShapes();

  m_shapes.push_back(shape);

  Generate();
}


void GameObj::SetSphericalShape(int newSize, double mass)
{
  double phys_radius = double(newSize)/(PIXEL_PER_METER*2);

  // Shape position is relative to body
  PhysicalCircle *shape = new PhysicalCircle();

  shape->SetRadius(phys_radius);
  shape->SetMass(mass);

  ClearShapes();
  m_shapes.push_back(shape);

  Generate();
}*/
