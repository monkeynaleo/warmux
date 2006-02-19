#ifndef _VECTOR2_H
#define _VECTOR2_H

#include <math.h>
#define VECTOR2_EPS_ZERO (0.05)

/**
 * Class for storing a vector of two points x, y.
 */
template<class T> class Vector2
{
	private:
		const static double EPS_ZERO = 0.05;

	public:
		T x, y;

		/** 
		 * Default constructor that will be a vector null (0, 0)
		 */
		inline Vector2(){
			x = 0;
			y = 0;
		}
		
		/**
		 * Constructor that build a new vector from two values, x and y.
		 *
		 * @param x 
		 * @param y
		 */
		inline Vector2(T x, T y){
			this->x = x;
			this->y = y;
		}

		/**
		 * Return the x coordinate.
		 */
		inline T GetX() const{
			return x;
		}

		/**
		 * Return the y coordinate.
		 */
		inline T GetY() const{
			return y;
		}

		/**
		 *
		 * @param p2
		 */
		inline bool operator!=(const Vector2<T> &p2) const{
			return (x != p2.x) || (y != p2.y);
		}
		
		/**
		 *
		 * @param p2
		 */
		inline bool operator>=(const Vector2<T> &p2) const{
			return (x >= p2.x) && (y >= p2.y);
		}

		/**
		 *
		 * @param p2
		 */
		inline bool operator<=(const Vector2<T> &p2) const{
			return (x <= p2.x) && (y <= p2.y);
		}

		/** 
		 *
		 * @param p2
		 */
		inline Vector2<T> operator+(const Vector2<T> &p2) const{
			Vector2<T> r = *this;

			r.x += p2.x;
			r.y += p2.y;

			return r;
		}

		/**
		 *
		 * @param p2
		 */
		inline Vector2<T> operator-(const Vector2<T> &p2) const{
			Vector2<T> r = *this;

			r.x -= p2.x;
			r.y -= p2.y;

			return r;
		}

		/**
		 *
		 *
		 * @param val
		 */
		inline Vector2<T> operator+(const T val) const{
			Vector2<T> r = *this;

			r.x += val;
			r.y += val;

			return r;
		}

		/**
		 *
		 * @param val
		 */
		inline Vector2<T> operator/(const T val) const{
	        Vector2<T> r = *this;

			r.x /= val;
            r.y /= val;

			return r;
        }


		/**
		 *
		 * @param val
		 */
		inline Vector2<T> operator-(const T val) const{
			Vector2<T> r = *this;

			r.x -= val;
			r.y -= val;

			return r;
		}
		
		/**
		 *
		 * @param p2
		 */
		inline double Distance(const Vector2<T> p2) const{
			double distPow2 = (p2.x-x)*(p2.x-x) + (p2.y-y)*(p2.y-y);
			return sqrt( distPow2 );
		}

		/**
		 *
		 */
		double Norm() const{
			return Distance( Vector2(0,0) );
		}

		/**
		 *
		 */
		void Clear(){
			x = 0;
			y = 0;
		}

		/**
		 *
		 */
		void SetValues( T xx, T yy ){
			x = xx;
			y = yy;
		}

		/**
		 *
		 */
		void SetValues( Vector2<T> v2){
			x = v2.x;
			y = v2.y;
		}

		/**
		 *
		 */
		inline bool IsZero(T val) const{
			return (val <= VECTOR2_EPS_ZERO) && (-val <= VECTOR2_EPS_ZERO);
		}

		/**
		 *
		 */
		inline bool IsXNull() const{
			return IsZero( x );
		}

		/**
		 *
		 */
		inline bool IsYNull() const{
			return IsZero( y );
		}

		/**
		 *
		 */
		bool IsNull() const{
			return IsXNull() && IsYNull();
		}

		/**
		 *  Calcule l'angle en radian du point M dans le repère de centre O
		 *
		 * Pour O=(0,0) :
		 * - M=(10,10) -> PI/4 (0.78)
		 * - M=(0,10) -> PI/2 (1.57)
		 * - M=(-10,10) -> 3*PI/4 (2.35)
		 * - M=(10,-10) -> -PI/4 (-0.78)
		 * - M=O -> 0
		 */
		double ComputeAngle() const{
			double angle;

			if( !IsZero( x ) )
				if( !IsZero( y ) ){
					angle = atan((double)y/x);
					if( x < 0 )
						if( y > 0 )
							angle += M_PI;
						else
							angle -= M_PI;
				}
				else
					if( x > 0)
						angle = 0;
					else
						angle = M_PI;
			else
				if( y > 0 )
					angle = M_PI_2;
				else if(y < 0)
					angle = -M_PI_2;
				else
					angle = 0;

			return angle;
		}

		/**
		 *
		 * @param v2
		 */
		double ComputeAngle(const Vector2<T> v2) const{
			Vector2<T> veq( v2.x - x, v2.y - y);

			return veq.ComputeAngle();
		}
};

typedef Vector2<double> DoubleVector;

#endif //_VECTOR2_H
