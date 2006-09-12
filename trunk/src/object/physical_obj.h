/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
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

#ifndef PHYSICAL_OBJECT_H
#define PHYSICAL_OBJECT_H

#include "physics.h"
#include "../tool/point.h"
#include "../tool/rectangle.h"

// Alive state
typedef enum
{
  ALIVE,
  DEAD,
  GHOST,
  DROWNED
} alive_t;

extern const double PIXEL_PER_METER;

double MeterDistance (const Point2i &p1, const Point2i &p2);

class PhysicalObj : public Physics
{
private:
  int m_posx, m_posy;

  // collision management
  bool m_goes_through_wall;
  bool m_collides_with_characters;
  bool m_collides_with_objects;
  PhysicalObj* m_last_colliding_object;
  PhysicalObj* m_overlapping_object;

  void CheckOverlapping();

protected:
  bool exterieur_monde_vide;// TO REMOVE!! It is the same for all physical objects !
  
  std::string m_name;

  // Rectangle used for collision tests
  uint m_test_left, m_test_right, m_test_top, m_test_bottom;

  // Object size and position.
  uint m_width, m_height;

  std::string m_rebound_sound;

  alive_t m_alive;
  int life_points; // Only used by petrol barrel and bonus box (character use their own damage system for now..)

  bool m_allow_negative_y;

public:
  PhysicalObj (const std::string &name, const std::string &xml_config="");
  virtual ~PhysicalObj ();

  //-------- Set position and size -------

  // Set/Get position
  void SetX (int x);
  void SetY (int y);
  void SetXY(const Point2i &position);
  int GetX() const;
  int GetY() const;
  const Point2i GetPosition() const;

  // Set/Get size
  void SetSize(const Point2i &newSize);
  int GetWidth() const;
  int GetHeight() const;
  Point2i GetSize() const;

  // Set/Get test rectangles
  void SetTestRect (uint left, uint right, uint top, uint bottom);
  const Rectanglei GetTestRect() const;   
  int GetTestWidth() const;
  int GetTestHeight() const;

  //----------- Access to datas (read only) ----------
  virtual const std::string &GetName() const { return m_name; }
  int GetCenterX() const;
  int GetCenterY() const;
  const Point2i GetCenter() const;
  const Rectanglei GetRect() const;
  bool GoesThroughWall() const { return m_goes_through_wall; }

  //----------- Physics related function ----------

  // Update position (and state) with current time
  void UpdatePosition();

  // Move the character until he gets out of the ground
  bool PutOutOfGround();
  bool PutOutOfGround(double direction); //Where direction is the angle of the direction
                                         // where the object is moved

  // Collision management
  void SetCollisionModel(bool goes_through_wall,
			 bool collides_with_characters,
			 bool collides_with_objects);
  void SetOverlappingObject(PhysicalObj* obj);

  // Collision test for point (x,y) -- public only for Uzi...
  bool CollisionTest(const Point2i &position);

  PhysicalObj* GetLastCollidingObject() const;
  bool IsInVacuumXY(const Point2i &position);
  bool IsInVacuum(const Point2i &offset); // relative to current position
  bool FootsInVacuumXY(const Point2i &position);
  bool FootsInVacuum();
  
  bool FootsOnFloor(int y) const;

  bool IsInWater() const;

  // The object is outside of the world
  bool IsOutsideWorldXY(Point2i position) const;
  bool IsOutsideWorld(const Point2i &offset) const;

  // Refresh datas
  virtual void Refresh() = 0;

  // Draw the object
  virtual void Draw() = 0;

  // Damage handling
  void AddDamage(uint damage_points);

  //-------- state ----
  void Init();
  void Ghost();
  void Drown();
  void GoOutOfWater(); // usefull for supertux.
  void RemoveFromPhysicalEngine();

  bool IsImmobile() const;
  bool IsDead() const;
  bool IsGhost() const;
  bool IsDrowned() const;

  // Est-ce que deux objets se touchent ? (utilise les rectangles de test)
  bool ObjTouche(const PhysicalObj &b) const;

  // Est-ce que le point p touche l'objet ?
  bool ObjTouche(const Point2i &p) const;

  bool PutRandomly(bool on_top_of_world, double min_dst_with_characters);

protected:
  virtual void SignalRebound();
  virtual void SignalObjectCollision(PhysicalObj * obj);
  virtual void SignalGroundCollision();
  virtual void SignalCollision();

private:
  //Renvoie la position du point de contact entre
  //l'obj et le terrain
  bool ContactPoint (int &x, int &y);

  void NotifyMove(Point2d oldPos, Point2d newPos);

  // The object fall directly to the ground (or become a ghost)
  void DirectFall();
};

#endif
