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
 * Arrow on top of the active character
 *****************************************************************************/

#ifndef PHYSICAL_ENGINE_H
#define PHYSICAL_ENGINE_H
//-----------------------------------------------------------------------------
#include "include/base.h"
#include "include/singleton.h"
#include <Box2D.h>
#include <map>
#include <vector>
//-----------------------------------------------------------------------------

extern const double PIXEL_PER_METER;

class PhysicalObj;
class Force;
class ContactListener;
class DebugDraw;

class PhysicalEngine : public Singleton<PhysicalEngine>
{
public:

  PhysicalEngine();
  ~PhysicalEngine();

  b2Body *GetNewGroundBody() ;
  b2Body *AddObject(PhysicalObj *new_obj);
  void RemoveObject(PhysicalObj *obj);
  void Step();
  void StaticStep();

  void AddForce(Force * force);
  void RemoveForce(Force *force);

  typedef enum {ADD,PERSIST,REMOVE} ContactType;

  void AddContactPoint(b2ContactPoint contact,ContactType type);
  void AddContactResult(b2ContactResult contact);

protected:

  uint frame_rate;
  uint last_step_time;
  uint iterations;
  b2AABB worldAABB;
  b2World *physic_world;
  b2Body *ground;
  ContactListener *m_contact_listener;
  DebugDraw *m_debug_draw;

  void ComputeContacts();

  std::map<b2Body *,PhysicalObj *> objects_list;

  std::vector<b2ContactPoint> added_contact_list;
  std::vector<b2ContactPoint> persist_contact_list;
  std::vector<b2ContactPoint> removed_contact_list;
  std::vector<b2ContactResult> result_contact_list;

  std::vector<Force *> m_force_list;
  void ClearContact();

  friend class Singleton<PhysicalEngine>;
};

class ContactListener : public b2ContactListener
{
public:
  ContactListener(PhysicalEngine *);

  virtual void Add(const b2ContactPoint* point);

  virtual void Persist(const b2ContactPoint* point);


  virtual void Remove(const b2ContactPoint* point);


  virtual void Result(const b2ContactResult* point);

protected:

  PhysicalEngine *engine;
};




//-----------------------------------------------------------------------------
#endif
