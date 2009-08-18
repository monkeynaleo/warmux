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
#include <WORMUX_random.h>
#include "weapon/weapon_launcher.h"
#include "game_obj.h"
#include "physical_engine.h"

#ifdef DEBUG
#include "graphic/color.h"
#endif

const int Y_OBJET_MIN = -10000;
const int WATER_RESIST_FACTOR = 40;


GameObj::GameObj (const std::string &name, const std::string &xml_config) :
 
  m_last_move(Time::GetInstance()->Read()),
  m_alive(ALIVE),
  m_energy(-1),
  m_name(name),
  m_allow_negative_y(false),
  m_ignore_movements(false),
 
  m_rebound_sound("")
  
{
  m_cfg = Config::GetInstance()->GetObjectConfig(m_name, xml_config);
  SetPhysic(PhysicalEngine::GetInstance()->CreateObject(m_cfg.m_type));
  GetPhysic()->SetContactListener(this);
  PhysicalEngine::GetInstance()->AddObject(GetPhysic());
  //ResetConstants();       // Set physics constants from the xml file

  /*m_body_def = new b2BodyDef();
  m_body_def->allowSleep = true;

  m_body_def->linearDamping = m_air_resist_factor;
  m_body_def->angularDamping = 0.1;
  m_body_def->position.Set(0.0f, 0.0f);
  m_body_def->fixedRotation = !m_rotating;
*/
 /* PhysicalCircle *circle =  PhysicalEngine::GetInstance()->CreateCircleShape();
  circle->SetRadius(10);
  circle->SetPosition(Point2d(0,0));
  circle->Generate();
  m_physic->AddShape(circle);*/
  InitShape(xml_config);
 /* if(m_auto_align_force >0)
  {
      PhysicalEngine::GetInstance()->AddAutoAlignObject(this);
  }
*/
  MSG_DEBUG("physical.mem", "Construction of %s", m_name.c_str());
}

GameObj::~GameObj ()
{
  MSG_DEBUG("physical.mem", "Destruction of %s", m_name.c_str());
  delete m_physic;
}

void GameObj::InitShape(const std::string &xml_config)
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
/*
    b2FilterData filter_data;
    filter_data.categoryBits = 0x0001;
    filter_data.maskBits = 0x0000;
    filter_data.maskBits = 0;
    shape->SetFilter(filter_data);
*/
    m_physic->AddShape(shape);
  }


/*
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
  }*/

  m_physic->Generate();
}


//-----------------------------------------------------------------------------


//---------------------------------------------------------------------------//
//--                         Class Parameters SET/GET                      --//
//---------------------------------------------------------------------------//


void GameObj::SetPosition(const Point2d &position)
{
   if (IsOutsideWorldXY(Point2i(int(position.x), int(position.y)))) {

      GetPhysic()->SetPosition(position);
      Ghost();
      SignalOutOfMap();

    } else {
      GetPhysic()->SetPosition(position);
    }
}

void GameObj::SetSafePosition(const Point2d &position) {
    GetPhysic()->SetPosition(position);
}

void GameObj::SetX(double x){
    SetPosition(Point2d(x,GetPosition().y));
}

void GameObj::SetY(double y){
    SetPosition(Point2d(GetPosition().x,y));
}

const Point2d GameObj::GetPosition() const
{
    return GetPhysic()->GetPosition();
}


double GameObj::GetX() const
{
  return GetPosition().x;
}

double GameObj::GetY() const
{
   return GetPosition().y;
}

double GameObj::GetMinX() const
{
    return GetRect().GetPositionX();
}

double GameObj::GetMinY() const
{
    return GetRect().GetPositionY();
}

double GameObj::GetMaxX() const
{
    return GetRect().GetPositionX() + GetRect().GetSizeX();
}

double GameObj::GetMaxY() const
{
    return GetRect().GetPositionY() + GetRect().GetSizeY();
}
void GameObj::SetSpeedXY (Point2d vector)
{

  if (EqualsZero(vector.x))
    vector.x = 0;
  if (EqualsZero(vector.y))
    vector.y = 0;

  bool was_moving = IsMoving();

  // setting to FreeFall is done in StartMoving()
  GetPhysic()->SetSpeedXY(vector);
  if (!was_moving && IsMoving()) {
    UpdateTimeOfLastMove();
  }
}

void GameObj::SetSpeed (double norm, double angle)
{
  SetSpeedXY(Point2d::FromPolarCoordinates(norm, angle));
}

void GameObj::AddSpeedXY (Point2d vector)
{
    SetSpeedXY(GetSpeed()+vector);
}

void GameObj::AddSpeed(double norm, double angle)
{
  AddSpeedXY(Point2d::FromPolarCoordinates(norm, angle));
}

void GameObj::GetSpeed(double &norm, double &angle) const
{
  Point2d speed ;

  speed = GetSpeed();
  norm = speed.Norm();
  angle = speed.ComputeAngle();
}

Point2d GameObj::GetSpeed () const
{
  return GetPhysic()->GetSpeed();
}

double GameObj::GetAngularSpeed() const
{
    return GetPhysic()->GetAngularSpeed();
}

double GameObj::GetSpeedAngle() const
{
  return GetSpeed().ComputeAngle();
}


double GameObj::GetMass() const
{
  return GetPhysic()->GetMass();
}

Point2d GameObj::GetSize() const
{
  return Point2d(GetRect().GetSizeX(),GetRect().GetSizeX());
}

const Rectangled GameObj::GetRect() const
{
    return GetPhysic()->GetBoundingBox();
}
const Rectanglei GameObj::GetRectI() const
{
    Rectangled rect = GetRect();
    Rectanglei out;
    out.SetPosition(rect.GetPosition());
    out.SetSize(rect.GetSize());
    return out;
}
void GameObj::SetEnergy(int energy){
    m_energy = energy;
}

void GameObj::StoreValue(Action *a)
{
  // Position
  a->Push(GetPosition());
  a->Push(GetAngle());

  // Speed
  double norm, angle;
  GetSpeed(norm, angle);
  a->Push(norm);
  a->Push(angle);

  // Collision information
  /*a->Push(m_collides_with_ground);
  a->Push(m_collides_with_characters);
  a->Push(m_collides_with_objects);
  a->Push(m_collides_with_projectiles);
  a->Push((int)m_minimum_overlapse_time);*/

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
/*
  MSG_DEBUG("physic.sync", "%s now - position x:%f, y:%f - speed x:%f, y:%f, angle:%f",
	    typeid(*this).name(), GetPhysXY().x, GetPhysXY().y, GetSpeed().x, GetSpeed().y, GetAngularSpeed());
*/
  a->Push(m_ignore_movements);
  a->Push(GetType());
  a->Push((int)m_alive);
  a->Push(m_energy);
  a->Push(m_allow_negative_y);
}

void GameObj::GetValueFromAction(Action *a)
{
  // Position
  Point2d position = a->PopPoint2d();
  SetPosition(position);
  double angle = a->PopDouble();
  SetAngle(angle);

  // Speed
  double norm = a->PopDouble();
  double speed_angle = a->PopDouble();
  SetSpeed(norm, speed_angle);

/*  // Collision information
  bool collides_with_ground, collides_with_characters, collides_with_objects,collides_with_projectiles;
  collides_with_ground     = !!a->PopInt();
  collides_with_characters = !!a->PopInt();
  collides_with_objects    = !!a->PopInt();
  collides_with_projectiles= !!a->PopInt();

    GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_GROUND,collides_with_ground);
    GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_CHARACTER,collides_with_characters);
    GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_ITEM,collides_with_objects);
    GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_PROJECTILE,collides_with_projectiles);

  m_minimum_overlapse_time   = (uint)a->PopInt();
*/
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
/*
  MSG_DEBUG("physic.sync", "%s now - position x:%f, y:%f - speed x:%f, y:%f, angle:%f",
	    typeid(*this).name(),  GetPhysXY().x, GetPhysXY().y, GetSpeed().x, GetSpeed().y, GetAngularSpeed());
*/
  m_ignore_movements         = !!a->PopInt();
  m_type                     =    (GameObjType) a->PopInt();
  m_alive                    = (alive_t)a->PopInt();
  m_energy                   = a->PopInt();
  m_allow_negative_y         = !!a->PopInt();
}
bool GameObj::CollidesWithGround(){
    //TODO : Implement
    return false;
}

void GameObj::UpdateTimeOfLastMove()
{
  m_last_move = Time::GetInstance()->Read();
}

void GameObj::SetEnergyDelta(int delta, bool /*do_report*/)
{
  if (m_energy == -1)
    return;

  m_energy += delta;
  if (m_energy <= 0 && !IsGhost()) {
    Ghost();
    m_energy = -1;
  }
}

void GameObj::UpdatePosition ()
{
  if ( IsOutsideWorldXY( Point2i( GetX(),GetY()) )) {
    Ghost();
    SignalOutOfMap();
  }

  // No ghost allowed here !
  if (IsGhost())
    return;
/*
  if (m_collides_with_ground) {

    // object is not moving and has no reason to move
    if ( !IsMoving() && IsColliding() && !IsInWater() ) return;

    // object is not moving BUT it should fall !
    if ( !IsMoving() && IsColliding() ) StartMoving();
  }

  if (IsGhost())
    return;

  if (IsInWater()) {

    GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_GROUND,false);
    GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_CHARACTER,false);
    GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_ITEM,false);
    GetPhysic()->SetCollisionCategory(PhysicalObj::COLLISION_PROJECTILE,false);
  }

  // Classical object sometimes sinks in water and sometimes goes out of water!
  if (m_collides_with_ground) {

    if ( IsInWater() && m_alive != DROWNED && m_alive != DEAD)
      Drown();
    else if ( !IsInWater() && m_alive == DROWNED )
      GoOutOfWater();
  }
  CheckOverlapping();*/
}

bool GameObj::PutOutOfGround(double /*direction*/, double /*max_distance*/)
{
  /*if (IsOutsideWorld(Point2i(0, 0)))
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
*/
  return true;
}

bool GameObj::PutOutOfGround()
{
 /* if (IsOutsideWorld(Point2i(0, 0)))
    return false;

  if (IsInVacuum(Point2i(0, 0)))
    return true;

  bool left,right,top,bottom;
  left   = GetWorld().IsInVacuum_left(*this, 0, 0);
  right  = GetWorld().IsInVacuum_right(*this, 0, 0);
  top    = GetWorld().IsInVacuum_top(*this, 0, 0);
  bottom = GetWorld().IsInVacuum_bottom(*this, 0, 0);

  int dx = (int)GetRect().GetSizeX() * (right-left);
  int dy = (int)GetRect().GetSizeY() * (top-bottom);

  if (dx == 0 && dy == 0)
    return false; //->Don't know in which direction we should go...

  Point2i b(dx, dy);

  double dir = b.ComputeAngle();
  return PutOutOfGround(dir);*/
    return false;
}

void GameObj::Init()
{
  GetPhysic()->Activate();
  if (m_alive != ALIVE)
    MSG_DEBUG( "physic.state", "%s - Init.", m_name.c_str());

  m_alive = ALIVE;
  StopMovement();
}

void GameObj::Ghost ()
{
  if (m_alive == GHOST)
    return;

  #ifdef WMX_LOG
  	   bool was_dead = IsDead();
  	 #endif
   m_alive = GHOST; 	   m_alive = GHOST;
  	 #ifdef WMX_LOG
   MSG_DEBUG("physic.state", "%s - Ghost, was_dead = %d", m_name.c_str(), was_dead); 	   MSG_DEBUG("physic.state", "%s - Ghost, was_dead = %d", m_name.c_str(), was_dead);
  	 #endif
  SignalGhostState(m_energy==0);
  // The object became a gost
  StopMovement();
}

void GameObj::Drown()
{
  ASSERT (m_alive != DROWNED);
  MSG_DEBUG("physic.state", "%s - Drowned...", m_name.c_str());
  m_alive = DROWNED;

  // Set the air grab to water resist factor.
  GetPhysic()->SetAirFrictionFactor(WATER_RESIST_FACTOR * GetPhysic()->GetAirFrictionFactor());

  // Ensure the gravity factor is upper than 0.0
  if (EqualsZero(GetPhysic()->GetGravityFactor()))
    GetPhysic()->SetGravityFactor(0.1);

  if (GetType() == GAME_FIRE) {
    // If fire, do smoke...
    GetWorld().water.Smoke(GetPosition());
  } else if (GetMass() >= 2) {
    // make a splash in the water :-)
    GetWorld().water.Splash(GetPosition());
  }

  StopMovement();
  SignalDrowning();
}

void GameObj::GoOutOfWater()
{
  ASSERT (m_alive == DROWNED);
  MSG_DEBUG("physic.state", "%s - Go out of water!...", m_name.c_str());
  m_alive = ALIVE;

  // Set the air grab to normal air resist factor.
  GetPhysic()->SetAirFrictionFactor(m_cfg.m_air_resist_factor);
  GetPhysic()->SetGravityFactor(m_cfg.m_gravity_factor);
  SignalGoingOutOfWater();
}

void GameObj::SignalRebound()
{
  // TO CLEAN...:
  if (!m_rebound_sound.empty())
    JukeBox::GetInstance()->Play("default", m_rebound_sound) ;
}

bool GameObj::IsSleeping() const {
    return GetPhysic()->IsSleeping();
}

bool GameObj::IsOutsideWorldXY(const Point2i& position) const
{
  // to take object rotation into account
  int x =  position.x - GetWidth()/2;
  int y = position.y - GetHeight()/2;

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

bool GameObj::IsOutsideWorld(const Point2i &offset) const
{
  return IsOutsideWorldXY( GetPhysic()->GetPosition() + offset );
}

bool GameObj::IsInVacuumXY(const Point2i &position, bool check_object) const
{
  if (IsOutsideWorldXY(position))
    return GetWorld().IsOpen();

  if (check_object && GetPhysic()->CollidedObjectXY(position))
    return false;

  // to take object rotation into account
  Rectanglei rect(GetMinX() - GetX() + position.x, GetMinY() - GetY() + position.y,
		  GetWidth(), GetHeight());

  return GetWorld().RectIsInVacuum(rect);
}

bool GameObj::IsInVacuum(const Point2i &offset, bool check_objects) const
{
  return IsInVacuumXY(GetPosition() + offset, check_objects);
}

bool GameObj::IsInWater () const
{
  ASSERT (!IsGhost());
  if (!GetWorld().water.IsActive())
    return false;

  int x = InRange_Long(GetX(), 0, GetWorld().GetWidth()-1);

  return (int)GetWorld().water.GetHeight(x) < GetY();
}

void GameObj::DirectFall()
{

  while (!IsGhost() && !IsInWater() && !GetPhysic()->IsColliding()) {
    MSG_DEBUG("physic.fall", "%s - x=%f, y=%f\n", m_name.c_str(), GetX(), GetY());
    SetY(GetY()+1.0);

  }
}

bool GameObj::IsImmobile() const
{
  bool r = IsSleeping()
    || m_ignore_movements
    || (!IsMoving() && GetPhysic()->IsColliding())
    || IsGhost();

  return r;
}

bool GameObj::IsGhost() const
{
  return (m_alive == GHOST);
}

bool GameObj::IsDrowned() const
{
  return (m_alive == DROWNED);
}

bool GameObj::IsDead() const
{
  bool r = IsGhost()
    || IsDrowned()
    || m_alive == DEAD;

  return r;
}

bool GameObj::PutRandomly(bool on_top_of_world, double min_dst_with_characters, bool net_sync)
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

    SetSafePosition(position);

    MSG_DEBUG("physic.position", "%s (try %u/%u) - Test in %d, %d",
              m_name.c_str(), bcl, NB_MAX_TRY, position.x, position.y);

    // Check physical object is not in the ground
    Rectangled rect = GetRect();
    Rectanglei recti;
    recti.SetPositionX((int)rect.GetPositionX());
    recti.SetPositionY((int)rect.GetPositionY());
    recti.SetSizeX((int)rect.GetSizeX());
    recti.SetSizeY((int)rect.GetSizeY());

    ok &= !IsGhost() && GetWorld().ParanoiacRectIsInVacuum(recti)  && IsInVacuum( Point2i(0, 1) );
    if (!ok) {
      MSG_DEBUG("physic.position", "%s - Put it in the ground -> try again !", m_name.c_str());
      continue;
    }

    /* check if the area rigth under the object has a bottom on the ground */
    ok &= !GetWorld().ParanoiacRectIsInVacuum(Rectanglei(GetPosition().x, position.y, 1, GetWorld().GetHeight() -
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

	  if(GetPhysic()->IsColliding((**character).GetPhysic())) {
            MSG_DEBUG("physic.position", "%s - Object is too close from character %s", m_name.c_str(), (*character)->m_name.c_str());
            ok = false;
	  }

	} else {
	  Point2i p1 = (*character)->GetPosition();
	  Point2i p2 = GetPosition();
	  double dst = p1.Distance( p2 );

	  // ok this test is not perfect but quite efficient ;-)
	  // else we need to check each distance between each "corner"
	  if (dst < min_dst_with_characters)
	    ok = false;
	}
      }

      if (ok && on_top_of_world)
	SetPosition(position);
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

void GameObj::DrawShape(const Color& color) const
{
  GetPhysic()->DrawShape(color);
 /* std::list<PhysicalShape*>::const_iterator it;

  for (it = m_shapes.begin(); it != m_shapes.end(); it++) {
    (*it)->DrawBorder(color);
  }*/

  /*Rectanglei test_rect = GetRectI();

  Rectanglei rect(test_rect.GetPosition() - Camera::GetRef().GetPosition(),
		  test_rect.GetSize());*/

  //GetMainWindow().RectangleColor(rect, primary_blue_color);

  if (GetMass()) {
/*    std::string txt = Format("%.2f kg", m_body->GetMass());
    Text mass_text(txt, white_color, Font::FONT_TINY, Font::FONT_NORMAL, false);
    mass_text.DrawTopLeft(GetPosition() - Camera::GetRef().GetPosition());*/
  }
}
#endif

//---------------------------------------------------------------------------//
//--                            Physical Simulation                        --//
//---------------------------------------------------------------------------//


void GameObj::StopMovement()
{
  if (!IsMoving())
    return;

  MSG_DEBUG ("physic.physic", "Stops moving: %s.", typeid(*this).name());
  GetPhysic()->StopMovement();
 
  UpdateTimeOfLastMove();

}

bool GameObj::IsMoving() const
{
    return GetPhysic()->IsMoving();
}

bool GameObj::IsFalling() const
{
  return (GetSpeed().y  > 0.1);
}


double GameObj::GetAngle() const
{
  return -GetPhysic()->GetAngle();
}

void GameObj::SetAngle(double angle)
{
    GetPhysic()->SetAngle(angle);
}

void GameObj::SetFixed(bool i_fixed)
{
    GetPhysic()->SetFixed(i_fixed);
}

bool GameObj::IsRotationFixed() const{
    return GetPhysic()->IsRotationFixed();
}
PhysicalObj *GameObj::GetPhysic() const {
    ASSERT(m_physic);
    return m_physic;
}

void GameObj::SetPhysic(PhysicalObj *obj) {
    m_physic = obj;
}
