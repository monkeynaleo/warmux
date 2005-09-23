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
 * Rectangle.h: Standard C++ Rectangle template
 ******************************************************************************
 * 2005/09/21:  Jean-Christophe Duberga (jcduberga@gmx.de) 
 *              Initial version
 *****************************************************************************/

#ifndef _RECTANGLE_H
#define _RECTANGLE_H

#include <cmath>

template<class T> class Rectangle
{
   
 public:
   inline Rectangle(){}
   inline Rectangle(T ox, T oy, T width, T height)
     {
	this->x = ox;
	this->y = oy;
	this->w = width;
	this->h = height;
     }
   
   inline void Clip( const Rectangle &cr)
     {
	T left    = ( x < cr.x ) ? cr.x : ( x > cr.x+cr.w ) ? cr.x+cr.w : x; 
	T right   = ( x+w < cr.x ) ? cr.x : ( x+w > cr.x+cr.w ) ? cr.x+cr.w : x+w;
	T top     = ( y < cr.y ) ? cr.y : ( y > cr.y+cr.h ) ? cr.y+cr.h : y;
	T bottom  = ( y+h < cr.y ) ? cr.y : ( y+h > cr.y+cr.h ) ? cr.y+cr.h : y+h;
	
	x = left;
	w = right-left;
	y = top;
	h = bottom-top;
     }
   
   T x, y, w, h;
};

   
typedef Rectangle<int>    Rectanglei;   
typedef Rectangle<float>  Rectanglef;   
typedef Rectangle<double> Rectangled;   


#endif // _RECTANGLE_H
