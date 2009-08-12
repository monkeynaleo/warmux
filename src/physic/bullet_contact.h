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
 * Todo : description
 ******************************************************************************/

#ifndef BULLET_CONTACT_H
#define BULLET_CONTACT_H

#include "physic/physical_contact.h"

class BulletShape;

class BulletContact : public PhysicalContact
{
  
public: 

  BulletContact();

  void Signal();

  // A
  Point2d GetPositionA();
  void SetPositionA(const Point2d &position);
  void SetSpeedXYA(Point2d vector);
  void GetSpeedA(double &norm, double &angle_rad) const;
  Point2d GetSpeedA() const;
  PhysicalShape *GetShapeA();
  BulletShape *GetBulletShapeA();
  void SetShapeA(BulletShape * shape);



  // B
  Point2d GetPositionB();
  void SetPositionB(const Point2d &position);
  void SetSpeedXYB(Point2d vector);
  void GetSpeedB(double &norm, double &angle_rad) const;
  Point2d GetSpeedB() const ;
  PhysicalShape *GetShapeB();
  BulletShape *GetBulletShapeB();
  void SetShapeB(BulletShape * shape);



protected:
  BulletShape * m_shape_A;
  Point2d m_position_A;
  Point2d m_speed_A;



  BulletShape * m_shape_B;
  Point2d m_position_B;
  Point2d m_speed_B;



};

#endif
