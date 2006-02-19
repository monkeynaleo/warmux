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

#include "physical_obj.h"
#include <stdlib.h>
#include <iostream>
#include "physics.h"
#include "../game/config.h"
#include "../game/time.h"
#include "../map/map.h"
#include "../team/teams_list.h"
#include "../tool/debug.h"
#include "../tool/math_tools.h"
#include "../tool/point.h"
#include "../tool/rectangle.h"
#include "../weapon/ninja_rope.h"

const int Y_OBJET_MIN = -10000;
const int WATER_RESIST_FACTOR = 6 ;

const double PIXEL_PER_METER = 40 ;

double MeterDistance (const Point2i &p1, const Point2i &p2)
{
  return p1.Distance(p2) / PIXEL_PER_METER ;
}

PhysicalObj::PhysicalObj (GameLoop &p_game_loop, const std::string &name, double mass) :
  Physics(p_game_loop),
  m_name(name),
  m_width(0),
  m_height(0)
{
  m_type = objCLASSIC;
//  m_moving = false;
  
  m_allow_negative_y = false;
  m_alive = ALIVE;

  m_rebound_sound = "";
  
  m_test_left = 0;
  m_test_right = 0;
  m_test_top = 0;
  m_test_bottom = 0;

  m_ready = true;
}

PhysicalObj::~PhysicalObj ()
{}

//---------------------------------------------------------------------------//
//--                         Class Parameters SET/GET                      --//
//---------------------------------------------------------------------------//

void PhysicalObj::SetX (int x){
  SetXY( Point2i(x, GetY()) );
}

void PhysicalObj::SetY (int y){
  SetXY( Point2i(GetX(), y) );
}

void PhysicalObj::SetXY(const Point2i &position){
  if( position != GetPos() )
    world.ToRedrawOnMap( Rectanglei(position, GetSize()) );

  if( IsOutsideWorldXY( position ) )
      Ghost();
  else{
	  Point2d physPos(position.x, position.y);
	  
      SetPhysXY( physPos / PIXEL_PER_METER );
      if( FootsInVacuum() )
		  StartMoving();
  }
}

const Point2i PhysicalObj::GetPos() const 
{ 
  return Point2i(GetX(), GetY()); 
}

int PhysicalObj::GetX() const
{
  return (int)round(GetPhysX() * PIXEL_PER_METER);
}

int PhysicalObj::GetY() const
{
  return (int)round(GetPhysY() * PIXEL_PER_METER);
}

int PhysicalObj::GetCenterX() const 
{ 
  return GetX() +m_test_left +GetTestWidth()/2;
}

int PhysicalObj::GetCenterY() const 
{ 
  return GetY() +m_test_top +GetTestHeight()/2;
}

const Point2i PhysicalObj::GetCenter() const 
{
  return Point2i(GetCenterX(), GetCenterY());
}

void PhysicalObj::SetSize (uint width, uint height){ 
  assert (width != 0);
  assert (height != 0);
  m_width = width; 
  m_height = height; 
  SetPhysSize (width / PIXEL_PER_METER, height / PIXEL_PER_METER);
}

void PhysicalObj::SetSize(const Point2i &newSize){
  SetSize(newSize.x, newSize.y);
}

// Lit les dimensions de l'objet
int PhysicalObj::GetWidth() const{ 
  assert (m_width != 0); 
  return m_width; 
}

int PhysicalObj::GetHeight() const{
  assert (m_height != 0); 
  return m_height; 
}

Point2i PhysicalObj::GetSize() const{
	return Point2i(m_width, m_height);
}

void PhysicalObj::SetTestRect (uint left, uint right, uint top, uint bottom)
{
  m_test_left =  left;
  m_test_right = right;
  m_test_top = top;
  m_test_bottom = bottom;
}

int PhysicalObj::GetTestWidth() const 
{ 
  return m_width -m_test_left -m_test_right; 
}

int PhysicalObj::GetTestHeight() const 
{ 
  return m_height -m_test_top -m_test_bottom; 
}

const Rectanglei PhysicalObj::GetRect() const 
{ 
  return Rectanglei( GetX(), GetY(), m_width, m_height);
}

const Rectanglei PhysicalObj::GetTestRect() const 
{ 
  return Rectanglei( GetX()+m_test_left,       
		     GetY()+m_test_top, 
		     m_width-m_test_right-m_test_left, 
		     m_height-m_test_bottom-m_test_top);  
}

// Move to a point with collision test
// Return true if collision occured
bool PhysicalObj::NotifyMove(double old_x, double old_y,
			     double new_x, double new_y,
			     double &contact_x, double &contact_y,
			     double &contact_angle)
{
  if(IsGhost())
    return false;

  double x,y,dx,dy;
  int tmp_x, tmp_y;
  int cx, cy;
  bool collision = false ;

  // Convert meters to pixels.

  old_x *= PIXEL_PER_METER;
  old_y *= PIXEL_PER_METER;
  new_x *= PIXEL_PER_METER;
  new_y *= PIXEL_PER_METER;

  // Compute distance between old and new position.

  double lg = Point2d(old_x, old_y).Distance( Point2d(new_x, new_y));

  if (lg == 0)
    return false ;

  // Compute increments to move the object step by step from the old
  // to the new position.

  dx = (new_x - old_x) / lg;
  dy = (new_y - old_y) / lg;

  // First iteration position.

  x = old_x + dx;
  y = old_y + dy;

  //  printf ("--- PhysicalObj::NotifyMove from %f,%f to %f,%f\n",
  //  	  old_x, old_y, new_x, new_y);

  //  printf ("--- PhysicalObj::MoveCollisionTest dx = %f - dy = %f - lg = %f\n",
  //	  dx, dy, lg);

  if (m_type == objUNBREAKABLE || IsInWater())
    return false ;

  do
  {
	Point2i tmpPos( (int)round(x), (int)round(y) );
    
    // Check if we exit the world. If so, we stop moving and return.
    if( IsOutsideWorldXY(tmpPos) ){
		if( !config.exterieur_monde_vide ){
	    	tmpPos.x = BorneLong(tmpPos.x, 0, world.GetWidth() - GetWidth() - 1);
		    tmpPos.y = BorneLong(tmpPos.y, 0, world.GetHeight() - GetHeight() - 1);
			
            MSG_DEBUG( "physic.state", "DeplaceTestCollision touche un bord : %d, %d", tmp_x, tmp_y );
	  	}

		SetXY( tmpPos );
		break;
    }

    // Test if we collide something...
    if( CollisionTest(tmpPos) ){
		MSG_DEBUG( "physic.state", "DeplaceTestCollision: collision par TestCollision." );

		// Set the object position to the current position.
		SetXY( Point2i( (int)round(x - dx), (int)round(y - dy)) );

		// Find the contact point and collision angle.
		// !!! ContactPoint(...) _can_ return false when CollisionTest(...) is true !!!
		// !!! WeaponProjectiles collide on objects, so computing the tangeante to the ground leads
		// !!! uninitialised values of cx and cy!!
		if( ContactPoint(cx, cy) ){
			contact_angle = world.ground.Tangeante(cx, cy);
			contact_x = (double)cx / PIXEL_PER_METER ;
			contact_y = (double)cy / PIXEL_PER_METER ;
		}else{
			contact_angle = - GetSpeedAngle();
			contact_x = x;
			contact_y = y;
		}

		collision = true;
		break;
    }
    
	// Next motion step
    x += dx;
    y += dy;
    lg -= 1.0 ;    
  }while (0 < lg);


  if (ActiveTeam().GetWeaponType() == WEAPON_NINJA_ROPE &&
      ActiveTeam().GetWeapon().IsActive()) {
    Weapon& tmp = ActiveTeam().AccessWeapon();
    NinjaRope * ninjarope = (NinjaRope *)(&tmp);
    ninjarope->NotifyMove(collision) ;
  }
   
  return collision;
}

void PhysicalObj::UpdatePosition ()
{
  // No ghost allowed here !
  if (IsGhost()) return; 

  if ((m_type == objCLASSIC) && !IsMoving()
      && !FootsInVacuum() && !IsInWater()) return;

  if(!IsMoving() && FootsInVacuum() && m_type != objUNBREAKABLE) StartMoving();

  // Compute new position.
  RunPhysicalEngine();

  if (IsGhost()) return;

  if (IsInWater() && (m_alive != DROWNED) && m_type != objUNBREAKABLE) Drown();

  if (m_type == objUNBREAKABLE || IsInWater())
  {
    if( IsOutsideWorldXY(GetPos()) )
      Ghost();

    return;
  }
}

bool PhysicalObj::PutOutOfGround(double direction)
{
  const int max_step = 30;

  if( IsInVacuum(Point2i(0, 0)) )
    return true;

  double dx = cos(direction);
  double dy = sin(direction);

  int step=1;
  while(step<max_step && !IsInVacuum( 
			  Point2i((int)(dx * (double)step),(int)(dy * (double)step)) ))
    step++;

  if(step<max_step)
    SetXY( Point2i((int)(dx * (double)step)+GetX(),(int)(dy * (double)step)+GetY()) );
  else
    return false; //Can't put the object out of the ground

  return true;
}

bool PhysicalObj::PutOutOfGround()
{
  if( IsInVacuum(Point2i(0, 0)) )
    return true;

  bool left,right,top,bottom;
  left   = world.IsInVacuum_left(*this, 0, 0);
  right  = world.IsInVacuum_right(*this, 0, 0);
  top    = world.EstDansVide_haut(*this, 0, 0);
  bottom = world.EstDansVide_bas(*this, 0, 0);

  int dx = (int)GetTestRect().GetSizeX() * (right-left);
  int dy = (int)GetTestRect().GetSizeY() * (top-bottom);

  if( dx == 0 && dy == 0 )
    return false; //->Don't know in which direction we should go...

  Point2i b(dx, dy);

  double dir = b.ComputeAngle();
  return PutOutOfGround(dir);
}

void PhysicalObj::Ready()
{
  if (m_alive != ALIVE)
	MSG_DEBUG( "physic.state", "Ready.");
  m_alive = ALIVE;
  StopMoving();
}


void PhysicalObj::Die()
{
  assert (m_alive == ALIVE || m_alive == DROWNED);
  
  MSG_DEBUG( "physic.state", "Is dying..");
  
  m_alive = DEAD;
  if (m_alive != DROWNED)
	SignalDeath();
}

void PhysicalObj::Ghost ()
{
  if (m_alive == GHOST)
	return;

  bool was_dead = IsDead(); 
  m_alive = GHOST;
  MSG_DEBUG("physic.state", "Ghost, was_dead = %d", was_dead);

  // L'objet devient un fantome
  m_pos_y.x1 = 0.0 ;
  StopMoving();

  SignalGhostState(was_dead);
}

void PhysicalObj::Drown()
{
  assert (m_alive != DROWNED);
  MSG_DEBUG("physic.state", "Drowned...");
  m_alive = DROWNED;

  // Set the air grab to water resist factor.
  m_air_resist_factor = WATER_RESIST_FACTOR ;

  StartMoving();
  SignalDrowning();
}

bool PhysicalObj::IsReady() const
{
  return (!IsMoving() && !FootsInVacuum() && m_ready)||(m_alive == GHOST);
}

bool PhysicalObj::IsDead () const
{ return ((m_alive == GHOST) || (m_alive == DROWNED) || (m_alive == DEAD)); }

bool PhysicalObj::IsGhost() const 
{ return (m_alive == GHOST); }

bool PhysicalObj::IsDrowned() const 
{ return (m_alive == DROWNED); }

void PhysicalObj::SignalRebound()
{
  // TO CLEAN...
   if (!m_rebound_sound.empty())
     jukebox.Play("share", m_rebound_sound) ;
}


bool PhysicalObj::IsOutsideWorldXY(Point2i position) const{
  int x = position.x + m_test_left;
  int y = position.y + m_test_top;

  if( world.EstHorsMondeXlarg(x, GetTestWidth()) )
	  return true;
  if( world.EstHorsMondeYhaut(y, GetTestHeight()) ){
    if( m_allow_negative_y )
      if( (Y_OBJET_MIN <= y) && (y + GetTestHeight() - 1 < 0) )
		  return false;
    return true;
  }
  return false;
}    

bool PhysicalObj::IsOutsideWorld(const Point2i &offset) const{
	return IsOutsideWorldXY( GetPos() + offset );
}

bool PhysicalObj::FootsOnFloor(int y) const
{
  // If outside is empty, the object can't hit the ground !
  if (config.exterieur_monde_vide) return false;

  const int y_max = world.GetHeight()-m_height +m_test_bottom;
  return (y_max <= y);
}

bool PhysicalObj::IsInVacuum(const Point2i &offset) const{
  return IsInVacuumXY(GetPos() + offset);
}

bool PhysicalObj::IsInVacuumXY(const Point2i &position) const{
  if( IsOutsideWorldXY(position) )
	  return config.exterieur_monde_vide;
  
  if( FootsOnFloor(position.y - 1) )
	  return false;

  Rectanglei rect(position.x + m_test_left, position.y + m_test_top,
		  m_width - m_test_right - m_test_left, m_height -m_test_bottom - m_test_top);
  
  return world.RectEstDansVide (rect);
}

bool PhysicalObj::FootsInVacuum() const{
  return FootsInVacuumXY(GetPos());
}

bool PhysicalObj::FootsInVacuumXY(const Point2i &position) const
{
  if( IsOutsideWorldXY(position) ){
	std::cout << "physicalobk.cpp:597: physobj is outside the world" << std::endl;
	return config.exterieur_monde_vide;
  }
   
  if( FootsOnFloor(position.y) ){
     std::cout << "physobj is on floor" << std::endl; 
     return false;
  }
   
  int y_test = position.y + m_height - m_test_bottom;

  Rectanglei rect( position.x + m_test_left, y_test,
		 m_width - m_test_right - m_test_left, 1);
  if( m_allow_negative_y && rect.GetPositionY() < 0){
	  int b = rect.GetPositionY() + rect.GetSizeY();
	  
	  rect.SetPositionY( 0 );
	  rect.SetSizeY( ( b > 0 ) ? b - rect.GetPositionY() : 0 );
  }
   
  return world.RectEstDansVide (rect);
}

bool PhysicalObj::IsInWater () const
{
  assert (!IsGhost());
  if (!world.water.IsActive()) return false;
  int x = BorneLong(GetCenterX(), 0, world.GetWidth()-1);
  return (int)world.water.GetHeight(x) < GetCenterY();
}

bool PhysicalObj::CollisionTest(const Point2i &position){ 
  return !IsInVacuumXY(position);
}

void PhysicalObj::DirectFall()
{
  while (!IsGhost() && !IsInWater() && FootsInVacuum())
      SetY(GetY()+1);
}

bool PhysicalObj::ContactPoint (int & contact_x, int & contact_y)
{
  int x1, x2, y1, y2;

  // On cherche un point de contact en bas de l'objet:
  y1 = (GetY()+m_height-m_test_bottom);
  y2 = y1-1;
  for (uint x=GetX()+ m_test_left; x<=(GetX()+m_width)-m_test_right; x++)
  {
    if(!world.EstHorsMonde(Point2i(x,y1)) && !world.EstHorsMonde(Point2i(x,y2))
    && world.ground.EstDansVide(x,y2) && !world.ground.EstDansVide(x,y1))
    {
      contact_x = x;
      contact_y = GetY() +m_height-m_test_bottom;
      return true;
    }
  }
  
  // On cherche un point de contact � gauche de l'objet:
  x1 = GetX()+m_test_left;
  x2 = x1+1;
  for(uint y=GetY()+m_test_top;y<=GetY()+m_height-m_test_bottom;y++)
  {
    if(!world.EstHorsMonde(Point2i(x1,y)) && !world.EstHorsMonde(Point2i(x2,y))
    && !world.ground.EstDansVide(x1,y) &&  world.ground.EstDansVide(x2,y))
    {
      contact_x = GetX() +m_test_left;
      contact_y = y;
      return true;
    }
  }

  // On cherche un point de contact � droite de l'objet:
  x1 = (GetX()+m_width-m_test_right);
  x2 = x1-1;
  for(uint y=GetY()+m_test_top;y<=GetY()+m_height-m_test_bottom;y++)
  {
    if(!world.EstHorsMonde(Point2i(x1,y)) && !world.EstHorsMonde(Point2i(x2,y))
       && !world.ground.EstDansVide(x1,y) && world.ground.EstDansVide(x2,y))
    {
      contact_x = GetX() + m_width - m_test_right;
      contact_y = y;
      return true;
    }
  }
  
  // On cherche un point de contact en haut de l'objet:
  y1 = GetY()+m_test_top;
  y2 = y1 - 1;
  for(uint x=GetX()+m_test_left;x<=GetX()+m_width-m_test_right;x++)
  {
    if(!world.EstHorsMonde(Point2i(x,y1)) && !world.EstHorsMonde(Point2i(x,y2))
    && !world.ground.EstDansVide(x, y1) && world.ground.EstDansVide(x, y2))
    {
      contact_x =x;
      contact_y = GetY() +m_test_top;
      return true;
    }
  }
  return false;
}

// Est-ce que deux objets se touchent ? (utilise les rectangles de test)
bool ObjTouche (const PhysicalObj &a, const PhysicalObj &b)
{
  return a.GetTestRect().Intersect( b.GetTestRect() );
}

// Est-ce que le point p touche l'objet a ?
bool ObjTouche (const PhysicalObj &a, const Point2i &p)
{
   return  a.GetTestRect().Contains( p );
}

