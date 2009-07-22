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

#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "physical_obj.h"
#include <WORMUX_rectangle.h>
#include "physic/object_cfg.h"


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

class GameObj
{
public :
    enum GameObjType {
        GAME_CHARACTER,
        GAME_FIRE,
        GAME_BULLET,
        GAME_NONE
    };
private:
 

  uint m_last_move;
 
 
  
  GameObjType m_type;
  alive_t m_alive;
  int m_energy;

  
Point2d m_size;
  PhysicalObj *m_physic;
  ObjectConfig m_cfg;
protected:
     std::string m_name;
     std::string m_unique_id;
    bool m_allow_negative_y;
     bool m_ignore_movements;
     std::string m_rebound_sound;


public:
  GameObj (const std::string &name, const std::string &xml_config="");
  /* Note : The copy constructor is not implemented (and this is not a bug)
   * because we can copy directly the pointer m_overlapping_object whereas this
   * object does not own it.
   * FIXME what happen if the object is deleted meanwhile ???*/
  virtual ~GameObj ();

  // Used to sync value across network
  virtual void GetValueFromAction(Action *);
  virtual void StoreValue(Action *);

  GameObjType GetType() { return m_type; }
  void SetType(GameObjType type) { m_type = type; }
  //----------- Access to datas (read only) ----------
  virtual const std::string &GetName() const { return m_name; }
  PhysicalObj *GetPhysic() const;
  void SetPhysic( PhysicalObj *obj);
  const std::string &GetUniqueId() const { return m_unique_id; }
  void SetUniqueId(const std::string& s) { m_unique_id = s; }
  Point2d GetSize() const;
  double GetWidth() const {return GetSize().x;}
  double GetHeight()const {return GetSize().y;}
  const Rectangled GetRect() const;
  const Rectanglei GetRectI() const;
  double GetMinX() const;
  double GetMinY() const;
  double GetMaxX() const;
  double GetMaxY() const;



  // Update position (and state) with current time
  void UpdatePosition();

  // Move the character until he gets out of the ground
  bool PutOutOfGround();
  bool PutOutOfGround(double direction, double max_distance=30); //Where direction is the angle of the direction
                                         // where the object is moved
                                         // and max_distance is max distance allowed when putting out

  bool IsInVacuumXY(const Point2i &position, bool check_objects = true) const;
  // Relative to current position
  bool IsInVacuum(const Point2i &offset, bool check_objects = true) const;

  bool IsInWater() const;
  

//PhysicMapping
  // Set/Get position
  void SetX(double x);
  void SetY(double y);
  void SetPosition(const Point2d &position);
  void SetSafePosition(const Point2d &position);
  double GetX() const;
  double GetY() const;
  const Point2d GetPosition() const ;
  void SetAngle(double angle_rad);
  double GetAngle() const;

  //State
  void SetFixed(bool i_fixed);
  bool IsFixed();
  void SetRotationFixed(bool rotating);
  bool IsRotationFixed() const;
  void StopMovement();
  void SetFast(bool is_fast);
  bool IsFast();

 // Speed
  void SetSpeedXY(Point2d vector);
  void SetSpeed(double norm, double angle_rad);
  void SetAngularSpeed(double speed);
  void AddSpeedXY(Point2d vector);
  void AddSpeed(double norm, double angle_rad);
  void AddAngularSpeed(double speed);
  void GetSpeed(double &norm, double &angle_rad) const;
  Point2d GetSpeed() const;
  double GetAngularSpeed() const;
  double GetSpeedAngle() const;
  virtual bool IsImmobile() const;
  virtual bool IsMoving() const;
  virtual bool IsSleeping() const;
 //  Mass
  double GetMass() const;

  // The object is falling ?
  bool IsFalling() const;

  // The object is outside of the world
  bool IsOutsideWorldXY(const Point2i& position) const;
  // Relative to current position
  bool IsOutsideWorld(const Point2i &offset = Point2i(0,0)) const;

  // Refresh datas
  virtual void Refresh() = 0;

  // Draw the object
  virtual void Draw() = 0;
#ifdef DEBUG
  void DrawShape(const Color& color) const;
#endif

// Energy related
  void SetEnergyDelta(int delta, bool do_report = true);
  void SetEnergy(int new_energy);
  inline const int & GetEnergy() const { return m_energy; };
  //-------- state ----
  void Init();
  virtual void Ghost();
  void Drown();
  void GoOutOfWater(); // usefull for supertux.


  bool IsGhost() const;
  bool IsDrowned() const;
  bool IsDead() const;
  alive_t GetAlive() const { return m_alive; };
  void SetAlive(alive_t alive) { m_alive = alive;}
  bool PutRandomly(bool on_top_of_world, double min_dst_with_characters, bool net_sync = true);
  bool CollidesWithGround();
protected:

  virtual void SignalOutOfMap() { };
  virtual void SignalDeath() { };
  virtual void SignalGhostState (bool) { };
  virtual void SignalDrowning() { };
  virtual void SignalGoingOutOfWater() { };

private:
  void InitShape(const std::string &xml_config);

  // The object fall directly to the ground (or become a ghost)
  void DirectFall();
  void UpdateTimeOfLastMove();
  
};

#endif
