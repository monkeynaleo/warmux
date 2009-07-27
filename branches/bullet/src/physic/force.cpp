/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2009 Wormux Team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either charactersion 2 of the License, or
 *  (at your option) any later charactersion.
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
 * A force.
 *****************************************************************************/

#include "physic/force.h"
#include "physic/physical_obj.h"
#include "physic/physical_engine.h"
#include "WORMUX_error.h"

Force::Force(PhysicalObj *target, Point2d /*target_point*/, Point2d force, bool is_local):
m_target_center(false)
{
  ASSERT(target);
  m_target = target;

  m_force = force;

  //m_target_point = b2Vec2(target_point.x/PIXEL_PER_METER, target_point.y /PIXEL_PER_METER);


  if(is_local)  {
//   m_target_point =  target->GetBody()->GetWorldPoint(m_target_point);
  }

  PhysicalEngine::GetInstance()->AddForce(this);

}

Force::Force(PhysicalObj *i_target, Point2d i_force):
m_target_center(true)
{
  ASSERT(i_target);
  m_target = i_target;
  m_force = i_force;

  PhysicalEngine::GetInstance()->AddForce(this);

}

Force::~Force()
{
  PhysicalEngine::GetInstance()->RemoveForce(this);
}


void Force::ComputeForce()
{
 //m_body->applyCentralForce(btVector3(vector.x,vector.y,0));

  if(!m_target_center)
  {
//    m_target->GetBody()->ApplyForce(m_force, m_target_point);
  }
  else
  {
 //   m_target->GetBody()->ApplyForce(m_force, m_target->GetBody()->GetWorldCenter());
  }

}
