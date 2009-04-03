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

#include <iostream>
#include <WORMUX_debug.h>
#include <WORMUX_point.h>
#include <WORMUX_rectangle.h>

#include "character/character.h"
#include "include/action.h"
#include "game/config.h"
#include "game/game_mode.h"
#include "game/time.h"
#include "map/map.h"
#include "network/randomsync.h"
#include "physic/force.h"
#include "physic/physical_engine.h"
#include "physic/physical_obj.h"
#include "physic/physical_shape.h"
#include "object/objects_list.h"
#include "sound/jukebox.h"
#include "team/macro.h"
#include "team/team.h"
#include "team/teams_list.h"
#include "tool/isnan.h"
#include "tool/math_tools.h"
#include "tool/random.h"
#include "weapon/weapon_launcher.h"

#ifdef DEBUG
#include "graphic/color.h"
#endif

const int Y_OBJET_MIN = -10000;
const int WATER_RESIST_FACTOR = 40;

double MeterDistance (const Point2i &p1, const Point2i &p2)
{
  return p1.Distance(p2) / PIXEL_PER_METER;
}

PhysicalObj::PhysicalObj (const std::string &name, const std::string &xml_config) :
  m_collides_with_ground(true),
  m_collides_with_characters(false),
  m_collides_with_objects(false),
  m_rebounding(true),
  m_overlapping_object(NULL),
  m_minimum_overlapse_time(0),
  m_nbr_contact(0),
  m_cfg(),
  m_extern_force_index(1),
  m_ignore_movements(false),
  m_is_character(false),
  m_is_fire(false),
  m_is_bullet(false),
  m_body(NULL),
  m_last_move(Time::GetInstance()->Read()),
  m_name(name),
  m_rebound_sound(""),
  m_initial_speed(0,0),
  m_fixed(false),
  m_alive(ALIVE),
  m_energy(-1),
  m_allow_negative_y(false)
{
  m_cfg = Config::GetInstance()->GetObjectConfig(m_name, xml_config);
  ResetConstants();       // Set physics constants from the xml file

  m_body_def = new b2BodyDef();
  m_body_def->allowSleep = true;



  m_body_def->linearDamping = m_air_resist_factor;
  m_body_def->angularDamping = 0.1;
  m_body_def->position.Set(0.0f, 0.0f);
  m_body_def->fixedRotation = !m_rotating;

  InitShape(xml_config);
  if(m_auto_align_force >0)
  {
      PhysicalEngine::GetInstance()->AddAutoAlignObject(this);
  }

  MSG_DEBUG("physical.mem", "Construction of %s", m_name.c_str());
}

PhysicalObj::~PhysicalObj ()
{

  MSG_DEBUG("physical.mem", "Destruction of %s", m_name.c_str());
  //ClearShapes();
  Desactivate();
  if(m_auto_align_force > 0)
  {
    PhysicalEngine::GetInstance()->RemoveAutoAlignObject(this);
  }
}

void PhysicalObj::InitShape(const std::string &xml_config)
{
  // Loading shape from file
  const xmlNode *elem = NULL;
  XmlReader doc;

  if (xml_config == "") {
    const XmlReader* ddoc = GameMode::GetInstance()->GetXmlObjects();
    elem = XmlReader::GetMarker(ddoc->GetRoot(), m_name);
  } else {
    ASSERT(doc.Load(xml_config));
    elem = XmlReader::GetMarker(doc.GetRoot(), m_name);
  }

  ASSERT(elem != NULL);

  xmlNodeArray shapes = XmlReader::GetNamedChildren(elem, "shape");
  xmlNodeArray::const_iterator shape_it;

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

  if (m_shapes.empty()) {
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
  }

  Generate();
}

//-----------------------------------------------------------------------------
void PhysicalObj::Activate()
{
  if (m_body)
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
  Generate();
}

void PhysicalObj::Generate()
{
  if (!m_body)
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

}

void PhysicalObj::Desactivate()
{
  if (!m_body)
    return;

  PhysicalEngine::GetInstance()->RemoveModifiedGravityObject(this);
  PhysicalEngine::GetInstance()->RemoveWindObject(this);
  RemoveAllExternForce();
  PhysicalEngine::GetInstance()->RemoveObject(this);
  m_body = NULL;

}

void PhysicalObj::GenerateMass()
{
  if (m_fixed) {
    b2MassData massData;
    m_body->SetMass(&massData);
  } else {
    m_body->SetMassFromShapes();
  }
}

//---------------------------------------------------------------------------//
//--                         Class Parameters SET/GET                      --//
//---------------------------------------------------------------------------//

void PhysicalObj::SetXY(const Point2i &position)
{
  SetXY(Point2d(double(position.x), double(position.y)));
}

void PhysicalObj::SetXY(const Point2d &position)
{
  if (m_body) {

    CheckOverlapping();

    if (IsOutsideWorldXY(Point2i(int(position.x), int(position.y)))) {

      SetPhysXY( position / PIXEL_PER_METER );
      Ghost();
      SignalOutOfMap();

    } else {
      SetPhysXY( position / PIXEL_PER_METER );

      if (!IsColliding()) {
        StartMoving();
      }
    }
  } else {
    SetPhysXY( position / PIXEL_PER_METER );
  }
}

double PhysicalObj::GetXdouble() const
{
  return round(GetPhysX() * PIXEL_PER_METER);
}

double PhysicalObj::GetYdouble() const
{
  return round(GetPhysY() * PIXEL_PER_METER);
}

int PhysicalObj::GetX() const
{
  return (int)(GetXdouble()+0.5f);//Round
}

int PhysicalObj::GetY() const
{
  return (int)(GetYdouble()+0.5f);//Round
}

double PhysicalObj::GetPhysX() const
{
  if (m_body) {
    return m_body->GetPosition().x;
  } else {
    return m_body_def->position.x;
  }
}

double PhysicalObj::GetPhysY() const
{
  if (m_body) {
    return m_body->GetPosition().y;
  } else {
    return m_body_def->position.y;
  }
}

Point2d PhysicalObj::GetPhysXY() const
{
  if (m_body) {
    return Point2d( m_body->GetPosition().x, m_body->GetPosition().y);
  } else {
    return Point2d(0.0,0.0);
  }
}

void PhysicalObj::SetPhysXY(double x, double y)
{
  if (m_body) {
  /* if (m_pos_x.x0 != x || m_pos_y.x0 != y) {
     m_pos_x.x0 = x;
     m_pos_y.x0 = y;*/
    m_body->SetXForm(b2Vec2(x,y),m_body->GetAngle());
    PhysicalEngine::GetInstance()->StaticStep();
    /*UpdateTimeOfLastMove();
      }*/
  } else {
    m_body_def->position.Set(x,y);
  }
}

void PhysicalObj::SetPhysXY(const Point2d &position)
{
  SetPhysXY(position.x, position.y);
}


void PhysicalObj::SetSpeedXY (Point2d vector)
{
  if (!m_body) {
    m_initial_speed = vector;
    return;
  }

  if (EqualsZero(vector.x))
    vector.x = 0;
  if (EqualsZero(vector.y))
    vector.y = 0;

  bool was_moving = IsMoving();

  // setting to FreeFall is done in StartMoving()
  m_body->SetLinearVelocity(b2Vec2(vector.x,vector.y));
  if (!was_moving && IsMoving()) {
    UpdateTimeOfLastMove();
    StartMoving();
    m_body->WakeUp();
  }
}

void PhysicalObj::SetSpeed (double norm, double angle)
{
  SetSpeedXY(Point2d::FromPolarCoordinates(norm, angle));
}

void PhysicalObj::AddSpeedXY (Point2d vector)
{
  if (EqualsZero(vector.x))
    vector.x = 0;
  if (EqualsZero(vector.y))
    vector.y = 0;

  bool was_moving = IsMoving();

  m_body->SetLinearVelocity(m_body->GetLinearVelocity()+b2Vec2(vector.x,vector.y));
  // setting to FreeFall is done in StartMoving()

  if (!was_moving && IsMoving()) {
    UpdateTimeOfLastMove();
    StartMoving();
    m_body->WakeUp();
  }
}

void PhysicalObj::AddSpeed(double norm, double angle)
{
  AddSpeedXY(Point2d::FromPolarCoordinates(norm, angle));
}

void PhysicalObj::GetSpeed(double &norm, double &angle) const
{
  Point2d speed ;

  speed = GetSpeedXY();
  norm = speed.Norm();
  angle = speed.ComputeAngle();
}

Point2d PhysicalObj::GetSpeedXY () const
{
  if (!m_body || !IsMoving())
    return Point2d(0.0,0.0);

  return Point2d(m_body->GetLinearVelocity().x, m_body->GetLinearVelocity().y);
}

Point2d PhysicalObj::GetSpeed() const
{
  return GetSpeedXY();
}

double PhysicalObj::GetAngularSpeed() const
{
  if (!m_body)
    return 0;

  return m_body->GetAngularVelocity();
}

double PhysicalObj::GetSpeedAngle() const
{
  return GetSpeedXY().ComputeAngle();
}

// TODO: REMOVE IT IN NEAR FUTURE
double PhysicalObj::GetInitialMass() const
{
  double mass = 0.0;
  for (std::list<PhysicalShape*>::const_iterator it = m_shapes.begin();
       it != m_shapes.end();
       it++) {
    mass += (*it)->GetMass();
  }

  return mass;
}

double PhysicalObj::GetMass() const
{
  ASSERT(m_body);

  return m_body->GetMass();
}

void PhysicalObj::SetBasicShape(const Point2i &newSize, double mass)
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

double PhysicalObj::GetWdouble() const
{
  ASSERT(m_shapes.size() != 0);

  std::list<PhysicalShape*>::const_iterator it = m_shapes.begin();
  double phys_min_x;
  double phys_max_x;
  double tmp_x;

  if (m_body) {
    phys_min_x = (*it)->GetCurrentMinX();
    phys_max_x = (*it)->GetCurrentMaxX();
  } else {
    phys_min_x = (*it)->GetInitialMinX();
    phys_max_x = (*it)->GetInitialMaxX();
  }

  for (it++; it != m_shapes.end(); it++) {

    if (m_body)
      tmp_x = (*it)->GetCurrentMinX();
    else
      tmp_x = (*it)->GetInitialMinX();

    if (tmp_x < phys_min_x)
      phys_min_x = tmp_x;

    if (m_body)
      tmp_x = (*it)->GetCurrentMaxX();
    else
      tmp_x = (*it)->GetInitialMaxX();

    if (tmp_x > phys_max_x)
      phys_max_x = tmp_x;
  }
  double phys_width = phys_max_x - phys_min_x;

  double pixel_width = phys_width * PIXEL_PER_METER;
  return pixel_width;
}

int PhysicalObj::GetWidth() const
{
  return int(GetWdouble()+0.5f);//Round
}

double PhysicalObj::GetHdouble() const
{
  ASSERT(m_shapes.size() != 0);

  std::list<PhysicalShape*>::const_iterator it = m_shapes.begin();
  double phys_min_y;
  double phys_max_y;
  double tmp_y;

  if (m_body) {
    phys_min_y = (*it)->GetCurrentMinY();
    phys_max_y = (*it)->GetCurrentMaxY();
  } else {
    phys_min_y = (*it)->GetInitialMinY();
    phys_max_y = (*it)->GetInitialMaxY();
  }

  for (it++; it != m_shapes.end(); it++) {

    if (m_body)
      tmp_y = (*it)->GetCurrentMinY();
    else
      tmp_y = (*it)->GetInitialMinY();

    if (tmp_y < phys_min_y)
      phys_min_y = tmp_y;

    if (m_body)
      tmp_y = (*it)->GetCurrentMaxY();
    else
      tmp_y = (*it)->GetInitialMaxY();

    if (tmp_y > phys_max_y)
      phys_max_y = tmp_y;
  }
  double phys_height = phys_max_y - phys_min_y;

  double pixel_height = phys_height * PIXEL_PER_METER;
  return pixel_height;
}

int PhysicalObj::GetHeight() const
{
  return int(GetHdouble()+0.5f);//Round
}

Point2d PhysicalObj::GetSizeDouble() const
{
  return Point2d(GetWdouble(), GetHdouble());
}

Point2i PhysicalObj::GetSize() const
{
  return Point2i(GetWidth(), GetHeight());
}

const Rectanglei PhysicalObj::GetTestRect() const
{
  int width = GetWidth();
  int height = GetHeight();
  if (width < 1)
    width = 1;
  if (height < 1)
    height = 1;

  return Rectanglei(GetMinX(), GetMinY(), width, height);
}

int PhysicalObj::GetMinX() const
{
  std::list<PhysicalShape*>::const_iterator it = m_shapes.begin();
  double shape_pos_x;
  double tmp_x;

  if (m_body)
    shape_pos_x = (*it)->GetCurrentMinX();
  else
    shape_pos_x = (*it)->GetInitialMinX();

  for (it++; it != m_shapes.end(); it++) {

    if (m_body)
      tmp_x = (*it)->GetCurrentMinX();
    else
      tmp_x = (*it)->GetInitialMinX();

    if (tmp_x < shape_pos_x)
      shape_pos_x = tmp_x;
  }

  if (!m_body)
    return int((GetPhysX() + shape_pos_x) * PIXEL_PER_METER);

  return int(shape_pos_x * PIXEL_PER_METER);
}

int PhysicalObj::GetMinY() const
{
  std::list<PhysicalShape*>::const_iterator it = m_shapes.begin();
  double shape_pos_y;
  double tmp_y;

  if (m_body)
    shape_pos_y = (*it)->GetCurrentMinY();
  else
    shape_pos_y = (*it)->GetInitialMinY();

  for (it++; it != m_shapes.end(); it++) {

    if (m_body)
      tmp_y = (*it)->GetCurrentMinY();
    else
      tmp_y = (*it)->GetInitialMinY();

    if (tmp_y < shape_pos_y)
      shape_pos_y = tmp_y;
  }

  if (!m_body)
    return int((GetPhysY() + shape_pos_y) * PIXEL_PER_METER);

  return int(shape_pos_y * PIXEL_PER_METER);
}

void PhysicalObj::SetAirResistFactor(double factor)
{
  if (m_air_resist_factor == factor)
    return;

  m_air_resist_factor = factor;
  m_body_def->linearDamping = m_air_resist_factor;
  if (m_body)
    printf("%s PhysicalObj::SetAirResistFactor(%f)\n", m_name.c_str(), factor);
}

void PhysicalObj::SetGravityFactor (double factor) {
  m_gravity_factor = factor;
  if(m_body && m_gravity_factor != 1){
    PhysicalEngine::GetInstance()->AddModifiedGravityObject(this);
  }
}

void PhysicalObj::StoreValue(Action *a)
{
  // Position
  a->Push(GetPhysXY());
  a->Push(GetAngle());

  // Speed
  double norm, angle;
  GetSpeed(norm, angle);
  a->Push(norm);
  a->Push(angle);

  // Collision information
  a->Push(m_collides_with_ground);
  a->Push(m_collides_with_characters);
  a->Push(m_collides_with_objects);
  a->Push((int)m_minimum_overlapse_time);

  // other information (mostly about rope)
  //  a->Push((int)m_motion_type);
  //a->Push(m_extern_force);
  a->Push((int)m_last_move);
  //  a->Push(m_fix_point_gnd);
  //  a->Push(m_fix_point_dxy);
  //  a->Push(m_rope_angle);
  //  a->Push(m_rope_length);
  //  a->Push(m_rope_elasticity);
  //  a->Push(m_elasticity_damping);
  //  a->Push(m_balancing_damping);
  //  a->Push(m_elasticity_off);

  MSG_DEBUG("physic.sync", "%s now - position x:%f, y:%f - speed x:%f, y:%f, angle:%f",
	    typeid(*this).name(), GetPhysXY().x, GetPhysXY().y, GetSpeed().x, GetSpeed().y, GetAngularSpeed());

  a->Push(m_ignore_movements);
  a->Push(m_is_character);
  a->Push((int)m_alive);
  a->Push(m_energy);
  a->Push(m_allow_negative_y);
}

void PhysicalObj::GetValueFromAction(Action *a)
{
  // Position
  Point2d position = a->PopPoint2d();
  SetPhysXY(position);
  double angle = a->PopDouble();
  SetAngle(angle);

  // Speed
  double norm = a->PopDouble();
  double speed_angle = a->PopDouble();
  SetSpeed(norm, speed_angle);

  // Collision information
  bool collides_with_ground, collides_with_characters, collides_with_objects;
  collides_with_ground     = !!a->PopInt();
  collides_with_characters = !!a->PopInt();
  collides_with_objects    = !!a->PopInt();
  SetCollisionModel(collides_with_ground, collides_with_characters, collides_with_objects);
  m_minimum_overlapse_time   = (uint)a->PopInt();

  // other information (mostly about rope)

  //  m_motion_type        = (MotionType_t)a->PopInt();
  // m_extern_force       = a->PopPoint2d();
  m_last_move          = (uint)a->PopInt();
  //  m_fix_point_gnd      = a->PopPoint2d();
  //  m_fix_point_dxy      = a->PopPoint2d();
  //  m_rope_angle         = a->PopEulerVector();
  //  m_rope_length        = a->PopEulerVector();
  //  m_rope_elasticity    = a->PopDouble();
  //  m_elasticity_damping = a->PopDouble();
  //  m_balancing_damping  = a->PopDouble();
  //  m_elasticity_off     = !!a->PopInt();

  MSG_DEBUG("physic.sync", "%s now - position x:%f, y:%f - speed x:%f, y:%f, angle:%f",
	    typeid(*this).name(),  GetPhysXY().x, GetPhysXY().y, GetSpeed().x, GetSpeed().y, GetAngularSpeed());

  m_ignore_movements         = !!a->PopInt();
  m_is_character             = !!a->PopInt();
  m_alive                    = (alive_t)a->PopInt();
  m_energy                   = a->PopInt();
  m_allow_negative_y         = !!a->PopInt();
}

void PhysicalObj::UpdateTimeOfLastMove()
{
  m_last_move = Time::GetInstance()->Read();
}


void PhysicalObj::SetBullet(bool is_bullet)
{
  m_is_bullet = is_bullet;
  Generate();
}

void PhysicalObj::AddShape(PhysicalShape *shape)
{
  m_shapes.push_back(shape);
}

void PhysicalObj::ClearShapes()
{
  std::list<PhysicalShape*>::iterator it;
  for (it = m_shapes.begin(); it != m_shapes.end(); it++) {
    delete (*it);
  }
  m_shapes.clear();
}

const b2FilterData& PhysicalObj::GetCollisionFilter() const
{
  ASSERT(m_shapes.size() > 0);

  return (m_shapes.front()->GetFilter());
}

void PhysicalObj::SetCollisionFilter(const b2FilterData& filter)
{
  std::list<PhysicalShape*>::iterator it;
  for (it = m_shapes.begin(); it != m_shapes.end(); it++) {
    (*it)->SetFilter(filter);
  }
  Generate();
}

void PhysicalObj::SetOverlappingObject(PhysicalObj* obj, int timeout)
{
  ASSERT(obj != NULL);

  m_overlapping_object = obj;
  ObjectsList::GetRef().AddOverlappedObject(this);
  MSG_DEBUG("physic.overlapping", "\"%s\" doesn't check any collision with \"%s\" anymore during %d ms",
	    GetName().c_str(), obj->GetName().c_str(), timeout);

  m_minimum_overlapse_time = 0;
  if (timeout > 0)
    m_minimum_overlapse_time = Time::GetInstance()->Read() + timeout;

  b2FilterData filter = GetCollisionFilter();
  filter.groupIndex = -1;
  SetCollisionFilter(filter);

  b2FilterData filter2 = obj->GetCollisionFilter();
  filter2.groupIndex = -1;
  obj->SetCollisionFilter(filter2);

  CheckOverlapping();
}

void PhysicalObj::ClearOverlappingObject()
{
  m_minimum_overlapse_time = 0;

  b2FilterData data = GetCollisionFilter();
  data.groupIndex = 0;
  SetCollisionFilter(data);

  if (m_overlapping_object != NULL) {

    b2FilterData data_obj = m_overlapping_object->GetCollisionFilter();
    data_obj.groupIndex = 0;
    m_overlapping_object->SetCollisionFilter(data_obj);

    m_overlapping_object = NULL;
    ObjectsList::GetRef().RemoveOverlappedObject(this);
    MSG_DEBUG( "physic.overlapping", "clearing overlapping object in \"%s\"", GetName().c_str());
  }
}

const PhysicalObj* PhysicalObj::GetOverlappingObject() const
{
  return m_overlapping_object;
}


bool PhysicalObj::IsOverlapping(const PhysicalObj* obj) const
{
  return m_overlapping_object == obj;
}

void PhysicalObj::CheckOverlapping()
{
  if (!m_overlapping_object)
    return;

  if (!m_body)
    return;

  // Check if we are still overlapping with this object
  if (!m_overlapping_object->GetTestRect().Intersect( GetTestRect() ) &&
      m_minimum_overlapse_time <= Time::GetInstance()->Read())
    {
      MSG_DEBUG("physic.overlapping", "\"%s\" just stopped overlapping with \"%s\" (%d ms left)",
		GetName().c_str(), m_overlapping_object->GetName().c_str(),
		(m_minimum_overlapse_time - Time::GetInstance()->Read()));
      ClearOverlappingObject();
    }
  else
    {
      MSG_DEBUG("physic.overlapping", "\"%s\" is overlapping with \"%s\"",
		GetName().c_str(), m_overlapping_object->GetName().c_str());
    }
}

void PhysicalObj::SetRebounding(bool rebounding)
{
  if (m_rebounding == rebounding)
    return;

  m_rebounding = rebounding;
  Generate();
}

uint PhysicalObj::AddExternForce (double norm, double angle)
{
  return AddExternForceXY(Point2d::FromPolarCoordinates(norm, angle));
}

void PhysicalObj::RemoveExternForce(uint index)
{
  if (index == 0)
    return;

  if (m_extern_force_map.count(index)) {
    PhysicalEngine::GetInstance()->RemoveForce(m_extern_force_map[index]);
    delete m_extern_force_map[index];
    m_extern_force_map.erase(index);
  }
}

uint PhysicalObj::AddExternForceXY (const Point2d& vector)
{
  if (!m_body)
    return 0;

  m_extern_force_map[m_extern_force_index] =  new Force(this, GetPhysXY(), vector, false) ;
  PhysicalEngine::GetInstance()->AddForce(m_extern_force_map[m_extern_force_index] );
  m_extern_force_index++;

  UpdateTimeOfLastMove();
  MSG_DEBUG ("physic.physic", "EXTERN FORCE %s.", typeid(*this).name());


  return m_extern_force_index-1;
}


void PhysicalObj::RemoveAllExternForce()
{
  std::map<uint,Force *>::iterator it;
  for( it = m_extern_force_map.begin() ; it != m_extern_force_map.end() ; it++ ) {
    RemoveExternForce(it->first);
  }
}

void PhysicalObj::ComputeAutoAlign()
{
  if(m_body){
    double delta = GetAngle() +GetSpeedAngle();
    while(delta >= 2 * M_PI)
    delta -= 2 * M_PI;
    while(delta < 0.0)
    delta += 2 * M_PI;

    double response = 0.0;
    if ( delta < M_PI/2 ){
        response = 1 - sin(delta + M_PI/2);
    } else if ( delta < M_PI ){
        response = 1 - sin(delta - M_PI/2);
    } else if ( delta < 3*M_PI/2 ){
        response = -1 + sin(delta - M_PI/2);
    } else {
        response = -1 + sin(delta + M_PI/2);
    }

    //double delta = sin (GetAngle() +GetSpeedAngle());

    if(abs(delta) <0.1)
    {
      //Fast stabilization
      m_body->SetAngularVelocity(m_body->GetAngularVelocity()/1.1);
    }else
    {
      b2Vec2 velocity = m_body->GetLinearVelocity();
      m_body->ApplyTorque( response  * m_auto_align_force * (velocity.x * velocity.x  + velocity.y * velocity.y));
    }

  }
}

bool PhysicalObj::IsSleeping() const
{
  //TODO Correct this because you must check that the only force is wind and that the speed is stable
  return (GetSpeed().x < 0.001 && GetSpeed().y < 0.001);
}

void PhysicalObj::SetEnergyDelta(int delta, bool /*do_report*/)
{
  if (m_energy == -1)
    return;

  m_energy += delta;
  if (m_energy <= 0 && !IsGhost()) {
    Ghost();
    m_energy = -1;
  }
}

void PhysicalObj::UpdatePosition ()
{
  if ( IsOutsideWorldXY( Point2i( GetX(),GetY()) )) {
    Ghost();
    SignalOutOfMap();
  }

  // No ghost allowed here !
  if (IsGhost())
    return;

  if (m_collides_with_ground) {

    // object is not moving and has no reason to move
    if ( !IsMoving() && IsColliding() && !IsInWater() ) return;

    // object is not moving BUT it should fall !
    if ( !IsMoving() && IsColliding() ) StartMoving();
  }

  if (IsGhost())
    return;

  if (IsInWater()) {
    SetCollisionModel(false,false,false);
  }

  // Classical object sometimes sinks in water and sometimes goes out of water!
  if (m_collides_with_ground) {

    if ( IsInWater() && m_alive != DROWNED && m_alive != DEAD)
      Drown();
    else if ( !IsInWater() && m_alive == DROWNED )
      GoOutOfWater();
  }
}

bool PhysicalObj::PutOutOfGround(double direction, double max_distance)
{
  if (IsOutsideWorld(Point2i(0, 0)))
    return false;

  if (IsInVacuum(Point2i(0, 0), false))
    return true;

  double dx = cos(direction);
  double dy = sin(direction);
  // (dx,dy) is a normal vector (cos^2+sin^2==1)

  double step = 1.0;
  while (step < max_distance && !IsInVacuum(Point2i((int)(dx * step),(int)(dy * step)), false ))
    step+=1.0;

  if (step < max_distance)
    SetXY( Point2i((int)(dx * step)+GetX(),(int)(dy * step)+GetY()) );
  else
    return false; //Can't put the object out of the ground

  return true;
}

bool PhysicalObj::PutOutOfGround()
{
  if (IsOutsideWorld(Point2i(0, 0)))
    return false;

  if (IsInVacuum(Point2i(0, 0)))
    return true;

  bool left,right,top,bottom;
  left   = GetWorld().IsInVacuum_left(*this, 0, 0);
  right  = GetWorld().IsInVacuum_right(*this, 0, 0);
  top    = GetWorld().IsInVacuum_top(*this, 0, 0);
  bottom = GetWorld().IsInVacuum_bottom(*this, 0, 0);

  int dx = (int)GetTestRect().GetSizeX() * (right-left);
  int dy = (int)GetTestRect().GetSizeY() * (top-bottom);

  if (dx == 0 && dy == 0)
    return false; //->Don't know in which direction we should go...

  Point2i b(dx, dy);

  double dir = b.ComputeAngle();
  return PutOutOfGround(dir);
}

void PhysicalObj::Init()
{
  Activate();
  if (m_alive != ALIVE)
    MSG_DEBUG( "physic.state", "%s - Init.", m_name.c_str());

  m_alive = ALIVE;
  ClearOverlappingObject();
  StopMoving();
}

void PhysicalObj::Ghost ()
{
  if (m_alive == GHOST)
    return;

  bool was_dead = IsDead();
  m_alive = GHOST;
  MSG_DEBUG("physic.state", "%s - Ghost, was_dead = %d", m_name.c_str(), was_dead);
  SignalGhostState(m_energy==0);
  // The object became a gost
  StopMoving();
}

void PhysicalObj::Drown()
{
  ASSERT (m_alive != DROWNED);
  MSG_DEBUG("physic.state", "%s - Drowned...", m_name.c_str());
  m_alive = DROWNED;

  // Set the air grab to water resist factor.
  SetAirResistFactor(WATER_RESIST_FACTOR * GetAirResistFactor());

  // Ensure the gravity factor is upper than 0.0
  if (EqualsZero(GetGravityFactor()))
    SetGravityFactor(0.1);

  if (m_is_fire) {
    // If fire, do smoke...
    GetWorld().water.Smoke(GetPosition());
  } else if (GetMass() >= 2) {
    // make a splash in the water :-)
    GetWorld().water.Splash(GetPosition());
  }

  StopMoving();
  StartMoving();
  SignalDrowning();
}

void PhysicalObj::GoOutOfWater()
{
  ASSERT (m_alive == DROWNED);
  MSG_DEBUG("physic.state", "%s - Go out of water!...", m_name.c_str());
  m_alive = ALIVE;

  // Set the air grab to normal air resist factor.
  SetAirResistFactor(m_cfg.m_air_resist_factor);
  SetGravityFactor(m_cfg.m_gravity_factor);
  StartMoving();
  SignalGoingOutOfWater();
}

void PhysicalObj::SignalRebound()
{
  // TO CLEAN...:
  if (!m_rebound_sound.empty())
    JukeBox::GetInstance()->Play("default", m_rebound_sound) ;
}

void PhysicalObj::SetCollisionGroup(int group)
{
  b2FilterData data = GetCollisionFilter();
  data.groupIndex = group;
  SetCollisionFilter(data);
}

void PhysicalObj::SetCollisionCategory(int category)
{
  b2FilterData data = GetCollisionFilter();
  data.categoryBits = category;
  SetCollisionFilter(data);
}


void PhysicalObj::SetCollisionModel(bool collides_with_ground,
                                    bool collides_with_characters,
                                    bool collides_with_objects)
{
  // SetSize() must be called before

  m_collides_with_ground = collides_with_ground;
  m_collides_with_characters = collides_with_characters;
  m_collides_with_objects = collides_with_objects;

  b2FilterData data = GetCollisionFilter();
  data.maskBits = 0x0000;

  if (m_collides_with_objects) {
    data.maskBits |= 0x0001;
  }

  if (m_collides_with_characters) {
    data.maskBits |= 0x0002;
  }

  if (m_collides_with_ground) {
    data.maskBits |= 0x0004;
  }

  SetCollisionFilter(data);

  // Check boolean values
#ifdef DEBUG
  if (m_collides_with_characters || m_collides_with_objects)
    ASSERT(m_collides_with_ground);

  if (!m_collides_with_ground) {
    ASSERT(!m_collides_with_characters);
    ASSERT(!m_collides_with_objects);
  }
#endif
}

bool PhysicalObj::IsOutsideWorldXY(const Point2i& position) const
{
  // to take object rotation into account
  int x = GetMinX() - GetX() + position.x;
  int y = GetMinY() - GetY() + position.y;

  if (GetWorld().IsOutsideWorldXwidth(x, GetWidth()))
    return true;

  if (GetWorld().IsOutsideWorldYheight(y, GetHeight())) {
    if (m_allow_negative_y &&
	Y_OBJET_MIN <= y &&
	y + GetHeight() - 1 < 0 )
	return false;

    return true;
  }
  return false;
}

bool PhysicalObj::IsOutsideWorld(const Point2i &offset) const
{
  return IsOutsideWorldXY( GetPosition() + offset );
}

bool PhysicalObj::IsInVacuumXY(const Point2i &position, bool check_object) const
{
  if (IsOutsideWorldXY(position))
    return GetWorld().IsOpen();

  if (check_object && CollidedObjectXY(position))
    return false;

  // to take object rotation into account
  Rectanglei rect(GetMinX() - GetX() + position.x, GetMinY() - GetY() + position.y,
		  GetWidth(), GetHeight());

  return GetWorld().RectIsInVacuum(rect);
}

bool PhysicalObj::IsInVacuum(const Point2i &offset, bool check_objects) const
{
  return IsInVacuumXY(GetPosition() + offset, check_objects);
}

// ====================================================
// WARNING: To rewrite using Box2D shapes and contacts
// ====================================================
PhysicalObj* PhysicalObj::CollidedObjectXY(const Point2i & position) const
{
  if (IsOutsideWorldXY(position))
    return NULL;

  // to take object rotation into account
  Rectanglei rect(GetMinX() - GetX() + position.x, GetMinY() - GetY() + position.y,
		  GetWidth(), GetHeight());

  if (m_collides_with_characters) {

    FOR_ALL_LIVING_CHARACTERS(team,character) {
        // We check both objet if one overlapse the other
        if ( (PhysicalObj*)(*character) != this &&
	     !IsOverlapping(*character) &&
	     !(*character)->IsOverlapping(this) &&
	     (*character)->GetTestRect().Intersect( rect ))
          return (PhysicalObj*)(*character);
    }
  }

  if (m_collides_with_objects) {
    FOR_EACH_OBJECT(it) {
        PhysicalObj * object=*it;

        // We check both objet if one overlapse the other
        if (object != this
	    && !IsOverlapping(object)
	    && !object->IsOverlapping(this)
            && object->m_collides_with_objects
            && object->GetTestRect().Intersect(rect)
	    && (!m_is_character
		|| object->m_collides_with_characters)
	    )
	    return object;
    }
  }

  return NULL;
}

PhysicalObj* PhysicalObj::CollidedObject(const Point2i & offset) const
{
  return CollidedObjectXY(GetPosition() + offset);
}

bool PhysicalObj::IsInWater () const
{
  ASSERT (!IsGhost());
  if (!GetWorld().water.IsActive())
    return false;

  int x = InRange_Long(GetCenterX(), 0, GetWorld().GetWidth()-1);

  return (int)GetWorld().water.GetHeight(x) < GetCenterY();
}

void PhysicalObj::DirectFall()
{
  while (!IsGhost() && !IsInWater() && !IsColliding()) {
    MSG_DEBUG("physic.fall", "%s - x=%f, y=%f\n", m_name.c_str(), GetXdouble(), GetYdouble());
    SetY(GetYdouble()+1.0);
  }
}

bool PhysicalObj::IsImmobile() const
{
  bool r = IsSleeping()
    || m_ignore_movements
    || (!IsMoving() && IsColliding())
    || IsGhost();

  return r;
}

bool PhysicalObj::IsGhost() const
{
  return (m_alive == GHOST);
}

bool PhysicalObj::IsDrowned() const
{
  return (m_alive == DROWNED);
}

bool PhysicalObj::IsDead() const
{
  bool r = IsGhost()
    || IsDrowned()
    || m_alive == DEAD;

  return r;
}

// Are the two object in contact ? (uses test rectangles)
bool PhysicalObj::Overlapse(const PhysicalObj &b) const
{
  return GetTestRect().Intersect( b.GetTestRect() );
}

// Do the point p touch the object ?
bool PhysicalObj::Contain(const Point2i &p) const
{
  return GetTestRect().Contains( p );
}

bool PhysicalObj::PutRandomly(bool on_top_of_world, double min_dst_with_characters, bool net_sync)
{
  uint bcl=0;
  uint NB_MAX_TRY = 60;
  bool ok;
  Point2i position;

  MSG_DEBUG("physic.position", "%s - Search a position...", m_name.c_str());

  do {
    bcl++;
    ok = true;
    Init();

    if (bcl >= NB_MAX_TRY) {
      MSG_DEBUG("physic.position", "%s - Impossible to find an initial position !!", m_name.c_str());
      return false;
    }

    if (on_top_of_world) {
      // Give a random position for x
      if (net_sync)
        position.x = RandomSync().GetLong(0, GetWorld().GetWidth() - GetWidth());
      else
        position.x = RandomLocal().GetLong(0, GetWorld().GetWidth() - GetWidth());

      position.y = -GetHeight()+1;
    } else {
      if (net_sync)
        position = RandomSync().GetPoint(GetWorld().GetSize() - GetSize() + 1);
      else
        position = RandomLocal().GetPoint(GetWorld().GetSize() - GetSize() + 1);
    }

    SetXY(position);

    MSG_DEBUG("physic.position", "%s (try %u/%u) - Test in %d, %d",
              m_name.c_str(), bcl, NB_MAX_TRY, position.x, position.y);

    // Check physical object is not in the ground
    ok &= !IsGhost() && GetWorld().ParanoiacRectIsInVacuum(GetTestRect())  && IsInVacuum( Point2i(0, 1) );
    if (!ok) {
      MSG_DEBUG("physic.position", "%s - Put it in the ground -> try again !", m_name.c_str());
      continue;
    }

    /* check if the area rigth under the object has a bottom on the ground */
    ok &= !GetWorld().ParanoiacRectIsInVacuum(Rectanglei(GetCenter().x, position.y, 1, GetWorld().GetHeight() -
							 (WATER_INITIAL_HEIGHT + 30) - position.y));
    if (!ok) {
      MSG_DEBUG("physic.position", "%s - Put in outside the map or in water -> try again", m_name.c_str());
      continue;
    }

    DirectFall();

    // Check distance with characters
    FOR_ALL_LIVING_CHARACTERS(team, character) {

      if ((*character) != this) {
	if (min_dst_with_characters == 0) {

	  if(Overlapse(**character)) {
            MSG_DEBUG("physic.position", "%s - Object is too close from character %s", m_name.c_str(), (*character)->m_name.c_str());
            ok = false;
	  }

	} else {
	  Point2i p1 = (*character)->GetCenter();
	  Point2i p2 = GetCenter();
	  double dst = p1.Distance( p2 );

	  // ok this test is not perfect but quite efficient ;-)
	  // else we need to check each distance between each "corner"
	  if (dst < min_dst_with_characters)
	    ok = false;
	}
      }

      if (ok && on_top_of_world)
	SetXY(position);
    }

  } while (!ok);

  MSG_DEBUG("physic.position", "Put '%s' after %u tries", m_name.c_str(), bcl);
  return true;
}

#ifdef DEBUG
#include "map/camera.h"
#include "graphic/colors.h"
#include "graphic/text.h"
#include "graphic/video.h"

void PhysicalObj::DrawPolygon(const Color& color) const
{
  std::list<PhysicalShape*>::const_iterator it;

  for (it = m_shapes.begin(); it != m_shapes.end(); it++) {
    (*it)->DrawBorder(color);
  }

  Rectanglei test_rect = GetTestRect();

  Rectanglei rect(test_rect.GetPosition() - Camera::GetRef().GetPosition(),
		  test_rect.GetSize());

  GetMainWindow().RectangleColor(rect, primary_blue_color);

  if (m_body->GetMass()) {
    std::string txt = Format("%.2f kg", m_body->GetMass());
    Text mass_text(txt, white_color, Font::FONT_TINY, Font::FONT_NORMAL, false);
    mass_text.DrawTopLeft(GetPosition() - Camera::GetRef().GetPosition());
  }
}
#endif

void PhysicalObj::ImpulseXY(const Point2d& vector)
{
  MSG_DEBUG("physic.move", "%s - impulse x=%f  y =%y\n", m_name.c_str(), vector.x, vector.y);

  m_body->ApplyImpulse(b2Vec2(vector.x,vector.y),b2Vec2(GetPhysX(),GetPhysY()));
}

void PhysicalObj::Impulse(double norm, double angle)
{
  ImpulseXY(Point2d::FromPolarCoordinates(norm, angle));
}



b2BodyDef *PhysicalObj::GetBodyDef()
{
  return m_body_def;
}


/*
void PhysicalObj::UnsetPhysFixationPoint()
{
  double speed_norm, angle ;

     GetSpeed (speed_norm, angle);

     angle = -angle ;

     SetSpeed(speed_norm, angle);

     UpdateTimeOfLastMove();

     m_pos_x.x2 = 0 ;
     m_pos_y.x2 = 0 ;

     m_rope_angle.Clear();
     m_rope_length.Clear();

     m_motion_type = FreeFall ;
}
*/

/*void PhysicalObj::ChangePhysRopeSize(double dl)
  {
  if ((dl < 0) && (m_rope_length.x0 < 0.5))
  return ;

  bool was_moving = IsMoving();

  m_rope_length.x0 += dl ;

  // Recompute angular speed depending on the new rope length.
  m_rope_angle.x1 = m_rope_angle.x1 * (m_rope_length.x0 - dl) / m_rope_length.x0 ;

  if (!was_moving && IsMoving())
  StartMoving();
  }*/


//---------------------------------------------------------------------------//
//--                            Physical Simulation                        --//
//---------------------------------------------------------------------------//

void PhysicalObj::StartMoving()
{
  UpdateTimeOfLastMove();

  //   if (m_motion_type == NoMotion)
  //     m_motion_type = FreeFall ;


  MSG_DEBUG ("physic.physic", "Starting to move: %s.", typeid(*this).name());
}

void PhysicalObj::StopMoving()
{

  if (!IsMoving())
    return;

  MSG_DEBUG ("physic.physic", "Stops moving: %s.", typeid(*this).name());

  SetSpeedXY(Point2d(0.0,0.0));

  UpdateTimeOfLastMove();


  /*if (m_motion_type != Pendulum)
    m_motion_type = NoMotion ;

    m_extern_force.Clear();*/
}

bool PhysicalObj::IsMoving() const
{
  /* return !EqualsZero(m_pos_x.x1)  ||
     !EqualsZero(m_pos_y.x1)  ||
     !m_extern_force.IsNull() ||
     m_motion_type != NoMotion;
  */
  bool is_not_moving;

  is_not_moving = m_body->GetLinearVelocity().x == 0.0;
  is_not_moving &= m_body->GetLinearVelocity().y == 0.0;
  is_not_moving &= m_body->GetAngularVelocity() == 0.0;


  return !is_not_moving;
}

bool PhysicalObj::IsFalling() const
{
  return (m_body->GetLinearVelocity().x  > 0.1);
}


void PhysicalObj::ClearContact()
{
  added_contact_list.clear();
  persist_contact_list.clear();
  removed_contact_list.clear();
  result_contact_list.clear();
}

void PhysicalObj::AddContact(const PhysicalShape * /*shape*/)
{
  m_nbr_contact++;
  MSG_DEBUG("physic.contact", "%s - Adding contact %d\n", m_name.c_str(), m_nbr_contact);
}

void PhysicalObj::RemoveContact(const PhysicalShape * /*shape*/)
{
  m_nbr_contact--;
  MSG_DEBUG("physic.contact", "%s - Removing contact %d\n", m_name.c_str(), m_nbr_contact);
}

bool PhysicalObj::IsColliding() const
{
  return (m_nbr_contact !=0);
}

double PhysicalObj::GetAngle() const
{
  return -m_body->GetAngle();
}

void PhysicalObj::SetAngle(double angle)
{
  ASSERT(m_rotating || angle == 0);

  if (m_body) {
    m_body->SetXForm(m_body->GetPosition(), angle);
  } else {
    m_body_def->angle = angle;
  }
}

const Point2i PhysicalObj::GetRotationCenter()
{
  b2Vec2 center = m_body->GetLocalCenter();
  return Point2i(lround(center.x*PIXEL_PER_METER),lround(center.y*PIXEL_PER_METER));
}

PhysicalShape *PhysicalObj::GetShape(b2Shape *shape)
{
  std::list<PhysicalShape *>::iterator it;

  for (it = m_shapes.begin(); it != m_shapes.end();it++) {
    if ((*it)->GetShape() == shape) {
      return *it;
    }
  }
  return NULL;
}

PhysicalShape *PhysicalObj::GetShape(std::string name)
{
  std::list<PhysicalShape *>::iterator it;

  for (it = m_shapes.begin(); it != m_shapes.end();it++) {
    if ((*it)->GetName() == name) {
      return *it;
    }
  }
  return NULL;
}

void PhysicalObj::SetFixed(bool i_fixed)
{
  if (i_fixed != m_fixed) {
    m_fixed = i_fixed;
    GenerateMass();
  }
}
