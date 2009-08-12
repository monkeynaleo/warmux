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
#include "physic/bullet_shape.h"
#include "physic/bullet_contact.h"
#include "physic/bullet_obj.h"

BulletContact::BulletContact():
m_shape_A(NULL),
m_position_A(0,0),
m_speed_A(0,0),
m_shape_B(NULL),
m_position_B(0,0),
m_speed_B(0,0){

}

void BulletContact::Signal(){

  if(m_shape_A){
    m_shape_A->SignalCollision(this);
  }
  if(m_shape_B){
    m_shape_B->SignalCollision(this);
  }
}

// A
Point2d BulletContact::GetPositionA(){
  return m_position_A;
}

void BulletContact::SetPositionA(const Point2d &position){
  m_position_A = position;
}
void BulletContact::SetSpeedXYA(Point2d vector){
  m_speed_A = vector;
}
void BulletContact::GetSpeedA(double &norm, double &angle_rad) const{
  Point2d speed ;
      speed = GetSpeedA();
      norm = speed.Norm();
      angle_rad = speed.ComputeAngle();
}
Point2d BulletContact::GetSpeedA() const{
  return m_speed_A;
}
PhysicalShape *BulletContact::GetShapeA(){
  if(!m_shape_A){
    return NULL;
  }
  return m_shape_A->GetPublicShape();
}


BulletShape *BulletContact::GetBulletShapeA(){
  return m_shape_A;
}
void BulletContact::SetShapeA(BulletShape * shape){
  m_shape_A = shape;
}



  // B
Point2d BulletContact::GetPositionB(){
  return m_position_B;
}

void BulletContact::SetPositionB(const Point2d &position){
  m_position_B = position;
}
void BulletContact::SetSpeedXYB(Point2d vector){
  m_speed_B = vector;
}
void BulletContact::GetSpeedB(double &norm, double &angle_rad) const{
  Point2d speed ;
       speed = GetSpeedB();
       norm = speed.Norm();
       angle_rad = speed.ComputeAngle();
}
Point2d BulletContact::GetSpeedB() const {
  return m_speed_B;
}
PhysicalShape *BulletContact::GetShapeB(){
  if(!m_shape_B){
      return NULL;
    }
  return m_shape_B->GetPublicShape();
}


BulletShape *BulletContact::GetBulletShapeB(){
  return m_shape_B;
}

void BulletContact::SetShapeB(BulletShape * shape){
  m_shape_B = shape;
}
