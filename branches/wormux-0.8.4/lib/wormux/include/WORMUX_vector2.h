#ifndef WORMUX_VECTOR2_H
#define WORMUX_VECTOR2_H

#include <math.h>
#undef min
#undef max
#define VECTOR2_EPS_ZERO (0.005)

/**
 * Class for storing a vector of two points x, y.
 */
template<class T> class Vector2
{
  public:
    T x, y;

    /**
     * Default constructor that will be a vector null (0, 0)
     */
    inline Vector2():
      x(0), y(0)
                { }

    /**
     * Constructor that build a new vector from two values, x and y.
     *
     * @param x
     * @param y
     */
    inline Vector2(T _x, T _y):
      x(_x), y(_y)
    {}

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
     */
    inline bool IsZero(T val) const{
      return (val == 0 || (val <= static_cast<T>(VECTOR2_EPS_ZERO) &&
                               val >= static_cast<T>(-VECTOR2_EPS_ZERO)));
    }

    // Comparators

    /**
     *
     */
    inline bool operator==(const Vector2<T> &p2) const{
      return IsZero(x - p2.x) && IsZero(y - p2.y);
    }

    /**
     *
     * @param p2
     */
    inline bool operator!=(const Vector2<T> &p2) const{
               //         return (x != p2.x) || (y != p2.y);
      return !IsZero(x - p2.x) || !IsZero(y - p2.y);
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
    inline bool operator>(const Vector2<T> &p2) const{
      return (x > p2.x) && (y > p2.y);
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
    inline bool operator<(const Vector2<T> &p2) const{
      return (x < p2.x) && (y < p2.y);
    }
    // Vector/Vector operations

    /**
     *
     * @param p2
     */
    inline Vector2<T> operator+(const Vector2<T> &p2) const{
      return Vector2<T>(x + p2.x, y + p2.y);
    }

    /**
     *
     * @param p2
     */
    inline Vector2<T> operator-(const Vector2<T> &p2) const{
      return Vector2<T>(x - p2.x, y - p2.y);
    }

                /**
     *
     * @returns the negative value of ourself
     */
    inline Vector2<T> operator-() const{
      return Vector2<T>(-x, -y);
    }

    /**
     *
     * @param p2
     */
    template< typename OtherT >
    inline Vector2<T> operator*(const Vector2<OtherT> &p2) const{
      return Vector2<T>( (T)( x * p2.x ), (T)( y * p2.y ) );
    }

    /**
     *
     * @param p2
     */
    inline Vector2<T> operator/(const Vector2<T> &p2) const{
      return Vector2<T>(x / p2.x, y / p2.y);
    }

    inline Vector2<T> operator%(const Vector2<T> &p2) const{
      return Vector2<T>(x % p2.x, y % p2.y);
    }

    // Vector/Scalar opertations

    /**
     *
     * @param val
     */
    inline Vector2<T> operator+(const T val) const{
      return Vector2<T>(x + val, y + val);
    }

    /**
     *
     * @param val
     */
    inline Vector2<T> operator-(const T val) const{
      return Vector2<T>(x - val, y - val);
    }

    /**
     *
     */
    /*
    inline Vector2<T> operator*(const T val) const{
      return Vector2<T>(x * val, y * val);
    }
    */

    template< typename OtherT >
    inline Vector2<T> operator*(const OtherT val) const{
      return Vector2<T>( (T)( x * val ), (T)( y * val ) );
    }

    /**
     *
     * @param val
     */
    /*
    inline Vector2<T> operator/(const T val) const{
      return Vector2<T>(x / val, y / val);
    }
    */

    template< typename OtherT >
    inline Vector2<T> operator/(const OtherT val) const{
      return Vector2<T>( (T)( x / val ), (T)( y / val ) );
    }

    /**
     *
     */
    inline Vector2<T> operator<<(unsigned int val) const{
      return Vector2<T>(x << val, y << val);
    }
    inline Vector2<T> operator>>(unsigned int val) const{
      return Vector2<T>(x >> val, y >> val);
    }

    // Operators on itself with a scalar

    /**
     *
     * @param val
     */
    inline void operator+=(const T val){
      x += val;
      y += val;
    }

    /**
     *
     * @param val
     */
    inline void operator-=(const T val){
      x -= val;
      y -= val;
    }

    /**
     *
     */
    inline void operator*=(const T val){
      x *= val;
      y *= val;
    }

    /**
     *
     */
    inline void operator/=(const T val){
      x /= val;
      y /= val;
    }

    // Operators on itself with an other vector

    /**
     *
     * @param p2
     */
    inline void operator+=(const Vector2<T> &p2){
      x += p2.x;
      y += p2.y;
    }

    /**
     * @param p2
     */
    inline void operator-=(const Vector2<T> &p2){
      x -= p2.x;
      y -= p2.y;
    }

    inline void operator <<= (unsigned int val){
      x <<= val;
      y <<= val;
    }
    inline void operator >>= (unsigned int val){
      x >>= val;
      y >>= val;
    }

    // Special operators

    /**
     *
     */
    /*
    inline Vector2<T> operator*(const Vector2<double> &p2){
      Vector2<T> r;

      r.x = (T)((double)x * p2.x);
      r.y = (T)((double)y * p2.y);

      return r;
    }
    */

    /**
     * Return the comparaison of two vector in the form of a vector.
     *
     * @param p2
     * @return A vector in which the elements are equal to 1 where the comparaison is true, 0 elsewhere.
     */
    inline Vector2<T> inf(const Vector2<T> &p2){
      return Vector2<T>( x < p2.x ? 1:0,
          y < p2.y ? 1:0);
    }

    /**
     * Return a vector made of the minimum coordinate of the two vectors.
     * Ce clair ne pas ?
     *
     * @param p2
     * @return
     */
    inline Vector2<T> min(const Vector2<T> &p2) const{
      return Vector2<T>( x < p2.x? x:p2.x,
          y < p2.y? y:p2.y );
    }

    /**
     * @param p2
     * @return
     */
    inline Vector2<T> max(const Vector2<T> &p2) const{
                        return Vector2<T>( x > p2.x? x:p2.x,
                       y > p2.y? y:p2.y );
    }

    inline Vector2<T> clamp(const Vector2<T> &min, const Vector2<T> &max) const{
      Vector2<T> r = *this;
      r = r.max(min);
      return r.min(max);
    }

    /**
     * For T == int, this method is defined at the end of this file
     * @param p2
     */
    inline T Distance(const Vector2<T> & p2) const{
      T distPow2 = (p2.x-x)*(p2.x-x) + (p2.y-y)*(p2.y-y);
      return sqrt( distPow2 );
    }

    /**
     *
     */
    T Norm() const{
      return Distance( Vector2(0,0) );
    }

    /**
     * [RCL]: using doubles, to keep the precision consistent with rest of
     * functions here
    */
    Vector2<T> GetNormal() const {
      double len = sqrt(x*x + y*y);
      if ( len > 0.00001 )
        return Vector2<T>( (T)(x/len), (T)(y/len) );
      return *this;
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
    void SetValues( const Vector2<T> & v2){
      x = v2.x;
      y = v2.y;
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

    /** Transform polar coordinate */
    static Vector2<T> FromPolarCoordinates(double norm, double angle) { return Vector2<T>(T(norm*cos(angle)), T(norm*sin(angle))); }

    /**
     *  Compute the angle of point M in the Cartesian plane
     *  centered on O
     *
     * Pour O=(0,0) :
     * - M=(10,10) -> PI/4 (0.78)
     * - M=(0,10) -> PI/2 (1.57)
     * - M=(-10,10) -> 3*PI/4 (2.35)
     * - M=(10,-10) -> -PI/4 (-0.78)
     * - M=O -> 0
     */
     double ComputeAngle() const{
       if( !IsZero( x ) )
         if( !IsZero( y ) ){
           double angle = atan(double(y)/double(x));
           if( x < 0 ){
             if( y > 0 ) return angle + M_PI;
             else        return angle - M_PI;
           }
           return angle;
         }
         else
           return (x > 0) ? 0 : M_PI;
       else if( y > 0 )
         return M_PI_2;
       else if(y < 0)
         return -M_PI_2;
       return 0;
    }

    /**
     *
     * @param v2
     */
    double ComputeAngle(const Vector2<T> & v2) const{
      Vector2<T> veq( v2.x - x, v2.y - y);

      return veq.ComputeAngle();
    }

    /**
     *
     * Computes angle at current point in triangle v1, this, v2
     * @param v1
     * @param v2
     * [RCL]: by the way, why double? floats are not enough?
     */
    double ComputeAngle(const Vector2<T> & v1, const Vector2<T> & v2) const
    {
      Vector2<T> side1( v1.x - x, v1.y - y);
      Vector2<T> side2( v2.x - x, v2.y - y);

      // we need double precision even for ints
      double l1 = sqrt( side1.x * side1.x + side1.y * side1.y );
      double l2 = sqrt( side2.x * side2.x + side2.y * side2.y );

      if ( l1 < 0.00001f || l2 < 0.00001f )
         return 0; // zero vector is collinear with every other

      // now we find the cos of the angle from dot product formula
      // side1 dot side2 = l1 * l2 * cos ( angle_between_side1_and_side2 )
      double cosA = ( side1.x * side2.x + side1.y * side2.y ) / ( l1 * l2 );
      return acos( cosA );
    }

    // convertor
    template< typename OtherT >
    operator Vector2<OtherT>() const
    {
        return Vector2<OtherT>( (OtherT)x, (OtherT)y );
    };
};

template < >
inline int Vector2<int>::Distance(const Vector2<int> & p2) const{
 int distPow2 = (p2.x-x)*(p2.x-x) + (p2.y-y)*(p2.y-y);
 return (int)sqrt( (float)distPow2 );
}

// some other auxilliaries
template< typename T >
inline Vector2<T> operator*(const T val, const Vector2<T> &v) {
    return Vector2<T>( v.x * val, v.y * val );
}

#endif //_VECTOR2_H
