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
 * Bullet shape
 *****************************************************************************/
#include "physic/bullet_shape.h"
#include "graphic/video.h"
#include "map/camera.h"
#include "physic/bullet_engine.h"
#include "physic/bullet_contact.h"
#include "physic/bullet_obj.h"

BulletShape::BulletShape():
m_contact_count(0),
m_last_contact_count(0)
{
  m_bullet_parent = NULL;
  m_native_shape = NULL;
  m_bullet_position = Point2d(0,0);
}

BulletShape::~BulletShape(){
  std::vector<BulletContact *>::const_iterator it;
  for(it = m_contact_list.begin(); it != m_contact_list.end(); it++ ){
    BulletContact *c = *it;
    if(c->GetBulletShapeA() == this){
      c->SetShapeA(this);
    }

    if(c->GetBulletShapeB() == this){
      c->SetShapeB(this);
    }

  }
}

#include <iostream>

void BulletShape::SetBulletParent(BulletObj *parent)
{
 m_bullet_parent = parent;
}

BulletObj *BulletShape::GetBulletParent()
{
 return m_bullet_parent;
}

bool BulletShape::IsColliding(const PhysicalObj *obj) const
{
  std::vector<BulletContact *>::const_iterator it;
    for(it = m_contact_list.begin(); it != m_contact_list.end(); it++ ){
      BulletContact *c = *it;
      if(c->GetShapeA() && c->GetShapeA()->GetParent() == obj){
        return true;
      }
      if(c->GetShapeB() && c->GetShapeB()->GetParent() == obj){
              return true;
      }
    }
    return false;
}


void BulletShape::SignalCollision(BulletContact * contact){
   GetBulletParent()->SignalCollision(contact);
}

PhysicalShape *BulletShape::GetPublicShape(){
  return m_public_shape;
}

bool BulletShape::AddContact(BulletContact *contact)
{
  bool exist = false;
  BulletShape *collider = NULL;
  if(contact->GetBulletShapeA() == this){
    collider = contact->GetBulletShapeB();
  }else{
    collider = contact->GetBulletShapeA();
  }

  std::vector<BulletContact *>::iterator it;
  for(it = m_contact_list.begin(); it != m_contact_list.end(); it++ ){
   BulletContact *c = *it;
   if(c != contact){
     if(c->GetBulletShapeA() == collider || c->GetBulletShapeB() == collider){
       exist = true;
       break;
     }
   }else{
     exist = true;
     break;
   }
  }

  if(!exist){
    m_contact_list.push_back(contact);
    m_contact_count++;
  }
  return !exist;
}

void BulletShape::RemoveContact(BulletContact *contact)
{
  std::vector<BulletContact *>::iterator it;
   for(it = m_contact_list.begin(); it != m_contact_list.end(); it++ ){
     BulletContact *c = *it;
     if(c == contact){
       m_contact_list.erase(it);
       break;
     }
   }
   m_contact_count--;

}

Point2d BulletShape::GetBulletPosition()
{
  return m_bullet_position;
}

//////////////Bullet Rectangle
BulletRectangle::BulletRectangle(double width, double height):PhysicalRectangle(width,height)
{

}

BulletRectangle::~BulletRectangle()
{

}

void BulletRectangle::Generate()
{
  m_public_shape = this;
  btBoxShape * new_shape = new btBoxShape(btVector3(m_width/(2*GetScale()),m_height/(2*GetScale()),100/(2*GetScale())));

  btScalar mass(1.0f);
  btVector3 localInertia(0, 0, 0);
  m_bullet_position = Point2d(m_width/2, m_height/2);

  new_shape->calculateLocalInertia(mass,localInertia);

  if(m_native_shape)
  {
    delete m_native_shape;
  }

  m_native_shape = new_shape;
  m_native_shape->setUserPointer(dynamic_cast<BulletShape *>(this));
}
 double BulletRectangle::Area() const
 {
   return m_width*m_height;
 }
  double BulletRectangle::GetCurrentWidth() const
  {

   /*  m_shape.getAabb   ( const btTransform &      t,
         btVector3 &     aabbMin,
         btVector3 &     aabbMax
 )    */
    return m_width;
  }

  double BulletRectangle::GetCurrentHeight() const
  {
    return m_height;
  }
  double BulletRectangle::GetInitialWidth() const
  {
    return m_width;
  }
  double BulletRectangle::GetInitialHeight() const
  {
    return m_height;
  }

  double BulletRectangle::GetCurrentMinX() const
  {
    return m_position.x;
  }
  double BulletRectangle::GetCurrentMaxX() const
  {
    return m_position.x+m_width;

  }
  double BulletRectangle::GetCurrentMinY() const
  {
    return m_position.y;
  }
  double BulletRectangle::GetCurrentMaxY() const
  {
    return m_position.y+m_height;
  }

  double BulletRectangle::GetInitialMinX() const
  {
    return m_position.x;
  }
  double BulletRectangle::GetInitialMaxX() const
  {
    return m_position.x+m_width;
  }
  double BulletRectangle::GetInitialMinY() const
  {
    return m_position.y;
  }
  double BulletRectangle::GetInitialMaxY() const
  {
    return m_position.y+m_height;
  }


#ifdef DEBUG
void BulletRectangle::DrawBorder(const Color& color) const
{

  ASSERT(m_parent);

    GetMainWindow().LineColor(m_parent->GetPosition().x-1 - Camera::GetInstance()->GetPosition().x,m_parent->GetPosition().x+1 - Camera::GetInstance()->GetPosition().x, m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y,m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y,Color(0,0,0,255));
    GetMainWindow().LineColor(m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x,m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x, m_parent->GetPosition().y-1 - Camera::GetInstance()->GetPosition().y,m_parent->GetPosition().y+1 - Camera::GetInstance()->GetPosition().y,Color(0,0,0,255));

    GetMainWindow().LineColor(
        m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x,
        m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x + m_width,
        m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y,
        m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y,
        color);

    GetMainWindow().LineColor(
        m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x ,
        m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x + m_width,
        m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y + m_height,
        m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y + m_height,
        color);

    GetMainWindow().LineColor(
        m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x ,
        m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x,
        m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y + m_height,
        m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y,
        color);

    GetMainWindow().LineColor(
           m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x + m_width,
           m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x + m_width,
           m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y + m_height,
           m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y,
           color);

    GetMainWindow().LineColor(m_parent->GetPosition().x-1 - Camera::GetInstance()->GetPosition().x,m_parent->GetPosition().x+1 - Camera::GetInstance()->GetPosition().x, m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y,m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y,Color(0,0,0,255));
    GetMainWindow().LineColor(m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x,m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x, m_parent->GetPosition().y-1 - Camera::GetInstance()->GetPosition().y,m_parent->GetPosition().y+1 - Camera::GetInstance()->GetPosition().y,Color(0,0,0,255));

    for(uint i = 0 ; i< m_contact_list.size() ; i++){
          GetMainWindow().PointColor(m_contact_list[i]->GetPositionA() - Camera::GetInstance()->GetPosition(),Color(255,0,255,255));
          GetMainWindow().PointColor(m_contact_list[i]->GetPositionB() - Camera::GetInstance()->GetPosition(),Color(255,0,255,255));
        }

    /*  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

  ASSERT(polygon->GetVertexCount() > 2);
  b2XForm xf = m_body->GetXForm();
  int init_x = lround(( b2Mul(xf,polygon->GetVertices()[0]).x)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x;
  int init_y = lround(( b2Mul(xf,polygon->GetVertices()[0]).y)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y;
  int prev_x = init_x;
  int prev_y = init_y;
  int x, y;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    x = lround(( b2Mul(xf,polygon->GetVertices()[i]).x)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x;
    y = lround(( b2Mul(xf,polygon->GetVertices()[i]).y)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y;

    GetMainWindow().LineColor(prev_x, x, prev_y, y, color);
    prev_x = x;
    prev_y = y;
  }

  GetMainWindow().LineColor(prev_x, init_x, prev_y, init_y, color);*/
}
#endif


  BulletPolygon::~BulletPolygon()
  {
      delete m_native_shape;
  }

  void BulletPolygon::Generate()
 {
    m_public_shape = this;
      btConvexHullShape * new_shape = new btConvexHullShape();
      new_shape->setMargin(0.0001);

      for(uint i=0;i < m_point_list.size(); i++)
      {
          new_shape->addPoint(btVector3(m_point_list[i].x/GetScale(),m_point_list[i].y/GetScale(),-50/GetScale()));
          new_shape->addPoint(btVector3(m_point_list[i].x/GetScale(),m_point_list[i].y/GetScale(),50/GetScale()));

      }

      if(m_native_shape)
      {
        delete m_native_shape;
      }

      m_native_shape = new_shape;
      m_native_shape->setUserPointer(dynamic_cast<BulletShape *>(this));

     // btCollisionShape* new_shape = new btBoxShape(btVector3(100,100,100));
     // m_native_shape = new_shape;
 }

#ifdef DEBUG

#include <iostream>
void BulletPolygon::DrawBorder(const Color& color) const
{
  ASSERT(m_parent);

  std::vector<Point2d> rotated_point_list;
  if(m_bullet_parent && m_bullet_parent->GetAngle() != 0){
    for (uint i = 0; i < m_point_list.size(); i++) {
      Point2d point;
      point.x = (m_point_list[i].x+GetPosition().x)* cos(m_bullet_parent->GetAngle()) + (m_point_list[i].y+GetPosition().y)* sin(m_bullet_parent->GetAngle());
      point.y = -( (m_point_list[i].y+GetPosition().y)* cos(m_bullet_parent->GetAngle()) - (m_point_list[i].x+GetPosition().x)* sin(m_bullet_parent->GetAngle()));
      rotated_point_list.push_back(point);
    }

  }else{
    rotated_point_list = m_point_list;
  }


  ASSERT(m_point_list.size() > 2);


  int init_x = lround(rotated_point_list[0].x+m_parent->GetPosition().x- Camera::GetInstance()->GetPosition().x);
  int init_y = lround(rotated_point_list[0].y+m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y);
  int prev_x = init_x;
  int prev_y = init_y;
  int x;
  int y;

  for(uint i = 1; i < rotated_point_list.size();i++){

        x = lround(rotated_point_list[i].x+m_parent->GetPosition().x- Camera::GetInstance()->GetPosition().x);
        y =  lround(rotated_point_list[i].y+m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y);

       GetMainWindow().LineColor(prev_x, x, prev_y, y, color);
       prev_x = x;
       prev_y = y;
  }
  GetMainWindow().LineColor(prev_x, init_x, prev_y, init_y, color);

  //Draw origine point
  GetMainWindow().PointColor(m_parent->GetPosition() - Camera::GetInstance()->GetPosition(),Color(0,0,0,255));

  for(uint i = 0 ; i< m_contact_list.size() ; i++){
        GetMainWindow().PointColor(m_contact_list[i]->GetPositionA() - Camera::GetInstance()->GetPosition(),Color(255,0,255,255));
        GetMainWindow().PointColor(m_contact_list[i]->GetPositionB() - Camera::GetInstance()->GetPosition(),Color(255,0,255,255));
      }
}
#endif

///////////
  //Bullet Circle


  BulletCircle::~BulletCircle()
  {
      delete m_native_shape;
  }

  void BulletCircle::Generate()
  {

    m_public_shape = this;
    btCylinderShapeZ * new_shape = new btCylinderShapeZ(btVector3(m_radius/GetScale(),m_radius/GetScale(),100/GetScale()));

    //new_shape->setLocalScaling(btVector3(m_radius,m_radius,10));
    btVector3 localInertia(0,0,0);
    new_shape->calculateLocalInertia(m_mass,localInertia);

   if(m_native_shape)
   {
     delete m_native_shape;
   }


   m_native_shape = new_shape;
   m_native_shape->setUserPointer(dynamic_cast<BulletShape *>(this));
}

#ifdef DEBUG
void BulletCircle::DrawBorder(const Color& color) const
{
  ASSERT(m_parent);

    GetMainWindow().CircleColor(
        GetPosition().x* cos(m_bullet_parent->GetAngle()) + GetPosition().y* sin(m_bullet_parent->GetAngle())+m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x,
         - GetPosition().x* sin(m_bullet_parent->GetAngle()) + GetPosition().y* cos(m_bullet_parent->GetAngle())+m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y,
        m_radius,
        color);


    GetMainWindow().LineColor(GetPosition().x+m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x, GetPosition().x+m_parent->GetPosition().x - Camera::GetInstance()->GetPosition().x + m_radius * cos(m_bullet_parent->GetAngle()),GetPosition().y+m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y, GetPosition().y+m_parent->GetPosition().y - Camera::GetInstance()->GetPosition().y + m_radius * sin(m_bullet_parent->GetAngle()),color);

    GetMainWindow().PointColor(m_parent->GetPosition() - Camera::GetInstance()->GetPosition(),Color(0,0,0,255));



    for(uint i = 0 ; i< m_contact_list.size() ; i++){
      GetMainWindow().PointColor(m_contact_list[i]->GetPositionA() - Camera::GetInstance()->GetPosition(),Color(255,0,255,255));
      GetMainWindow().PointColor(m_contact_list[i]->GetPositionB() - Camera::GetInstance()->GetPosition(),Color(255,0,255,255));
    }

/*  b2PolygonShape* polygon = (b2PolygonShape*)m_shape;

  ASSERT(polygon->GetVertexCount() > 2);
  b2XForm xf = m_body->GetXForm();
  int init_x = lround(( b2Mul(xf,polygon->GetVertices()[0]).x)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x;
  int init_y = lround(( b2Mul(xf,polygon->GetVertices()[0]).y)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y;
  int prev_x = init_x;
  int prev_y = init_y;
  int x, y;

  for (uint i = 1; i< uint(polygon->GetVertexCount()); i++) {

    x = lround(( b2Mul(xf,polygon->GetVertices()[i]).x)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().x;
    y = lround(( b2Mul(xf,polygon->GetVertices()[i]).y)*PIXEL_PER_METER) - Camera::GetInstance()->GetPosition().y;

    GetMainWindow().LineColor(prev_x, x, prev_y, y, color);
    prev_x = x;
    prev_y = y;
  }

  GetMainWindow().LineColor(prev_x, init_x, prev_y, init_y, color);*/
}
#endif

double BulletShape::GetScale() const
  {
   return (reinterpret_cast<BulletEngine *>(PhysicalEngine::GetInstance()))->GetScale();
  }

//Commun

bool BulletRectangle::IsColliding() const
{
  return (m_contact_count !=0);
}

bool BulletPolygon::IsColliding() const
{
  return (m_contact_count !=0);
}

bool BulletCircle::IsColliding() const
{
  return (m_contact_count !=0);
}


