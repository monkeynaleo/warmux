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

#include "../object/physical_obj.h"
#include "../object/physics.h"
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include "../map/map.h"
#include "../game/config.h"
#include "../tool/math_tools.h"
#ifdef CL
# include "../tool/geometry_tools.h"
#else
#include "../tool/Point.h"
#include "../tool/Rectangle.h"
#include "../tool/Distance.h"
#endif
//# include "../game/game_mode.h"
#include "../game/time.h"
//#include "../map/water.h"
#include "../sound/jukebox.h" //TODO remove this
#include "../weapon/ninja_rope.h"
//#include "../map/wind.h"
#include <iostream>
using namespace Wormux;
//-----------------------------------------------------------------------------

#ifdef DEBUG

//#define DEBUG_CHG_ETAT
//#define DEBUG_MSG_PHYSIQUE

#define COUT_DEBUG0 std::cout << "[Objet " << m_name << "]"
#define COUT_DEBUG COUT_DEBUG0 " "
#define COUT_PHYSIC COUT_DEBUG0 << "[Physique] "

#endif
//-----------------------------------------------------------------------------

const int Y_OBJET_MIN = -10000;
const int WATER_RESIST_FACTOR = 6 ;

const double PIXEL_PER_METER = 40 ;

#ifdef CL
double MeterDistance (const CL_Point &p1, const CL_Point &p2)
{
  return Distance(p1, p2) / PIXEL_PER_METER ;
}
#else
double MeterDistance (const Point2i &p1, const Point2i &p2)
{
  return Distance(p1, p2) / PIXEL_PER_METER ;
}
#endif
//-----------------------------------------------------------------------------

PhysicalObj::PhysicalObj (const std::string &name, double mass)
  : m_name(name),
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

//-----------------------------------------------------------------------------

PhysicalObj::~PhysicalObj ()
{}

//-----------------------------------------------------------------------------

//---------------------------------------------------------------------------//
//--                                                                       --//
//--                         Class Parameters SET/GET                      --//
//--                                                                       --//
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------

// Set / Get positions

void PhysicalObj::SetX (int x)
{
  SetXY(x, GetY()) ;
}

void PhysicalObj::SetY (int y)
{
  SetXY(GetX(), y) ;
}

void PhysicalObj::SetXY (int x, int y)
{
  if (IsOutsideWorldXY (static_cast<int>(x), static_cast<int>(y)))
    {
      Ghost();
    }
  else
    {
      SetPhysXY(1.0 * x / PIXEL_PER_METER, 1.0 * y / PIXEL_PER_METER);
      if(FootsInVacuum()) StartMoving();
    }
}

#ifdef CL
const CL_Point PhysicalObj::GetPos() const 
{ 
  return CL_Point(GetX(), GetY()); 
}
#else
const Point2i PhysicalObj::GetPos() const 
{ 
  return Point2i(GetX(), GetY()); 
}
#endif

int PhysicalObj::GetX() const
{
  return Arrondit(GetPhysX() * PIXEL_PER_METER);
}

int PhysicalObj::GetY() const
{
  return Arrondit(GetPhysY() * PIXEL_PER_METER);
}

//-----------------------------------------------------------------------------

int PhysicalObj::GetCenterX() const 
{ 
  return GetX() +m_test_left +GetTestWidth()/2;
}

int PhysicalObj::GetCenterY() const 
{ 
  return GetY() +m_test_top +GetTestHeight()/2;
}

#ifdef CL
const CL_Point PhysicalObj::GetCenter() const 
{
  return CL_Point(GetCenterX(), GetCenterY());
}
#else
const Point2i PhysicalObj::GetCenter() const 
{
  return Point2i(GetCenterX(), GetCenterY());
}
#endif
//-----------------------------------------------------------------------------

void PhysicalObj::SetSize (uint width, uint height) 
{ 
  assert (width != 0);
  assert (height != 0);
  m_width = width; 
  m_height = height; 
  SetPhysSize (width / PIXEL_PER_METER, height / PIXEL_PER_METER);
}

// Lit les dimensions de l'objet
int PhysicalObj::GetWidth() const 
{ 
  assert (m_width != 0); 
  return m_width; 
}

int PhysicalObj::GetHeight() const 
{ 
  assert (m_height != 0); 
  return m_height; 
}

//-----------------------------------------------------------------------------

void PhysicalObj::SetTestRect (uint left, uint right, uint top, uint bottom)
{
  m_test_left =  left;
  m_test_right = right;
  m_test_top = top;
  m_test_bottom = bottom;
}

//-----------------------------------------------------------------------------

int PhysicalObj::GetTestWidth() const 
{ 
  return m_width -m_test_left -m_test_right; 
}

int PhysicalObj::GetTestHeight() const 
{ 
  return m_height -m_test_top -m_test_bottom; 
}

//-----------------------------------------------------------------------------

#ifdef CL
const CL_Rect PhysicalObj::GetRect() const 
{ 
  const int x = GetX();
  const int y = GetY();
  return CL_Rect(x, y, x+m_width, y+m_height);
}
#else
const Rectanglei PhysicalObj::GetRect() const 
{ 
  return Rectanglei( GetX(), GetY(), m_width, m_height);
}
#endif
//-----------------------------------------------------------------------------

#ifdef CL
const CL_Rect PhysicalObj::GetTestRect() const 
{ 
  const int x = GetX();
  const int y = GetY();
  return CL_Rect(
    x +m_test_left,       
    y +m_test_top, 
    x +m_width  -m_test_right, 
    y +m_height -m_test_bottom);  
}
#else
const Rectanglei PhysicalObj::GetTestRect() const 
{ 
  return Rectanglei( GetX()+m_test_left,       
		     GetY()+m_test_top, 
		     m_width-m_test_right-m_test_left, 
		     m_height-m_test_bottom-m_test_top);  
}
#endif

//-----------------------------------------------------------------------------


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

  old_x *= PIXEL_PER_METER ;
  old_y *= PIXEL_PER_METER ;
  new_x *= PIXEL_PER_METER ;
  new_y *= PIXEL_PER_METER ;

  // Compute distance between old and new position.

  double lg = Distance ( Point2d(old_x, old_y), Point2d(new_x, new_y));

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
    tmp_x = Arrondit(x);
    tmp_y = Arrondit(y);

    //    printf ("--- PhysicalObj::MoveCollisionTest - Pos %d,%d\n", tmp_x, tmp_y);
    
    // Check if we exit the world. If so, we stop moving and return.
    if (IsOutsideWorldXY (tmp_x, tmp_y))
      {
	if (!config.exterieur_monde_vide)
	  {
	    tmp_x = BorneLong(tmp_x, 0, monde.GetWidth() -GetWidth() -1);
	    tmp_y = BorneLong(tmp_y, 0, monde.GetHeight() -GetHeight() -1);
#ifdef DEBUG_CHG_ETAT
	    COUT_DEBUG << "DeplaceTestCollision touche un bord :" 
		       << tmp_x << "," << tmp_y
		       << std::endl;
#endif
	  }

	SetXY (tmp_x, tmp_y);

	break ;
      }

    // Test if we collide something...
    if (CollisionTest(tmp_x, tmp_y))
      {
#ifdef DEBUG_CHG_ETAT
	COUT_DEBUG << "DeplaceTestCollision: collision par TestCollision."
		   << std::endl;
#endif
	// Yes ! There is  a collision.

	// Set the object position to the current position.

	SetXY (Arrondit(x-dx), Arrondit(y-dy));

	// Find the contact point and collision angle.
  // !!! ContactPoint(...) _can_ return false when CollisionTest(...) is true !!!
  // !!! WeaponProjectiles collide on objects, so computing the tangeante to the ground leads
  // !!! uninitialised values of cx and cy!!
	if(ContactPoint(cx,cy))
  {
    contact_angle = monde.terrain.Tangeante(cx,cy);
    contact_x = (double)cx / PIXEL_PER_METER ;
    contact_y = (double)cy / PIXEL_PER_METER ;
  }
  else
  {
    contact_angle = - GetSpeedAngle();
    contact_x = x;
    contact_y = y;
  }

	//        printf ("--- Collision !!! - Pos %d,%d\n", tmp_x, tmp_y);
    collision = true;
    break ;

      }
    
    // Next motion step
    x += dx;
    y += dy;
    lg -= 1.0 ;    
  } while (0 < lg);

   if (ninjarope.IsActive())
    ninjarope.NotifyMove(collision) ;
   
  return collision;
}
/*#else
bool PhysicalObj::NotifyMove(double old_x, double old_y,
			     double new_x, double new_y,
			     double &contact_x, double &contact_y,
			     double &contact_angle)
{
    return false;
}
#endif // TODO*/
//-----------------------------------------------------------------------------

void PhysicalObj::UpdatePosition ()
{
  // No ghost allowed here !
  if (IsGhost()) return; 

//  if(!IsMoving() && m_type != objCLASSIC) return;

  if ((m_type == objCLASSIC) && !IsMoving()
      && !FootsInVacuum() && !IsInWater()) return;

  if(!IsMoving() && FootsInVacuum() && m_type != objUNBREAKABLE) StartMoving();

  // Compute new position.

  RunPhysicalEngine();

  if (IsGhost()) return;

  if (IsInWater() && (m_alive != DROWNED) && m_type != objUNBREAKABLE) Drown();

  if (m_type == objUNBREAKABLE || IsInWater())
  {
    if(IsOutsideWorldXY(GetX(),GetY()))
      Ghost();

    return;
  }
}


//-----------------------------------------------------------------------------

void PhysicalObj::Ready()
{
#ifdef DEBUG_CHG_ETAT
  if (m_alive != ALIVE) COUT_DEBUG << "Ready." << std::endl;
#endif
  m_alive = ALIVE;
  StopMoving();
}

//-----------------------------------------------------------------------------

void PhysicalObj::Die()
{
  assert (m_alive == ALIVE || m_alive == DROWNED);
#ifdef DEBUG_CHG_ETAT
  COUT_DEBUG << "Meurt." << std::endl;
#endif
  m_alive = DEAD;
  if (m_alive != DROWNED) SignalDeath();
}

//-----------------------------------------------------------------------------

void PhysicalObj::Ghost ()
{
  if (m_alive == GHOST) return;

  bool was_dead = IsDead(); 
  m_alive = GHOST;
#ifdef DEBUG_CHG_ETAT
  COUT_DEBUG << "Fantome (etait_mort=" << was_dead << ")" << std::endl;
#endif

  // L'objet devient un fantome
  m_pos_y.x1 = 0.0 ;
  StopMoving();

  SignalGhostState(was_dead);
}

//-----------------------------------------------------------------------------

void PhysicalObj::Drown()
{
  assert (m_alive != DROWNED);
#ifdef DEBUG_CHG_ETAT
  COUT_DEBUG << "Se noie !!!" << std::endl;
#endif
  m_alive = DROWNED;

  // Set the air grab to water resist factor.
  m_air_resist_factor = WATER_RESIST_FACTOR ;

#ifdef DEBUG_CHG_ETAT
  COUT_DEBUG << "Se noie speed = " << m_speed.x << "," << m_speed.y << std::endl;
#endif
  StartMoving();
  SignalDrowning();
}

//-----------------------------------------------------------------------------

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
#ifdef CL
   if (!m_rebound_sound.empty())
   jukebox.Play(m_rebound_sound) ;
#else
  // TODO   
#endif
}


//-----------------------------------------------------------------------------

bool PhysicalObj::IsOutsideWorldXY (int x, int y) const
{
  x += m_test_left;
  y += m_test_top;
  if (monde.EstHorsMondeXlarg(x, GetTestWidth())) return true;
  if (monde.EstHorsMondeYhaut(y, GetTestHeight()))
  {
    if (m_allow_negative_y)
    {
      if ((Y_OBJET_MIN <= y) && (y+GetTestHeight()-1 < 0)) return false;
    }
    return true;
  }
  return false;
}    

//-----------------------------------------------------------------------------

bool PhysicalObj::IsOutsideWorld (int dx, int dy) const
{ return IsOutsideWorldXY (GetX()+dx, GetY()+dy); }

//-----------------------------------------------------------------------------

bool PhysicalObj::FootsOnFloor(int y) const
{
  // If outside is empty, the object can't hit the ground !
  if (config.exterieur_monde_vide) return false;

  const int y_max = monde.GetHeight()-m_height +m_test_bottom;
  return (y_max <= y);
}

//-----------------------------------------------------------------------------

bool PhysicalObj::IsInVacuum (int dx, int dy) const
{
  return IsInVacuumXY (GetX()+dx, GetY()+dy);
}

//-----------------------------------------------------------------------------

bool PhysicalObj::IsInVacuumXY (int x, int y) const
{
  if (IsOutsideWorldXY(x,y)) return config.exterieur_monde_vide;
  
  if (FootsOnFloor(y-1)) return false;

#ifdef CL
  CL_Rect rect(
    x +m_test_left,       
	y +m_test_top, 
	x +m_width  -m_test_right, 
	y +m_height -m_test_bottom/*-1*/);
  if (m_allow_negative_y)
  {
    if ((0<=rect.bottom) && (rect.top<0)) rect.top = 0;
  }
  return monde.RectEstDansVide (rect);
#else
  Rectanglei rect( x+m_test_left,y+m_test_top, m_width-m_test_right-m_test_left, m_height-m_test_bottom-m_test_top/*-1*/);
//  if (m_allow_negative_y)
//   {
//    if ((0<=rect.bottom) && (rect.top<0)) rect.top = 0;
//  }
  return monde.RectEstDansVide (rect);
#endif   
}

//-----------------------------------------------------------------------------

bool PhysicalObj::FootsInVacuum() const
{
  return FootsInVacuumXY(GetX(), GetY());
}

//-----------------------------------------------------------------------------

bool PhysicalObj::FootsInVacuumXY(int x, int y) const
{
  if (IsOutsideWorldXY(x,y)) 
     {
	std::cout << "physicalobk.cpp:597: physobj is outside the world" << std::endl;
	return config.exterieur_monde_vide;
     }
   
  if (FootsOnFloor(y)) {
     std::cout << "physobj is on floor" << std::endl; 
     return false;
  }
   
  int y_test = y + m_height - m_test_bottom/*-1*/;

#ifdef CL
  CL_Rect rect(
    x +m_test_left,       
    y_test,
    x +m_width  -m_test_right, 
    y_test+1);
  if (m_allow_negative_y)
  {
    if ((0<=rect.bottom) && (rect.top<0)) rect.top = 0;
  }

  return monde.RectEstDansVide (rect);
#else
  Rectanglei rect( x+m_test_left, y_test, m_width-m_test_right-m_test_left, 1);
  if (m_allow_negative_y)
  {
    if ( rect.y < 0 ) 
       {
	  int b = rect.y + rect.h;
	  rect.y = 0;
	  rect.h = ( b > 0 ) ? b - rect.y : 0;
       }
  }
   
/*  if ( monde.RectEstDansVide (Rectanglei(rect)))
     {
	std::cout << "physicalobk.cpp:629: physobj " << rect.x << " " << rect.y <<" (" << rect.w << "," << rect.h <<") dans le vide" << std::endl;
     }
   else
     {
	std::cout << "physicalobk.cpp:629: physobj " << rect.x << " " << rect.y <<" EST POSE" << std::endl;	
     }*/
   
  return monde.RectEstDansVide (rect);
   
#endif
}

//-----------------------------------------------------------------------------

bool PhysicalObj::IsInWater () const
{
  assert (!IsGhost());
  if (!monde.water.IsActive()) return false;
  int x = BorneLong(GetCenterX(), 0, monde.GetWidth()-1);
  return (int)monde.water.GetHeight(x) < GetCenterY();
}

//-----------------------------------------------------------------------------

bool PhysicalObj::CollisionTest (int x, int y)
{ 
  return !IsInVacuumXY (x,y);
}

//-----------------------------------------------------------------------------

void PhysicalObj::DirectFall()
{
  while (!IsGhost() && !IsInWater() && FootsInVacuum())
   {
//      std::cout << "DirectFall " << GetX() << " " << GetY() << std::endl;
      SetY(GetY()+1);
   }
}

//-----------------------------------------------------------------------------

bool PhysicalObj::ContactPoint (int & contact_x, int & contact_y)
{
  //On cherche un point de contact en bas de l'objet:
  for (uint x=GetX()+ m_test_left; x<=(GetX()+m_width)-m_test_right; x++)
  {
    if(!monde.EstHorsMonde(Point2i(x,(GetY()+m_height-m_test_bottom))))       
//    if(!monde.terrain.EstDansVide(x,(GetY()+m_height-m_test_bottom)+1)
//    &&( monde.terrain.EstDansVide(x,(GetY()+m_height-m_test_bottom))
//    || monde.terrain.EstDansVide(x,(GetY()+m_height-m_test_bottom)+2)))
    if(monde.terrain.EstDansVide(x,(GetY()+m_height-m_test_bottom)-1)
    && !monde.terrain.EstDansVide(x,(GetY()+m_height-m_test_bottom)))
    {
      contact_x = x;
      contact_y = GetY() +m_height-m_test_bottom;
      return true;
    }
  }
  //On cherche un point de contact � gauche de l'objet:
  for(uint y=GetY()+m_test_top;y<=GetY()+m_height-m_test_bottom;y++)
  {
    if(!monde.EstHorsMonde(Point2i(GetX()+m_test_left-1,y)))
//    if(!monde.terrain.EstDansVide(GetX()+m_test_left-1,y)
//    &&( monde.terrain.EstDansVide(GetX()+m_test_left,y)
//    ||  monde.terrain.EstDansVide(GetX()+m_test_left-2,y)))
    if(!monde.terrain.EstDansVide(GetX()+m_test_left,y)
    &&  monde.terrain.EstDansVide(GetX()+m_test_left+1,y))
    {
      contact_x = GetX() +m_test_left;
      contact_y = y;
      return true;
    }
  }
  //On cherche un point de contact � droite de l'objet:
  for(uint y=GetY()+m_test_top;y<=GetY()+m_height-m_test_bottom;y++)
  {
    if(!monde.EstHorsMonde(Point2i((GetX()+m_width-m_test_right)+1,y)))
//    if(!monde.terrain.EstDansVide((GetX()+m_width-m_test_right)+1,y)
//    &&( monde.terrain.EstDansVide((GetX()+m_width-m_test_right)+2,y)
//    ||  monde.terrain.EstDansVide((GetX()+m_width-m_test_right),y)))
    if(!monde.terrain.EstDansVide((GetX()+m_width-m_test_right),y)
    &&  monde.terrain.EstDansVide((GetX()+m_width-m_test_right)-1,y))
    {
      contact_x = GetX() + m_width - m_test_right;
      contact_y = y;
      return true;
    }
  }
  //On cherche un point de contact en haut de l'objet:
  for(uint x=GetX()+m_test_left;x<=GetX()+m_width-m_test_right;x++)
  {
    if(!monde.EstHorsMonde(Point2i(x,GetY()+m_test_top-1)))
//    if(!monde.terrain.EstDansVide(x,GetY()+m_test_top-1)
//    &&( monde.terrain.EstDansVide(x,GetY()+m_test_top-2)
//    ||  monde.terrain.EstDansVide(x,GetY()+m_test_top)))
    if(!monde.terrain.EstDansVide(x,GetY()+m_test_top)
    &&  monde.terrain.EstDansVide(x,GetY()+m_test_top-1))
    {
      contact_x =x;
      contact_y = GetY() +m_test_top;
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------

#ifndef CL // from tool/geomtry_tool.cpp

// Est-ce que deux objets se touchent ? (utilise les rectangles de test)
bool ObjTouche (const PhysicalObj &a, const PhysicalObj &b)
{
  return Intersect (a.GetTestRect(), b.GetTestRect());
}

//-----------------------------------------------------------------------------

// Est-ce que le point p touche l'objet a ?
bool ObjTouche (const PhysicalObj &a, const Point2i &p)
{
   Rectanglei _r = a.GetTestRect();
   Point2i _p = p;
   
   return IsInside ( _r, _p);
}

#endif // CL not defined
