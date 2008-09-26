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
 * Abstract class used for physical object (object with a size, mass,
 * etc.). This object can have differents state : ready, is moving, or ghost
 * (is outside of the world).
 *
 * You can : make the object move (with collision test), change state, etc.
 * If the object go outside of the world, it become a ghost.
 *****************************************************************************/

#include <iostream>
#include "character/character.h"
#include "include/action.h"
#include "game/config.h"

/*****************************************************************************/
/* TODO REMOVE THIS INCLUDE AS SOON AS PHYSICAL ENGINE IS REPAIRED */
#include "game/game.h"
/*****************************************************************************/

#include "game/time.h"
#include "map/map.h"
#include "network/randomsync.h"
#include "physic/physical_engine.h"
#include "physic/physical_obj.h"
#include "physic/physical_shape.h"
#include "physic/physics.h"
#include "object/objects_list.h"
#include "sound/jukebox.h"
#include "team/macro.h"
#include "team/team.h"
#include "team/teams_list.h"
#include "tool/debug.h"
#include "tool/isnan.h"
#include "tool/math_tools.h"
#include "tool/point.h"
#include "tool/random.h"
#include "tool/rectangle.h"
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
  m_test_left(0),
  m_test_right(0),
  m_test_top(0),
  m_test_bottom(0),
  m_overlapping_object(NULL),
  m_minimum_overlapse_time(0),
  m_ignore_movements(false),
  m_is_character(false),
  m_is_fire(false),
  m_name(name),
  m_rebound_sound(""),
  m_alive(ALIVE),
  m_energy(-1),
  m_allow_negative_y(false)
{
  m_is_physical_obj = true;
  m_cfg = Config::GetInstance()->GetObjectConfig(m_name,xml_config);
  ResetConstants();       // Set physics constants from the xml file

  SetSize(Point2i(1,1));

  MSG_DEBUG("physical.mem", "Construction of %s", m_name.c_str());
}

PhysicalObj::~PhysicalObj ()
{
  MSG_DEBUG("physical.mem", "Destruction of %s", m_name.c_str());
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
  CheckOverlapping();

  if( IsOutsideWorldXY( Point2i(int(position.x), int(position.y)) ) )
    {

      SetPhysXY( position / PIXEL_PER_METER );
      Ghost();
      SignalOutOfMap();
    }
  else
    {
      SetPhysXY( position / PIXEL_PER_METER );
      if( FootsInVacuum() ) StartMoving();
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

void PhysicalObj::SetSize(const Point2i &newSize)
{
  double phys_width = double(newSize.x)/PIXEL_PER_METER;
  double phys_height = double(newSize.y)/PIXEL_PER_METER;

  // Shape position is relative to body
  PhysicalPolygon *shape = new PhysicalPolygon(m_body);

  shape->AddPoint(Point2d(0 , 0));
  shape->AddPoint(Point2d(phys_width, 0));
  shape->AddPoint(Point2d(phys_width, phys_height));
  shape->AddPoint(Point2d(0 , phys_height));
  shape->SetMass(GetMass());

  //Physical shape

  b2FilterData filter_data;
  filter_data.categoryBits = 0x0001;
  filter_data.maskBits = 0x0000;
  if (m_shape != NULL) {
    filter_data = m_shape->GetFilter();
  }
  shape->SetFilter(filter_data);
  shape->Generate();

  if (m_shape)
    delete m_shape;

  m_shape = shape;
}

double PhysicalObj::GetWdouble() const
{
  ASSERT(m_shape);
  double phys_width = m_shape->GetCurrentWidth();
  double pixel_width = phys_width * PIXEL_PER_METER;
  return pixel_width;
}

int PhysicalObj::GetWidth() const
{
  return int(GetWdouble()+0.5f);//Round
}

double PhysicalObj::GetHdouble() const
{
  ASSERT(m_shape);
  double phys_height = m_shape->GetCurrentHeight();
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
  int width = GetWidth() - m_test_right - m_test_left;
  int height = GetHeight() - m_test_bottom - m_test_top;
  if (width < 1)
    width = 1;
  if (height < 1)
    height = 1;
  return Rectanglei(GetX() + m_test_left, GetY() + m_test_top, width, height);
}

int PhysicalObj::GetTestWidth() const
{
  return GetWidth() - m_test_left - m_test_right;
}

int PhysicalObj::GetTestHeight() const
{
  return GetHeight() - m_test_top - m_test_bottom;
}

void PhysicalObj::StoreValue(Action *a)
{
  Physics::StoreValue(a);
  a->Push(m_collides_with_ground);
  a->Push(m_collides_with_characters);
  a->Push(m_collides_with_objects);
  a->Push((int)m_minimum_overlapse_time);
  a->Push(m_ignore_movements);
  a->Push(m_is_character);
  a->Push((int)m_test_left);
  a->Push((int)m_test_right);
  a->Push((int)m_test_top);
  a->Push((int)m_test_bottom);
  a->Push((int)m_alive);
  a->Push(m_energy);
  a->Push(m_allow_negative_y);
}

void PhysicalObj::GetValueFromAction(Action *a)
{
  Physics::GetValueFromAction(a);
  m_collides_with_ground     = !!a->PopInt();
  m_collides_with_characters = !!a->PopInt();
  m_collides_with_objects    = !!a->PopInt();
  m_minimum_overlapse_time   = (uint)a->PopInt();
  m_ignore_movements         = !!a->PopInt();
  m_is_character             = !!a->PopInt();
  m_test_left                = (uint)a->PopInt();
  m_test_right               = (uint)a->PopInt();
  m_test_top                 = (uint)a->PopInt();
  m_test_bottom              = (uint)a->PopInt();
  m_alive                    = (alive_t)a->PopInt();
  m_energy                   = a->PopInt();
  m_allow_negative_y         = !!a->PopInt();
}

void PhysicalObj::SetOverlappingObject(PhysicalObj* obj, int timeout)
{
  m_minimum_overlapse_time = 0;
  if(obj != NULL) {
    m_overlapping_object = obj;
    ObjectsList::GetRef().AddOverlappedObject(this);
    MSG_DEBUG("physic.overlapping", "\"%s\" doesn't check any collision with \"%s\" anymore during %d ms",
              GetName().c_str(), obj->GetName().c_str(), timeout);
  } else {
    if(m_overlapping_object != NULL) {
      m_overlapping_object = NULL;
      ObjectsList::GetRef().RemoveOverlappedObject(this);
      MSG_DEBUG( "physic.overlapping", "clearing overlapping object in \"%s\"", GetName().c_str());
    }
    return;
  }
  if (timeout > 0)
    m_minimum_overlapse_time = Time::GetInstance()->Read() + timeout;

  CheckOverlapping();
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
  if (m_overlapping_object == NULL)
    return;

  // Check if we are still overlapping with this object
  if (!m_overlapping_object->GetTestRect().Intersect( GetTestRect() ) &&
      m_minimum_overlapse_time <= Time::GetInstance()->Read())
    {
      MSG_DEBUG("physic.overlapping", "\"%s\" just stopped overlapping with \"%s\" (%d ms left)",
		GetName().c_str(), m_overlapping_object->GetName().c_str(),
		(m_minimum_overlapse_time - Time::GetInstance()->Read()));
      SetOverlappingObject(NULL);
    }
  else
    {
      MSG_DEBUG("physic.overlapping", "\"%s\" is overlapping with \"%s\"",
		GetName().c_str(), m_overlapping_object->GetName().c_str());
    }
}

// WARNING: MUST BE REMOVED
void PhysicalObj::SetTestRect (uint /*left*/, uint /*right*/, uint /*top*/, uint /*bottom*/)
{
  // m_test_left = left;
  // m_test_right = right;
  // m_test_top = top;
  // m_test_bottom = bottom;
}

void PhysicalObj::SetEnergyDelta(int delta, bool /*do_report*/)
{
  if (m_energy == -1)
    return;
  m_energy += delta;
  if (m_energy <= 0 && !IsGhost())
    {
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
    if ( !IsMoving() && !FootsInVacuum() && !IsInWater() ) return;

    // object is not moving BUT it should fall !
    if ( !IsMoving() && FootsInVacuum() ) StartMoving();
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
  if(IsOutsideWorld(Point2i(0, 0)))
    return false;

  if( IsInVacuum(Point2i(0, 0), false) )
    return true;

  double dx = cos(direction);
  double dy = sin(direction);
  // (dx,dy) is a normal vector (cos^2+sin^2==1)

  double step=1;
  while( step<max_distance && !IsInVacuum(
					  Point2i((int)(dx * step),(int)(dy * step)), false ))
    step+=1.0;

  if(step<max_distance)
    SetXY( Point2i((int)(dx * step)+GetX(),(int)(dy * step)+GetY()) );
  else
    return false; //Can't put the object out of the ground

  return true;
}

bool PhysicalObj::PutOutOfGround()
{
  if(IsOutsideWorld(Point2i(0, 0)))
    return false;

  if( IsInVacuum(Point2i(0, 0)) )
    return true;

  bool left,right,top,bottom;
  left   = GetWorld().IsInVacuum_left(*this, 0, 0);
  right  = GetWorld().IsInVacuum_right(*this, 0, 0);
  top    = GetWorld().IsInVacuum_top(*this, 0, 0);
  bottom = GetWorld().IsInVacuum_bottom(*this, 0, 0);

  int dx = (int)GetTestRect().GetSizeX() * (right-left);
  int dy = (int)GetTestRect().GetSizeY() * (top-bottom);

  if( dx == 0 && dy == 0 )
    return false; //->Don't know in which direction we should go...

  Point2i b(dx, dy);

  double dir = b.ComputeAngle();
  return PutOutOfGround(dir);
}

void PhysicalObj::Init()
{
  if (m_alive != ALIVE)
    MSG_DEBUG( "physic.state", "%s - Init.", m_name.c_str());
  m_alive = ALIVE;
  SetOverlappingObject(NULL);
  StopMoving();
}

void PhysicalObj::Ghost ()
{

  if (m_alive == GHOST)
    return;

  bool was_dead = IsDead();
  m_alive = GHOST;
  MSG_DEBUG("physic.state", "%s - Ghost, was_dead = %d", m_name.c_str(), was_dead);

  // The object became a gost
  StopMoving();

  SignalGhostState(was_dead);
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

  // If fire, do smoke...
  if (m_is_fire)
    GetWorld().water.Smoke(GetPosition());
  // make a splash in the water :-)
  else if (GetMass() >= 2)
    GetWorld().water.Splash(GetPosition());

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
  // TO CLEAN...
  if (!m_rebound_sound.empty())
    JukeBox::GetInstance()->Play("share", m_rebound_sound) ;
}

void PhysicalObj::SetCollisionModel(bool collides_with_ground,
                                    bool collides_with_characters,
                                    bool collides_with_objects)
{
  // SetSize() must be called before
  ASSERT(m_shape != NULL);

  m_collides_with_ground = collides_with_ground;
  m_collides_with_characters = collides_with_characters;
  m_collides_with_objects = collides_with_objects;

  b2FilterData data = m_shape->GetFilter();
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

  m_shape->SetFilter(data);
  m_shape->Generate();

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
  int x = position.x + m_test_left;
  int y = position.y + m_test_top;

  if (GetWorld().IsOutsideWorldXwidth(x, GetTestWidth()))
    return true;

  if (GetWorld().IsOutsideWorldYheight(y, GetTestHeight())) {
    if (m_allow_negative_y &&
	Y_OBJET_MIN <= y &&
	y + GetTestHeight() - 1 < 0 )
	return false;

    return true;
  }
  return false;
}

bool PhysicalObj::IsOutsideWorld(const Point2i &offset) const
{
  return IsOutsideWorldXY( GetPosition() + offset );
}

int count = 0;

bool PhysicalObj::FootsOnFloor(int y) const
{
  //TODO : calculate collision with water
  // If outside is empty, the object can't hit the ground !
  if ( GetWorld().IsOpen() ) return false;

  const int y_max = GetWorld().GetHeight() +m_test_bottom;
  return (y_max <= y);
}

bool PhysicalObj::IsInVacuumXY(const Point2i &position, bool check_object) const
{
  if (IsOutsideWorldXY(position))
    return GetWorld().IsOpen();

  if (FootsOnFloor(position.y - 1))
    return false;

  if (check_object && CollidedObjectXY(position))
    return false;

  Rectanglei rect(position.x + m_test_left, position.y + m_test_top,
		  GetTestWidth(), GetTestHeight());

  return GetWorld().RectIsInVacuum(rect);
}

bool PhysicalObj::IsInVacuum(const Point2i &offset, bool check_objects) const
{
  return IsInVacuumXY(GetPosition() + offset, check_objects);
}

bool PhysicalObj::FootsInVacuum() const
{
  /*  Point2i position = GetPosition();

  if (IsOutsideWorldXY(position)) {
    MSG_DEBUG("physical", "%s - physobj is outside the world", m_name.c_str());
    return GetWorld().IsOpen();
  }

  if (FootsOnFloor(position.y)) {
    MSG_DEBUG("physical", "%s - physobj is on floor", m_name.c_str());
    return false;
  }

  int y_test = position.y + GetHeight() - m_test_bottom;

  Rectanglei rect( position.x + m_test_left, y_test,
                   GetWidth() - m_test_right - m_test_left, 1);

  if (m_allow_negative_y && rect.GetPositionY() < 0) {
    int b = rect.GetPositionY() + rect.GetSizeY();

    rect.SetPositionY( 0 );
    rect.SetSizeY( ( b > 0 ) ? b - rect.GetPositionY() : 0 );
  }

  if (CollidedObjectXY( position + Point2i(0, 1)) != NULL )
    return false;

    return GetWorld().RectIsInVacuum (rect);*/
  return (m_nbr_contact == 0);
}

PhysicalObj* PhysicalObj::CollidedObjectXY(const Point2i & position) const
{
  if( IsOutsideWorldXY(position) )
    return NULL;

  Rectanglei rect(position.x + m_test_left, position.y + m_test_top,
		  GetTestWidth(), GetTestHeight());

  if (m_collides_with_characters)
    {
      FOR_ALL_LIVING_CHARACTERS(team,character)
      {
        // We check both objet if one overlapse the other
        if ( (PhysicalObj*)(*character) != this &&
	     !IsOverlapping(*character) &&
	     !(*character)->IsOverlapping(this) &&
	     (*character)->GetTestRect().Intersect( rect ))
          return (PhysicalObj*)(*character);
      }
    }

  if (m_collides_with_objects)
    {
      FOR_EACH_OBJECT(it)
      {
        PhysicalObj * object=*it;
        // We check both objet if one overlapse the other
        if (object != this && !IsOverlapping(object) && !object->IsOverlapping(this)
            && object->m_collides_with_objects
            && object->GetTestRect().Intersect(rect) )
	  {
	    if (!m_is_character || object->m_collides_with_characters)
            return object;
	  }
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
  if (!GetWorld().water.IsActive()) return false;
  int x = InRange_Long(GetCenterX(), 0, GetWorld().GetWidth()-1);
  return (int)GetWorld().water.GetHeight(x) < GetCenterY();
}

void PhysicalObj::DirectFall()
{
  while (!IsGhost() && !IsInWater() && FootsInVacuum())
    SetY(GetYdouble()+1.0);
}

bool PhysicalObj::IsImmobile() const
{
  bool r = IsSleeping()
    || m_ignore_movements
    || (!IsMoving() && !FootsInVacuum())
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

bool PhysicalObj::IsFire() const
{
  return m_is_fire;
}

// Are the two object in contact ? (uses test rectangles)
bool PhysicalObj::Overlapse(const PhysicalObj &b) const
{
  return GetTestRect().Intersect( b.GetTestRect() );
}

// Do the point p touch the object ?
bool PhysicalObj::Contain(const Point2i &p) const
{
  return  GetTestRect().Contains( p );
}

bool PhysicalObj::PutRandomly(bool on_top_of_world, double min_dst_with_characters, bool net_sync)
{
  uint bcl=0;
  uint NB_MAX_TRY = 60;
  bool ok;
  Point2i position;

  MSG_DEBUG("physic.position", "%s - Search a position...", m_name.c_str());

  do
  {
    bcl++;
    ok = true;
    Init();

    if (bcl >= NB_MAX_TRY) {
      MSG_DEBUG("physic.position", "%s - Impossible to find an initial position !!", m_name.c_str());
      return false;
    }

    if (on_top_of_world) {
      // Give a random position for x
      if(net_sync)
        position.x = RandomSync().GetLong(0, GetWorld().GetWidth() - GetWidth());
      else
        position.x = RandomLocal().GetLong(0, GetWorld().GetWidth() - GetWidth());
      position.y = -GetHeight()+1;
    } else {
      if(net_sync)
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
    FOR_ALL_LIVING_CHARACTERS(team, character) if ((*character) != this)
    {
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
        if (dst < min_dst_with_characters) ok = false;
      }
    }

    if (ok && on_top_of_world) SetXY(position);
  } while (!ok);

  MSG_DEBUG("physic.position", "Put '%s' after %u tries", m_name.c_str(), bcl);

  return true;
}

#ifdef DEBUG
void PhysicalObj::DrawPolygon(const Color& color) const
{
  m_shape->DrawBorder(color);
}
#endif
