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

#include "physic/force.h"
#include "physic/physical_engine.h"
#include "physic/physical_shape.h"
#include "physic/physics.h"
#include <stdlib.h>
#include <iostream>
#include "game/config.h"
#include "game/game_mode.h"
#include "game/time.h"
#include "tool/debug.h"
#include "tool/isnan.h"
#include "tool/math_tools.h"
#include "include/action.h"

// Physical constants
const double STOP_REBOUND_LIMIT = 0.5 ;
const double AIR_RESISTANCE_FACTOR = 40.0 ;
const double PHYS_DELTA_T = 0.02 ;         // Physical simulation time step
const double PENDULUM_REBOUND_FACTOR = 0.8 ;

Physics::Physics ():
  m_motion_type(NoMotion),
  m_extern_force_index(1),
  m_angle(0.0f),
  m_last_move(Time::GetInstance()->Read()),
  m_fix_point_gnd(),
  m_fix_point_dxy(),
  m_rope_angle(),
  m_rope_length(),
  m_rope_elasticity(10.0),
  m_elasticity_damping(0.20),
  m_balancing_damping(0.40),
  m_is_physical_obj(false),
  m_elasticity_off(true),
  m_shape(NULL),
  m_cfg()

{
  m_body_def = new b2BodyDef();
  m_body_def->allowSleep = true;
  m_body_def->linearDamping = 0.0f;
  m_body_def->angularDamping = 0.01f;

  m_body_def->position.Set(0.0f, 4.0f);
  m_body = PhysicalEngine::GetInstance()->AddObject(this);
}

Physics::~Physics()
{
  if (m_shape)
    delete m_shape;

  PhysicalEngine::GetInstance()->RemoveObject(this);
}

//---------------------------------------------------------------------------//
//--                         Class Parameters SET/GET                      --//
//---------------------------------------------------------------------------//

double Physics::GetPhysX() const
{
  return m_body->GetPosition().x;
}

double Physics::GetPhysY() const
{
  return m_body->GetPosition().y;
}

Point2d Physics::GetPos() const
{
  return Point2d( m_body->GetPosition().x,m_body->GetPosition().y);
}

void Physics::SetPhysXY(double x, double y)
{
  /* if (m_pos_x.x0 != x || m_pos_y.x0 != y) {
     m_pos_x.x0 = x;
     m_pos_y.x0 = y;*/
  m_body->SetXForm(b2Vec2(x,y),m_body->GetAngle());

  /*UpdateTimeOfLastMove();
    }*/
}

void Physics::SetPhysXY(const Point2d &position)
{
  SetPhysXY(position.x, position.y);
}

// Set the air resist factor
void Physics::SetSpeedXY (Point2d vector)
{
  if (EqualsZero(vector.x)) vector.x = 0;
  if (EqualsZero(vector.y)) vector.y = 0;
  bool was_moving = IsMoving();

  // setting to FreeFall is done in StartMoving()
  m_body->SetLinearVelocity(b2Vec2(vector.x,vector.y));
  if (!was_moving && IsMoving()) {
    UpdateTimeOfLastMove();
    StartMoving();
    m_body->WakeUp();
  }
}

void Physics::SetSpeed (double norm, double angle)
{
  SetSpeedXY(Point2d::FromPolarCoordinates(norm, angle));
}

void Physics::AddSpeedXY (Point2d vector)
{
  if (EqualsZero(vector.x)) vector.x = 0;
  if (EqualsZero(vector.y)) vector.y = 0;
  bool was_moving = IsMoving();

  m_body->SetLinearVelocity(m_body->GetLinearVelocity()+b2Vec2(vector.x,vector.y));
  // setting to FreeFall is done in StartMoving()

  if (!was_moving && IsMoving()) {
    UpdateTimeOfLastMove();
    StartMoving();
    m_body->WakeUp();
  }
}

void Physics::AddSpeed(double norm, double angle)
{
  AddSpeedXY(Point2d::FromPolarCoordinates(norm, angle));
}

void Physics::GetSpeed(double &norm, double &angle) const
{
  Point2d speed ;

  switch (m_motion_type) {
  case FreeFall:
    speed = GetSpeedXY();
    norm = speed.Norm();
    angle = speed.ComputeAngle();
    break ;

  case Pendulum:
    // Transform angular speed to linear speed.

    norm = fabs(m_rope_length.x0 * m_rope_angle.x1);

    if (m_rope_angle.x1 > 0)
      angle = fabs(m_rope_angle.x0) ;
    else
      angle = fabs(m_rope_angle.x0) - M_PI ;

    if (m_rope_angle.x0 < 0)
      angle = -angle ;
    break;

  case NoMotion:
    norm = 0.0;
    angle = 0.0;
    break;

  default:
    ASSERT(false);
    break ;
  }
}

Point2d Physics::GetSpeedXY () const
{
  return (!IsMoving()) ? Point2d(0.0, 0.0) : Point2d(m_body->GetLinearVelocity().x,m_body->GetLinearVelocity().y);
}

Point2d Physics::GetSpeed() const
{
  return GetSpeedXY();
}

double Physics::GetAngularSpeed() const
{
  return m_body->GetAngularVelocity();
}

double Physics::GetSpeedAngle() const
{
  return GetSpeedXY().ComputeAngle();
}

b2BodyDef *Physics::GetBodyDef()
{
  return m_body_def;
}


void Physics::StoreValue(Action *a)
{
  a->Push((int)m_motion_type);
  //a->Push(m_extern_force);
  a->Push((int)m_last_move);
  a->Push(m_fix_point_gnd);
  a->Push(m_fix_point_dxy);
  a->Push(m_rope_angle);
  a->Push(m_rope_length);
  a->Push(m_rope_elasticity);
  a->Push(m_elasticity_damping);
  a->Push(m_balancing_damping);
  a->Push(m_elasticity_off);

  /*MSG_DEBUG( "physic.sync", "%s now - x0:%f, x1:%f, x2:%f - y0:%f, y1:%f, y2:%f - extern_force: %f, %f",
    typeid(*this).name(),
    m_body->GetPosition().x, m_pos_x.x1, m_pos_x.x2,
    m_pos_y.x0, m_pos_y.x1, m_pos_y.x2,
    m_extern_force.x, m_extern_force.y);*/
}

void Physics::GetValueFromAction(Action *a)
{
  m_motion_type        = (MotionType_t)a->PopInt();
// m_extern_force       = a->PopPoint2d();
  m_last_move          = (uint)a->PopInt();
  m_fix_point_gnd      = a->PopPoint2d();
  m_fix_point_dxy      = a->PopPoint2d();
  m_rope_angle         = a->PopEulerVector();
  m_rope_length        = a->PopEulerVector();
  m_rope_elasticity    = a->PopDouble();
  m_elasticity_damping = a->PopDouble();
  m_balancing_damping  = a->PopDouble();
  m_elasticity_off     = !!a->PopInt();

  /*  MSG_DEBUG( "physic.sync", "%s now - x0:%f, x1:%f, x2:%f - y0:%f, y1:%f, y2:%f - extern_force: %f, %f",
      typeid(*this).name(),
      m_pos_x.x0, m_pos_x.x1, m_pos_x.x2,
      m_pos_y.x0, m_pos_y.x1, m_pos_y.x2,
      m_extern_force.x, m_extern_force.y);*/
}

void Physics::SetMass(double mass)
{
  m_mass = mass;

  b2MassData massData;
  massData.mass = m_mass;
  massData.center.SetZero();
  massData.I = 0.0f;

  m_body->SetMass(&massData);
}

uint Physics::AddExternForceXY (const Point2d& vector)
{

  m_extern_force_map[m_extern_force_index] =  new Force(this, GetPos(), vector, false) ;
  PhysicalEngine::GetInstance()->AddForce(m_extern_force_map[m_extern_force_index] );
  m_extern_force_index++;


  //bool was_moving = IsMoving();

  UpdateTimeOfLastMove();
  MSG_DEBUG ("physic.physic", "EXTERN FORCE %s.", typeid(*this).name());

//  m_extern_force.SetValues(vector);

  /*if (!was_moving && IsMoving())
    StartMoving();*/

    return m_extern_force_index-1;
}

uint Physics::AddExternForce (double norm, double angle)
{
  return AddExternForceXY(Point2d::FromPolarCoordinates(norm, angle));
}

void Physics::RemoveExternForce(unsigned index)
{
  if(index!=0){
  PhysicalEngine::GetInstance()->RemoveForce(m_extern_force_map[index]);
  delete m_extern_force_map[index];
  m_extern_force_map.erase(index);
  }
}

void Physics::ImpulseXY(const Point2d& vector)
{
  std::cout<<"Impulse x="<<vector.x<<" y ="<<vector.y<<std::endl;
    m_body->ApplyImpulse(b2Vec2(vector.x,vector.y),b2Vec2(GetPhysX(),GetPhysY()));
}

void Physics::Impulse(double norm, double angle)
{
  ImpulseXY(Point2d::FromPolarCoordinates(norm, angle));
}

void Physics::SetBullet(bool is_bullet)
{
    m_body->SetBullet(is_bullet);
}
// Set fixation point positions
/*void Physics::SetPhysFixationPointXY(double g_x, double g_y, double dx,
  double dy)
  {
  double fix_point_x, fix_point_y ;
  double old_length ;

  Point2d V ;
  m_fix_point_gnd.x = g_x ;
  m_fix_point_gnd.y = g_y ;
  m_fix_point_dxy.x = dx ;
  m_fix_point_dxy.y = dy ;

  UpdateTimeOfLastMove();
  //  printf ("Fixation (%f,%f) dxy(%f,%f)\n",  g_x, g_y, dx, dy);

  fix_point_x = m_pos_x.x0 + dx ;
  fix_point_y = m_pos_y.x0 + dy ;

  old_length = m_rope_length.x0 ;
  m_rope_length.x0 = Point2d(fix_point_x,fix_point_y).Distance( Point2d(g_x,g_y) );

  if (m_motion_type == Pendulum)
  {
  // We were already fixed. By changing the fixation point, we have
  // to recompute the angular speed depending of the new rope length.
  // And don't forget to recompute the angle, too!
  V.x = fix_point_x - g_x ;
  V.y = fix_point_y - g_y ;
  m_rope_angle.x0 = M_PI_2 - V.ComputeAngle() ;

  m_rope_angle.x1 = m_rope_angle.x1 * old_length / m_rope_length.x0 ;
  }
  else
  {
  // We switch from a regular move to a pendulum move.
  // Compute the initial angle
  V.x = fix_point_x - g_x ;
  V.y = fix_point_y - g_y ;
  m_rope_angle.x0 = M_PI_2 - V.ComputeAngle() ;

  // Convert the linear speed to angular speed.
  m_rope_angle.x1 = (m_pos_x.x1 * cos(m_rope_angle.x0) +
  m_pos_y.x1 * sin(m_rope_angle.x0) ) / m_rope_length.x0;

  // Reset the angular acceleration.
  m_rope_angle.x2 = 0 ;

  bool was_moving = IsMoving();
  m_motion_type = Pendulum ;
  if (!was_moving && IsMoving())
  StartMoving();
  }
  }
*/
void Physics::UnsetPhysFixationPoint()
{
  /* double speed_norm, angle ;

     GetSpeed (speed_norm, angle);

     angle = -angle ;

     SetSpeed(speed_norm, angle);

     UpdateTimeOfLastMove();

     m_pos_x.x2 = 0 ;
     m_pos_y.x2 = 0 ;

     m_rope_angle.Clear();
     m_rope_length.Clear();

     m_motion_type = FreeFall ;*/
}

/*void Physics::ChangePhysRopeSize(double dl)
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

void Physics::StartMoving()
{
  UpdateTimeOfLastMove();

  if (m_motion_type == NoMotion)
    m_motion_type = FreeFall ;


  MSG_DEBUG ("physic.physic", "Starting to move: %s.", typeid(*this).name());
}

void Physics::StopMoving()
{

  if(!IsMoving()) return;

  if (IsMoving()) MSG_DEBUG ("physic.physic", "Stops moving: %s.", typeid(*this).name());
  // Always called by PhysicalObj::StopMoving

  SetSpeedXY(Point2d(0.0,0.0));

  UpdateTimeOfLastMove();


  /*if (m_motion_type != Pendulum)
    m_motion_type = NoMotion ;

    m_extern_force.Clear();*/
}

bool Physics::IsMoving() const
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

bool Physics::IsSleeping() const
{
  return m_body->IsSleeping();
  /*
  // return true if not moving since 1 sec.
  int delta = Time::GetInstance()->Read() - m_last_move;
  if(delta > 400) {
  MSG_DEBUG( "physic.sleep", "%s is sleeping since %d ms.", typeid(*this).name(), delta);
  return true;
  }
  MSG_DEBUG( "physic.notsleeping", "%s is not sleeping.", typeid(*this).name());
  return false;
  */
}

void Physics::UpdateTimeOfLastMove()
{
  m_last_move = Time::GetInstance()->Read();
}

bool Physics::IsFalling() const
{
  return (m_body->GetLinearVelocity().x  > 0.1);
}


void Physics::AddAddedContactPoint(b2ContactPoint contact)
{
    added_contact_list.push_back(contact);
}

void Physics::AddPersistContactPoint(b2ContactPoint contact)
{
    persist_contact_list.push_back(contact);
}

void Physics::AddRemovedContactPoint(b2ContactPoint contact)
{
    removed_contact_list.push_back(contact);
}

void Physics::AddContactResult(b2ContactResult contact)
{
  result_contact_list.push_back(contact);
}

void Physics::ClearContact()
{
  added_contact_list.clear();
  persist_contact_list.clear();
  removed_contact_list.clear();
  result_contact_list.clear();
}

void Physics::AddContact()
{
  m_nbr_contact++;
}

void Physics::RemoveContact()
{
  m_nbr_contact--;
}


double Physics::GetAngle() const
{
   return -m_body->GetAngle();
 }

void Physics::SetAngle(double angle)
{
  m_body->SetXForm(m_body->GetPosition(), -angle/180.0f * b2_pi);

}

void Physics::SignalRebound() {}
