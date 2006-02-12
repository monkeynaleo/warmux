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
#include "vector2.h"

/** 
 * This template handle rectangles.
 * 
 * @param T Type for position and size of the Rectangle
 */
template<class T> class Rectangle
{
	private:
		/** Position of the rectangle. */
		Vector2<T> position;
		/** Size of the rectangle. */
		Vector2<T> size; 

	public:
		/**
		 * Default constructor
		 */
		inline Rectangle(){
		}
		
		/**
		 * Constructor for building a rectangle of a certain size
		 *
		 * @param x Position among the x axe
		 * @param y Position among the y axe
		 * @param width Width of the new rectangle
		 * @param height Height of the new rectangle
		 */
		inline Rectangle(T x, T y, T width, T height){
			position.SetValues( x, y );
			size.SetValues( width, height );
		}

		inline Rectangle(Vector2<T> thePosition, Vector2<T> theSize){
			position = thePosition;
			size = theSize;
		}

		/**
		 * Set the position of the rectangle.
		 *
		 * @param x new X position.
		 * @param y new Y position.
		 */
		inline void SetPosition(T x, T y){
			position.SetValues(x, y);
		}

		inline void SetPosition(const Vector2<T> &newPos){
			position = newPos;
		}

		inline void SetPositionX(T x){
			position.x = x;
		}

		inline void SetPositionY(T y){
			position.y = y;
		}

		/**
		 * Set the size of the rectangle.
		 *
		 * @param sizeX new size among the x axe.
		 * @param sizeY new size among the y axe.
		 */
		inline void SetSize(T sizeX, T sizeY){
			size.SetValues(sizeX, sizeY);
		}

		inline void SetSizeX(T sizeX){
			size.x = sizeX;
		}

		inline void SetSizeY(T sizeY){
			size.y = sizeY;
		}

		inline Vector2<T> GetPosition() const{
			return position;
		}
		
		inline T GetPositionX() const{
			return position.x;			
		}

		inline T GetPositionY() const{
			return position.y;
		}

		inline Vector2<T> GetSize() const{
			return size;
		}
		
		inline T GetSizeX() const{
			return size.x;
		}

		inline T GetSizeY() const{
			return size.y;
		}

		/**
		 * Clip the current rectangle using an other rectangle.
		 * 
		 * @param cr The rectangle used for clipping
		 */
		void Clip( const Rectangle &cr){
			if( !Intersect(cr) ){
				size.x = 0;
				size.y = 0;

				return;
			}
			
			Vector2<T> newPositionBR = GetBottomRightPoint();

			if( position.x < cr.position.x )
				position.x = cr.position.x;
			
			if( position.x > cr.GetBottomRightPoint().x )
				position.x = cr.GetBottomRightPoint().x;

			if( position.y < cr.position.y )
				position.y = cr.position.y;
			
			if( position.y > cr.GetBottomRightPoint().y )
				position.y = cr.GetBottomRightPoint().y;

			if( newPositionBR.x < cr.position.x )
				newPositionBR.x = cr.position.x;
			
			if( newPositionBR.x > cr.GetBottomRightPoint().x )
				newPositionBR.x = cr.GetBottomRightPoint().x;

			if( newPositionBR.y < cr.position.y )
				newPositionBR.y = cr.position.y;
			
			if( newPositionBR.y > cr.GetBottomRightPoint().y )
				newPositionBR.y = cr.GetBottomRightPoint().y;

			size = newPositionBR - position - -(1);
			assert( cr.Contains( *this ) );
		}

		/** 
		 * Return true if the point p is contained in the rectangle.
		 * 
		 * @param p Point used to perform the check.
		 */
		inline bool Contains( const Vector2<T> p ) const{
			if( IsSizeZero() )
				return false;

			return p >= GetTopLeftPoint() &&
				p <= GetBottomRightPoint();
		}

		/**
		 * Return true if r2 is contained in the current rectangle.
		 *
		 * @param r2 The rectangle for witch the check if performed.
		 */
		inline bool Contains( const Rectangle<T> &r2 ) const{
			if( r2.IsSizeZero() )
				return false;
			
			return Contains( r2.GetTopLeftPoint() ) &&
				Contains( r2.GetBottomRightPoint() );
		}

		/** 
		 * Return true if there is an intersection between the current rectangle
		 * and the r2 rectangle.
		 *
		 * @param r2 The second rectangle.
		 */
		inline bool Intersect( const Rectangle<T> &r2 ) const{
			if( IsSizeZero() || r2.IsSizeZero() )
				return false;
			
			Vector2<T> r1BR = GetBottomRightPoint();
			Vector2<T> r2BR = r2.GetBottomRightPoint();
			Vector2<T> r1TL = GetTopLeftPoint();
			Vector2<T> r2TL = r2.GetTopLeftPoint();

			if( r1BR.x < r2TL.x || r1BR.y < r2TL.y ||
				   r2BR.x < r1TL.x || r2BR.y < r1TL.y ) 
				return false;

			return true;
		}

		/** 
		 * Return the point in the top left corner of the rectangle.
		 * 
		 * If the rectangle has a size of zero, this point doesn't exist,
		 * so the program crash with a failled assertion.
		 */
		inline Vector2<T> GetTopLeftPoint() const{
			assert( !IsSizeZero() );
			return position;
		}

		/** 
		 * Return the point in the top right corner of the rectangle.
		 * 
		 * If the rectangle has a size of zero, this point doesn't exist,
		 * so the program crash with a failled assertion.
		 */
		inline Vector2<T> GetTopRightPoint() const{
			assert( !IsSizeZero() );
			Vector2<T> r = position;

			r.x += size.x - 1;

			return r;
		}

		/** 
		 * Return the point in the bottom left corner of the rectangle.
		 * 
		 * If the rectangle has a size of zero, this point doesn't exist,
		 * so the program crash with a failled assertion. 
		 */
		inline Vector2<T> GetBottomLeftPoint() const{
			assert( !IsSizeZero() );
			Vector2<T> r = position;

			r.y += size.y - 1;

			return r;
		}

		/** 
		 * Return the point in the top left corner of the rectangle.
		 * 
		 * If the rectangle has a size of zero, this point doesn't exist,
		 * so the program crash with a failled assertion. */
		inline Vector2<T> GetBottomRightPoint() const{
			assert( !IsSizeZero() );
			return position + size - 1;
		}

		/** 
		 * Return true if the rectangle has a size of zero.
		 */
		inline bool IsSizeZero() const{
			return size.IsXNull() || size.IsYNull();
		}
};

typedef Rectangle<int>    Rectanglei;
typedef Rectangle<float>  Rectanglef;
typedef Rectangle<double> Rectangled;
#endif // _RECTANGLE_H
