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

#ifndef PHYSICAL_OBJECT_H
#define PHYSICAL_OBJECT_H

#include <Box2D.h>
#include "tool/point.h"
#include "tool/rectangle.h"
#include <map>
#include <vector>
#include "object/object_cfg.h"

// Alive state
typedef enum
{
  ALIVE,
  DEAD,
  GHOST,
  DROWNED
} alive_t;

class Action;
class Color;
class Force;
class PhysicalShape;

double MeterDistance (const Point2i &p1, const Point2i &p2);

class PhysicalObj : private ObjectConfig
{
private:
  /* If you need this, implement it (correctly)*/
  const PhysicalObj& operator=(const PhysicalObj&);
  /*********************************************/

  // collision management
  bool m_collides_with_ground;
  bool m_collides_with_characters;
  bool m_collides_with_objects;

  // WARNING: MUST BE REMOVED
  // Rectangle used for collision tests
  uint m_test_left, m_test_right, m_test_top, m_test_bottom;

  PhysicalObj* m_overlapping_object;
  uint m_minimum_overlapse_time;
  uint m_nbr_contact;

  // Other physics constants stored there :
  ObjectConfig m_cfg;

  std::map<unsigned,Force *> m_extern_force_map;
  uint m_extern_force_index;

protected:
  bool m_ignore_movements;
  bool m_is_character;
  bool m_is_fire;
  b2Body *m_body;
  b2BodyDef *m_body_def;
  PhysicalShape *m_shape;
  virtual void CheckOverlapping();
  uint m_last_move;
  std::string m_name;
  std::string m_unique_id;
  std::string m_rebound_sound;

  alive_t m_alive;
  int m_energy;

  bool m_allow_negative_y;

  std::vector<b2ContactPoint> added_contact_list;
  std::vector<b2ContactPoint> persist_contact_list;
  std::vector<b2ContactPoint> removed_contact_list;
  std::vector<b2ContactResult> result_contact_list;

public:
  PhysicalObj (const std::string &name, const std::string &xml_config="");
  /* Note : The copy constructor is not implemented (and this is not a bug)
   * because we can copy directly the pointer m_overlapping_object whereas this
   * object does not own it.
   * FIXME what happen if the object is deleted meanwhile ???*/
  virtual ~PhysicalObj ();

  // Used to sync value across network
  virtual void GetValueFromAction(Action *);
  virtual void StoreValue(Action *);

  //-------- Position, speed and size -------

  // Set/Get position
  void SetX(double x) { SetXY( Point2d(x, GetYdouble()) ); };
  void SetY(double y) { SetXY( Point2d(GetXdouble(), y) ); };
  void SetXY(const Point2i &position);
  void SetXY(const Point2d &position);
  int GetX() const;
  int GetY() const;
  double GetXdouble() const;
  double GetYdouble() const;
  const Point2d GetPosition() const { return Point2d(GetXdouble(), GetYdouble()) ;};

  // Set/Get position
  void SetPhysXY(double x, double y);
  void SetPhysXY(const Point2d &position);

  b2Body *GetBody() { return m_body;};


  double GetPhysX() const;
  double GetPhysY() const;
  Point2d GetPhysXY() const;
  Point2d GetPos() const;

  ///////////////////
  // Speed

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

  // Set/Get size
  virtual void SetSize(const Point2i &newSize);
  int GetWidth() const;
  int GetHeight() const;
  Point2i GetSize() const;

  double GetWdouble() const;
  double GetHdouble() const;
  Point2d GetSizeDouble() const;


  // WARNING: MUST BE REMOVED
  // Set/Get test rectangles
  void SetTestRect (uint left, uint right, uint top, uint bottom);
  const Rectanglei GetTestRect() const;
  int GetTestWidth() const;
  int GetTestHeight() const;

  //----------- Access to datas (read only) ----------
  virtual const std::string &GetName() const { return m_name; }

  const std::string &GetUniqueId() const { return m_unique_id; }
  void SetUniqueId(const std::string& s) { m_unique_id = s; }

  int GetCenterX() const { return GetX() +m_test_left +GetTestWidth()/2; };
  int GetCenterY() const { return GetY() +m_test_top +GetTestHeight()/2; };
  const Point2i GetCenter() const { return Point2i(GetCenterX(), GetCenterY()); };
  const Rectanglei GetRect() const { return Rectanglei( GetX(), GetY(), 1, 1); };
  bool CollidesWithGround() const { return m_collides_with_ground; }
  bool IsCharacter() const { return m_is_character; }

  //----------- Physics related function ----------

  void SetMass(double mass);
  double GetMass() const { return m_mass; }

  void SetWindFactor (double wind_factor) { m_wind_factor = wind_factor; };
  double GetWindFactor () const { return m_wind_factor; }

  void SetAirResistFactor (double factor) { m_air_resist_factor = factor; };
  double GetAirResistFactor () const{ return m_air_resist_factor; }

  void SetGravityFactor (double factor) { m_gravity_factor = factor; };
  double GetGravityFactor () const { return m_gravity_factor; }

  void AddAddedContactPoint(b2ContactPoint contact);
  void AddPersistContactPoint(b2ContactPoint contact);
  void AddRemovedContactPoint(b2ContactPoint contact);
  void AddContactResult(b2ContactResult contact);

  void AddContact();
  void RemoveContact();
  void ClearContact();

  void SetBullet(bool is_bullet);

  // Add new strength
  uint AddExternForceXY (const Point2d& vector);
  uint AddExternForce (double nonrm, double angle);
  void RemoveExternForce(unsigned force_index);
  void ImpulseXY(const Point2d& vector);
  void Impulse(double norm, double angle);

  // Update position (and state) with current time
  void UpdatePosition();

  // Move the character until he gets out of the ground
  bool PutOutOfGround();
  bool PutOutOfGround(double direction, double max_distance=30); //Where direction is the angle of the direction
                                         // where the object is moved
                                         // and max_distance is max distance allowed when putting out

  // Collision management
  void SetCollisionModel(bool collides_with_ground,
                         bool collides_with_characters,
                         bool collides_with_objects);
  void SetOverlappingObject(PhysicalObj* obj, int timeout = 0);


  void SetRebounding (bool rebounding) { m_rebounding = rebounding; }
  bool GetRebounding () const { return m_rebounding; }

  const PhysicalObj* GetOverlappingObject() const;
  virtual bool IsOverlapping(const PhysicalObj* obj) const;

  bool IsInVacuumXY(const Point2i &position, bool check_objects = true) const;
  // Relative to current position
  bool IsInVacuum(const Point2i &offset, bool check_objects = true) const;

  PhysicalObj* CollidedObjectXY(const Point2i & position) const;
  // Relative to current position
  PhysicalObj* CollidedObject(const Point2i & offset = Point2i(0,0)) const;

  bool FootsInVacuum() const;

  bool IsInWater() const;


  // The object is falling ?
  bool IsFalling() const;

  b2BodyDef *GetBodyDef();
  // The object is outside of the world
  bool IsOutsideWorldXY(const Point2i& position) const;
  // Relative to current position
  bool IsOutsideWorld(const Point2i &offset = Point2i(0,0)) const;

  // Refresh datas
  virtual void Refresh() = 0;

  // Draw the object
  virtual void Draw() = 0;
#ifdef DEBUG
  void DrawPolygon(const Color& color) const;
#endif

  // Damage handling
  virtual void SetEnergyDelta(int delta, bool do_report = true);

  //-------- state ----
  void Init();
  void Ghost();
  void Drown();
  void GoOutOfWater(); // usefull for supertux.

  // Start moving
  void StartMoving();

  // Stop moving
  void StopMoving();
  virtual bool IsImmobile() const;

  bool IsGhost() const;
  bool IsDrowned() const;
  bool IsDead() const;

  // The object is moving ?
  bool IsMoving() const;
  // Is this object not moving ?
  virtual bool IsSleeping() const;


 // Reset the physics constants (mass, air_resistance...) to the default values in the cfg
  void ResetConstants() { *((ObjectConfig*)this) = m_cfg; };

  // Are the two object in contact ? (uses test rectangles)
  bool Overlapse(const PhysicalObj &b) const;

  // Do the point p touch the object ?
  bool Contain(const Point2i &p) const;

  bool PutRandomly(bool on_top_of_world, double min_dst_with_characters, bool net_sync = true);

  virtual void SignalRebound();
  virtual void SignalObjectCollision(PhysicalObj *,const Point2d&) { };
  virtual void SignalGroundCollision(const Point2d&) { };
  virtual void SignalCollision(const Point2d&) { };
protected:
  virtual void SignalOutOfMap() { };
  virtual void SignalDeath() { };
  virtual void SignalGhostState (bool) { };
  virtual void SignalDrowning() { };
  virtual void SignalGoingOutOfWater() { };
private:

  // The object fall directly to the ground (or become a ghost)
  void DirectFall();
  void UpdateTimeOfLastMove();
};

#endif
