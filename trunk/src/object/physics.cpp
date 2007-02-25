/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
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

#include "../object/physics.h"
#include <stdlib.h>
#include <iostream>
#include "../game/config.h"
#include "../game/game_mode.h"
#include "../game/time.h"
#include "../tool/debug.h"
#include "../tool/math_tools.h"
#include "../map/wind.h"

// Physical constants
const double STOP_REBOUND_LIMIT = 0.5 ;
const double AIR_RESISTANCE_FACTOR = 40.0 ;
const double PHYS_DELTA_T = 0.02 ;         // Physical simulation time step
const double PENDULUM_REBOUND_FACTOR = 0.8 ;

Physics::Physics ()
{
  m_extern_force.Clear();
  m_pos_x.Clear();
  m_pos_y.Clear();

  m_fix_point_gnd.Clear();
  m_fix_point_dxy.Clear();
  m_rope_angle.Clear();
  m_rope_length.Clear();

  m_rope_elasticity = 10.0 ;
  m_elasticity_damping = 0.20 ;
  m_balancing_damping = 0.40 ;

  m_motion_type = NoMotion ;
  m_elasticity_off = true;

  m_last_move = Time::GetInstance()->Read() ;
}

void Physics::ResetConstants()
{
  // Load the constants (mass, air_resistance...) into the object
  *((ObjectConfig*)this) = m_cfg;
}

Physics::~Physics () {}

//---------------------------------------------------------------------------//
//--                         Class Parameters SET/GET                      --//
//---------------------------------------------------------------------------//

// Set / Get positions

void Physics::SetPhysXY(double x, double y)
{
  m_pos_x.x0 = x;
  m_pos_y.x0 = y;
}

void Physics::SetPhysXY(const Point2d &position){
	SetPhysXY(position.x, position.y);
}

double Physics::GetPhysX() const
{
  return m_pos_x.x0;
}

double Physics::GetPhysY() const
{
  return m_pos_y.x0;
}

Point2d Physics::GetPos() const{
	return Point2d( m_pos_x.x0, m_pos_y.x0);
}

void Physics::SetPhysSize (double width, double height)
{
  m_phys_width = width ;
  m_phys_height = height ;
}

// Set positions
void Physics::SetMass(double mass)
{
  m_mass = mass ;
}

// Set the wind factor
void Physics::SetWindFactor (double factor)
{
  m_wind_factor = factor;
}

// Set the air resist factor
void Physics::SetAirResistFactor (double factor)
{
  m_air_resist_factor = factor;
}

// Set the wind factor
void Physics::SetGravityFactor (double factor)
{
  m_gravity_factor = factor;
}

void Physics::SetSpeed (double length, double angle)
{
  Point2d vector( length*cos(angle), length*sin(angle) );
  SetSpeedXY(vector);
}

void Physics::SetSpeedXY (Point2d vector)
{
  if (EgalZero(vector.x)) vector.x = 0;
  if (EgalZero(vector.y)) vector.y = 0;
  bool was_moving = IsMoving();

  m_pos_x.x1 = vector.x ;
  m_pos_y.x1 = vector.y ;
  m_motion_type = FreeFall ;

  if (!was_moving && IsMoving()) StartMoving();
}

void Physics::AddSpeed (double length, double angle)
{
  Point2d vector( length*cos(angle), length*sin(angle) );
  AddSpeedXY (vector);
}

void Physics::AddSpeedXY (Point2d vector)
{
  if (EgalZero(vector.x)) vector.x = 0;
  if (EgalZero(vector.y)) vector.y = 0;
  bool was_moving = IsMoving();

  m_pos_x.x1 += vector.x ;
  m_pos_y.x1 += vector.y ;
  m_motion_type = FreeFall ;

  if (!was_moving && IsMoving()) StartMoving();
}

void Physics::GetSpeed(double &norm, double &angle) const
{
  Point2d speed ;

  switch (m_motion_type) {
    case FreeFall:
      GetSpeedXY(speed);
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
      assert(false);
      break ;
  }
}

void Physics::GetSpeedXY(Point2d &vector) const
{
  if(!IsMoving())
    {
      vector.Clear();
      return;
    }
  vector.SetValues(m_pos_x.x1, m_pos_y.x1);
}

Point2d Physics::GetSpeed() const
{
  Point2d tmp;
  GetSpeedXY(tmp);
  return tmp;
}

double Physics::GetAngularSpeed() const
{
  return m_rope_angle.x1 ;
}

double Physics::GetSpeedAngle() const
{
  double angle ;
  Point2d speed ;

  GetSpeedXY(speed);
  angle = speed.ComputeAngle();

  return angle ;
}

void Physics::SetExternForce (double length, double angle)
{
  Point2d vector(length*cos(angle), length*sin(angle));

  SetExternForceXY(vector);
}

void Physics::SetExternForceXY (Point2d vector)
{
  bool was_moving = IsMoving();

  m_extern_force.SetValues(vector);

  if (!was_moving && IsMoving())
    StartMoving();
}

Point2d Physics::GetExternForce() const
{
  return m_extern_force;
}

// Set fixation point positions
void Physics::SetPhysFixationPointXY(double g_x, double g_y, double dx,
				     double dy)
{
  double fix_point_x, fix_point_y ;
  double old_length ;

  Point2d V ;
  m_fix_point_gnd.x = g_x ;
  m_fix_point_gnd.y = g_y ;
  m_fix_point_dxy.x = dx ;
  m_fix_point_dxy.y = dy ;

  //  printf ("Fixation (%f,%f) dxy(%f,%f)\n",  g_x, g_y, dx, dy);

  fix_point_x = m_pos_x.x0 + dx ;
  fix_point_y = m_pos_y.x0 + dy ;

  old_length = m_rope_length.x0 ;
  m_rope_length.x0 = Point2d(fix_point_x,fix_point_y).Distance( Point2d(g_x,g_y) );

  if (m_motion_type == Pendulum)
    {
      // We was already fixed. By changing the fixation point, we have
      // to recompute the angular speed depending of the new rope length.
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
      if (!was_moving && IsMoving()) StartMoving();
    }
}

void Physics::UnsetPhysFixationPoint()
{
  double speed_norm, angle ;

  GetSpeed (speed_norm, angle);

  angle = -angle ;

  SetSpeed(speed_norm, angle);

  m_pos_x.x2 = 0 ;
  m_pos_y.x2 = 0 ;

  m_rope_angle.Clear();
  m_rope_length.Clear();

  m_motion_type = FreeFall ;
}

void Physics::ChangePhysRopeSize(double dl)
{
  if ((dl < 0) && (m_rope_length.x0 < 0.5))
    return ;

  bool was_moving = IsMoving();

  m_rope_length.x0 += dl ;

  // Recompute angular speed depending on the new rope length.
  m_rope_angle.x1 = m_rope_angle.x1 * (m_rope_length.x0 - dl) / m_rope_length.x0 ;

  if (!was_moving && IsMoving()) StartMoving();
}

double Physics::GetRopeAngle()
{
  return m_rope_angle.x0 ;
}

double Physics::GetRopeLength()
{
  return m_rope_length.x0;
}

//---------------------------------------------------------------------------//
//--                            Physical Simulation                        --//
//---------------------------------------------------------------------------//

void Physics::StartMoving()
{
  m_last_move = Time::GetInstance()->Read();

  if (m_motion_type == NoMotion)
    m_motion_type = FreeFall ;

  MSG_DEBUG ("physic.physic", "Start moving.");
}

void Physics::StopMoving()
{
  if (IsMoving()) MSG_DEBUG ("physic.physic", "End of a movement...");
  // Always called by PhysicalObj::StopMoving
  m_pos_x.x1 = 0 ;
  m_pos_x.x2 = 0 ;
  m_pos_y.x1 = 0 ;
  m_pos_y.x2 = 0 ;
  if (m_motion_type != Pendulum)
    m_motion_type = NoMotion ;

  m_extern_force.Clear();
}

bool Physics::IsMoving() const
{
  return ( (!EgalZero(m_pos_x.x1)) ||
	   (!EgalZero(m_pos_y.x1)) ||
	   (!m_extern_force.IsNull() ) ||
	   (m_motion_type != NoMotion) ) ;
//	   (m_motion_type == Pendulum) ) ;
}

bool Physics::IsFalling() const
{
  return ( ( m_motion_type == FreeFall ) &&
	   ( m_pos_y.x1 > 0.1) );
}

// Compute the next position of the object during a pendulum motion.
void Physics::ComputePendulumNextXY (double delta_t)
{
  //  double l0 = 5.0 ;

  //  printf ("Physics::ComputePendulumNextXY - Angle %f\n", m_rope_angle.x0);

  // Elactic rope length equation
  // l" + D.l' + (k/m - a'^2).l = g.cos a + k/m . l0

//   ComputeOneEulerStep(m_rope_length,
// 		      /* a */ 1,
// 		      /* b */ m_elasticity_damping,
// 		      /* c */ m_rope_elasticity / m_mass - m_rope_angle.x1 * m_rope_angle.x1,
// 		      /* d */ game_mode.gravity * cos (m_rope_angle.x0) + m_rope_elasticity/m_mass * l0,
//                       delta_t);

  // Pendulum motion equation (angle equation)
  // a'' + (D + 2.l'/l).a' = -g/l.sin a + F/l.cos a
  m_rope_angle.ComputeOneEulerStep(
		      /* a */ 1,
		      /* b */ m_balancing_damping + 2 * m_rope_length.x1 / m_rope_length.x0,
		      /* c */ 0,
		      /* d */ -GameMode::GetInstance()->gravity / m_rope_length.x0 * sin (m_rope_angle.x0)
		              +m_extern_force.x / m_rope_length.x0 * cos (m_rope_angle.x0),
                      delta_t);

  double x = m_fix_point_gnd.x - m_fix_point_dxy.x
             + m_rope_length.x0 * sin(m_rope_angle.x0);
  double y = m_fix_point_gnd.y - m_fix_point_dxy.y
             + m_rope_length.x0 * cos(m_rope_angle.x0);

  //  printf ("Physics::ComputePendulumNextXY - Angle(%f,%f,%f)\n",
  //  	  m_rope_angle.x0, m_rope_angle.x1, m_rope_angle.x2);

  SetPhysXY(x,y);
}

// Compute the next position of the object during a free fall.
void Physics::ComputeFallNextXY (double delta_t)
{
  double speed_norm, speed_angle ;
  double air_resistance_factor ;

  double weight_force ;
  double wind_force ;

  // Free fall motion equation
  // m.g + wind -k.v = m.a

  // Weight force = m * g

  // printf ("av : (%5f,%5f) - (%5f,%5f) - (%5f,%5f)\n", m_pos_x.x0,
  //	  m_pos_y.x0, m_pos_x.x1, m_pos_y.x1, m_pos_x.x2, m_pos_y.x2);

  weight_force = GameMode::GetInstance()->gravity * m_gravity_factor * m_mass ;

  // Wind force

  wind_force = wind.GetStrength() * m_wind_factor ;

  // Air resistanceance factor

  GetSpeed(speed_norm, speed_angle);

  air_resistance_factor = AIR_RESISTANCE_FACTOR * m_air_resist_factor ;

  MSG_DEBUG( "physic.fall", "Fall %s; mass %5f, weight %5f, wind %5f, air %5f", typeid(*this).name(), m_mass, weight_force,wind_force, air_resistance_factor);

  // Equation on X axys : m.x'' + k.x' = wind
  m_pos_x.ComputeOneEulerStep(m_mass, air_resistance_factor, 0,
		      wind_force + m_extern_force.x, delta_t);

  // Equation on Y axys : m.y'' + k.y' = m.g
  m_pos_y.ComputeOneEulerStep(m_mass, air_resistance_factor, 0,
		      weight_force + m_extern_force.y, delta_t);


    // printf ("F : Pd(%5f) EF(%5f)\n", weight_force, m_extern_force.y);

   // printf ("ap : (%5f,%5f) - (%5f,%5f) - (%5f,%5f)\n", m_pos_x.x0,
  //	  m_pos_y.x0, m_pos_x.x1, m_pos_y.x1, m_pos_x.x2, m_pos_y.x2);
}

// Compute the position of the object at current time.
Point2d Physics::ComputeNextXY(double delta_t){
  if (m_motion_type == FreeFall)
    ComputeFallNextXY(delta_t);

  if (m_motion_type == Pendulum)
    ComputePendulumNextXY(delta_t);

  m_last_move = Time::GetInstance()->Read() ;

  return Point2d(m_pos_x.x0, m_pos_y.x0);
}

void Physics::RunPhysicalEngine()
{
  double step_t, delta_t = (Time::GetInstance()->Read() - m_last_move) / 1000.0;
  Point2d oldPos;
  Point2d newPos;

  step_t = PHYS_DELTA_T;

  //  printf ("Delta_t = %f (last %f - current %f)\n", delta_t, m_last_move/1000.0,
  //	  global_time.Read()/1000.0);

  // Compute object move for each physical engine time step.

  while (delta_t > 0.0){
    if (delta_t < PHYS_DELTA_T)
      step_t = delta_t ;

    oldPos = GetPos();

    newPos = ComputeNextXY(step_t);

    if( newPos != oldPos)  {
      // The object has moved. Notify the son class.
      MSG_DEBUG( "physic.move", "Move %s (%f, %f) -> (%f, %f)", typeid(*this).name(), oldPos.x, oldPos.y, newPos.x, newPos.y);
      NotifyMove(oldPos, newPos);
    }

    delta_t -= PHYS_DELTA_T ;
  }

  return;
}

/* contact_angle is the angle of the surface we are rebounding on */
void Physics::Rebound(Point2d contactPos, double contact_angle)
{
  double norme, angle;

  // Get norm and angle of the object speed vector.
  GetSpeed(norme, angle);

  switch (m_motion_type) {
  case FreeFall :
    if (m_rebounding)
    {
      // Compute rebound angle.
      /* if no tangent rebound in the opposit direction */
      if(contact_angle == NAN)
	angle = angle + M_PI ;
      else
	angle =  M_PI - angle -2.0 *  contact_angle;

      // Apply rebound factor to the object speed.
      norme = norme * m_rebound_factor;

      // Apply the new speed to the object.
      SetSpeed(norme, angle);

      // Check if we should stop rebounding.
      if (norme < STOP_REBOUND_LIMIT){
	StopMoving();
	return;
      }
      SignalRebound();
    }
    else
      StopMoving();
    break;

  case Pendulum:
    {
      Point2d V ;

      // Recompute new angle.
      V.x = m_pos_x.x0 + m_fix_point_dxy.x - m_fix_point_gnd.x;
      V.y = m_pos_y.x0 + m_fix_point_dxy.y - m_fix_point_gnd.y;

      m_rope_angle.x0 = M_PI_2 - V.ComputeAngle();

      // Convert the linear speed of the rebound to angular speed.
      V.x = PENDULUM_REBOUND_FACTOR * norme * cos(angle);
      V.y = PENDULUM_REBOUND_FACTOR * norme * sin(angle);

      angle = angle + M_PI;

      m_rope_angle.x1 = (norme * cos(angle) * cos(m_rope_angle.x0) +
			 norme * sin(angle) * sin(m_rope_angle.x0) ) / m_rope_length.x0;

      m_rope_angle.x2 = 0;
      m_extern_force.Clear();
    }
    break ;

  default:
    break ;
  }

}

void Physics::SignalGhostState(bool)  {}
void Physics::SignalDeath() {}
void Physics::SignalDrowning() {}
void Physics::SignalRebound() {}
