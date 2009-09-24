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
 * Arrow on top of the active character
 *****************************************************************************/

#ifndef _BULLET_ENGINE_H
#define	_BULLET_ENGINE_H
#include "physical_engine.h"
#include "btBulletDynamicsCommon.h"
#include <vector>

class BulletObj;
class BulletGround;
class BulletContact;

class BulletEngine : public PhysicalEngine
{
    public:


  BulletEngine();
  ~BulletEngine();

  PhysicalGround *CreateGround();
  PhysicalObj *CreateObject(ObjectType type);
  PhysicalPolygon *CreatePolygonShape();
  PhysicalCircle *CreateCircleShape();
  PhysicalRectangle *CreateRectangleShape(double width, double height);

  void AddGround(PhysicalGround *new_obj);
  void AddObject(PhysicalObj *new_obj);
  void RemoveObject(PhysicalObj *obj);
  void RemoveGround(PhysicalGround *obj);

  void AddConstraint();
  void Step();
  void VirtualStep();

  void AddForce(Force * force);
  void RemoveForce(Force *force);

  void AddSpecialObject(BulletObj* i_object);
  void RemoveSpecialObject(BulletObj* i_object);

  void SetWindVector(const Point2d &i_wind_vector);

  static PhysicalEngine *GetInstance();
  static void SetInstance(PhysicalEngine * instance);


  double GetScale() const;
protected:

  static bool ContactAddedCallback(btManifoldPoint& cp,const btCollisionObject* colObj0, int partId0, int index0, const btCollisionObject* colObj1, int partId1, int index1);
  static bool ContactProcessedCallback(btManifoldPoint& cp,void* colObj0, void* colObj1);
  static bool ContactDestroyedCallback(void* userPersistentData);
  void ApplyContacts();

  void CleanGarbage();
  void PerformAddList();

  btDiscreteDynamicsWorld *m_world;
  double m_scale;
  std::vector<Force *> m_force_list;
  std::vector<BulletObj *> m_object_list;
  std::vector<BulletObj *> m_special_object_list;

  std::vector<BulletGround *> m_garbage_list;
  std::vector<BulletGround *> m_add_list;
  std::vector<BulletContact *> m_contact_add_list;
  std::vector<BulletContact *> m_contact_remove_list;
  bool m_is_in_step;
};


#endif	/* _BULLET_ENGINE_H */

