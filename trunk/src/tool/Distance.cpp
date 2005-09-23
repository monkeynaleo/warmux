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
 * Distance.cpp: Comppute distances between different geometrics things
 ******************************************************************************
 * 2005/09/21:   Jean-Christophe Duberga (jcduberga@gmx.de) 
 *               Initial version
 ******************************************************************************/

#include "Distance.h"
#include "Point.h"
#include "Rectangle.h"

int Distance( const Point2i &p1, const Point2i &p2)
{
   return (int) sqrt( (p2.x-p1.x)*(p2.x-p1.x) + (p2.y-p1.y)*(p2.y-p1.y));
}

float Distance( const Point2f &p1, const Point2f &p2)
{
   return sqrtf( (p2.x-p1.x)*(p2.x-p1.x) + (p2.y-p1.y)*(p2.y-p1.y));
}

float Distance( const Point2d &p1, const Point2d &p2)
{
   return sqrt( (p2.x-p1.x)*(p2.x-p1.x) + (p2.y-p1.y)*(p2.y-p1.y));   
}

bool IsInside( const Rectanglei &r, const Point2i &p)
{
   return ( p.x >= r.x && p.x <= r.x+r.w && p.y >= r.y && p.y <= r.y+r.h ); 
}

bool IsInside( const Rectanglef &r, const Point2f &p)
{
   return ( p.x >= r.x && p.x <= r.x+r.w && p.y >= r.y && p.y <= r.y+r.h );  
}

bool IsInside( const Rectangled &r, const Point2d &p)
{
   return ( p.x >= r.x && p.x <= r.x+r.w && p.y >= r.y && p.y <= r.y+r.h );  
}

bool Intersect( const Rectanglei &r1, const Rectanglei &r2)
{
   return IsInside( r1, Point2i( r2.x, r2.y)) 
       || IsInside( r1, Point2i( r2.x, r2.y+r2.h))
       || IsInside( r1, Point2i( r2.x+r2.w, r2.y+r2.h))
       || IsInside( r1, Point2i( r2.x+r2.w, r2.y));
}
