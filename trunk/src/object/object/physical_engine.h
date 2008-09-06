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
#include "include/singleton.h"
#include <Box2D.h>
#include <map>
//-----------------------------------------------------------------------------

class Physics;
//class ContactListener;

class PhysicalEngine : public Singleton<PhysicalEngine>
{
public:

PhysicalEngine();
  ~PhysicalEngine();
  b2Body *AddObject(Physics *new_obj);
  void RemoveObject(Physics *obj);
  void Step();

protected:

  uint frame_rate;
  uint last_step_time;
  uint iterations;
  b2AABB worldAABB;
  b2World *physic_world;

  std::map<b2Body *,Physics *> objects_list;


  friend class Singleton<PhysicalEngine>;



};
/*
 class ContactListener : public b2ContactListener
  {
  public:
    ContactListener(PhysicalEngine *){};

    void Add(const b2ContactPoint* point){};

    void Persist(const b2ContactPoint* point){};


    void Remove(const b2ContactPoint* point){};


      void Result(const b2ContactResult* point){};

  };*/




//-----------------------------------------------------------------------------
#endif
