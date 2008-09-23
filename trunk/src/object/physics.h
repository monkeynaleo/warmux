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

#ifndef PHYSICS_H
#define PHYSICS_H

#include <Box2D.h>

#include "include/base.h"
#include "tool/euler_vector.h"
#include "tool/point.h"
#include "object_cfg.h"
#include <vector>
#include <map>

enum MotionType_t
{
  NoMotion,
  FreeFall,
  Pendulum,
};

typedef enum {
  NO_COLLISION = 0,
  COLLISION_ON_GROUND,
  COLLISION_ON_OBJECT
} collision_t;

class Game;
class Action;
class Force;
class PhysicalShape;

class Physics : private ObjectConfig
{
private:
  MotionType_t m_motion_type ;

 // Point2d m_extern_force;  // External strength applyed to the object
  std::map<unsigned,Force *> m_extern_force_map;
  unsigned m_extern_force_index;
  double m_angle;


protected:
  uint m_last_move;             // Time since last move
  double m_phys_width, m_phys_height;
  Point2d m_fix_point_gnd;   // Rope fixation point to the ground.
  Point2d m_fix_point_dxy;   // Rope delta to fixation point to the object
  EulerVector m_rope_angle;       // Rope angle.
  EulerVector m_rope_length;      // Rope length.
  double m_rope_elasticity;       // The smallest, the more elastic.
  double m_elasticity_damping;    // 0 means perpetual motion.
  double m_balancing_damping;     // 0 means perpetual balancing.
  bool m_is_physical_obj;

  std::vector<b2ContactPoint> added_contact_list;
  std::vector<b2ContactPoint> persist_contact_list;
  std::vector<b2ContactPoint> removed_contact_list;
  std::vector<b2ContactResult> result_contact_list;

  // Define if the rope is elastic or not.
  bool m_elasticity_off ;
  b2BodyDef *m_body_def;
  b2Body *m_body;
  PhysicalShape *m_shape;

  // Other physics constants stored there :
  ObjectConfig m_cfg;
public:
  Physics ();
  virtual ~Physics ();

  // Set/Get position
  void SetPhysXY(double x, double y);
  void SetPhysXY(const Point2d &position);

  b2Body *GetBody() { return m_body;};
  double GetPhysX() const;
  double GetPhysY() const;
  Point2d GetPos() const;

  // Set size
  void SetMass(double mass);
  double GetMass() const { return m_mass; }

  void SetWindFactor (double wind_factor) { m_wind_factor = wind_factor; };
  double GetWindFactor () const { return m_wind_factor; }

  void SetAirResistFactor (double factor) { m_air_resist_factor = factor; };
  double GetAirResistFactor () const{ return m_air_resist_factor; }

  void SetGravityFactor (double factor) { m_gravity_factor = factor; };
  double GetGravityFactor () const { return m_gravity_factor; }

  void SetRebounding (bool rebounding) { m_rebounding = rebounding; }
  bool GetRebounding () const { return m_rebounding; }

  bool IsPhysicalObj() const { return m_is_physical_obj; }

  void AddAddedContactPoint(b2ContactPoint contact);
  void AddPersistContactPoint(b2ContactPoint contact);
  void AddRemovedContactPoint(b2ContactPoint contact);
  void AddContactResult(b2ContactResult contact);

  void ClearContact();


  // Used to sync value across network
  virtual void GetValueFromAction(Action *);
  virtual void StoreValue(Action *);

  // Reset the physics constants (mass, air_resistance...) to the default values in the cfg
  void ResetConstants() { *((ObjectConfig*)this) = m_cfg; };

  // Set initial speed.
  void SetSpeedXY (Point2d vector);
  void SetSpeed (double norm, double angle);
  void SetAngle(double angle);
  // Add a initial speed to the current speed.
  void AddSpeedXY (Point2d vector);
  void AddSpeed (double norm, double angle);

  // Get current object speed
  void GetSpeed (double &norm, double &angle) const;
  Point2d GetSpeedXY () const;
  Point2d GetSpeed() const;
  double GetAngularSpeed() const;
  double GetAngle() const;
  double GetSpeedAngle() const;


  // Add new strength
  uint AddExternForceXY (const Point2d& vector);
  uint AddExternForce (double norm, double angle);
  void RemoveExternForce(unsigned force_index);
  void ImpulseXY(const Point2d& vector);
  void Impulse(double norm, double angle);


  //Point2d GetExternForce() const { return m_extern_force; };

  // Add / Remove a fixation point.
 /* void SetPhysFixationPointXY(double g_x, double g_y,
                              double dx, double dy) ;*/
  void UnsetPhysFixationPoint() ;
 // void ChangePhysRopeSize(double delta) ;

  double GetRopeAngle() const { return m_rope_angle.x0; };
  void SetRopeAngle(double angle) { m_rope_angle.x0 = angle; };

  double GetRopeLength() const { return m_rope_length.x0; };
  void SetRopeLength(double length) { m_rope_length.x0 = length; };

  MotionType_t GetMotionType() const { return m_motion_type; };

  // Physical engine : update position (and state) with current time
  void RunPhysicalEngine();

  void SetBullet(bool is_bullet);
  // Notify the son class that the object has moved.
//  virtual collision_t NotifyMove(Point2d oldPos, Point2d newPos) = 0;

  // Start moving
  void StartMoving();

  // Stop moving
  void StopMoving();

  // The object is moving ?
  bool IsMoving() const;
  // Is this object not moving ?
  virtual bool IsSleeping() const;

  // The object is falling ?
  bool IsFalling() const;

  b2BodyDef *GetBodyDef();

  virtual void SignalRebound();
  virtual void SignalObjectCollision(Physics *,const Point2d&) { };
  virtual void SignalGroundCollision(const Point2d&) { };
  virtual void SignalCollision(const Point2d&) { };


protected:
  // Compute current (x,y) position
  Point2d ComputeNextXY(double delta_t);

  virtual void SignalDeath() { };
  virtual void SignalGhostState (bool) { };
  virtual void SignalDrowning() { };
  virtual void SignalGoingOutOfWater() { };

  // Make the object rebound
  void Rebound(Point2d contactPos, double contact_angle);
private:

  void ComputeFallNextXY (double delta_t);

  void ComputePendulumNextXY (double delta_t);

  void UpdateTimeOfLastMove();
};

#endif
