/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
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


#ifndef FORCE_H
#define FORCE_H

#include "tool/point.h"
#include <Box2D.h>
class PhysicalObj;
struct b2Vec2;

class Force
{

public:
  Force(PhysicalObj *target, Point2d target_point, Point2d force, bool is_local = true);
  ~Force();
  PhysicalObj *m_target;
  b2Vec2 m_target_point;
  b2Vec2 m_force;

  void ComputeForce();

protected:



};

#endif //FORCE_H
