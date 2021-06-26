#include "BaseLib.h"

const int32_t Lerp( const int32_t v1, const int32_t v2, const float t )
{
  return (int32_t)( ( v2 - v1 )*t + v1 );
}

const float Lerp( const float v1, const float v2, const float t )
{
  return ( v2 - v1 )*t + v1;
}

const uint32_t Lerp( const uint32_t v1, const uint32_t v2, const float t )
{
  return (uint32_t)( ( v2 - v1 )*t + v1 );
}

const int64_t Lerp( const int64_t v1, const int64_t v2, const float t )
{
  return (int64_t)( ( v2 - v1 )*t + v1 );
}

const double Lerp( const double v1, const double v2, const float t )
{
  return ( v2 - v1 )*t + v1;
}

const uint64_t Lerp( const uint64_t v1, const uint64_t v2, const float t )
{
  return (uint64_t)( ( v2 - v1 )*t + v1 );
}

const CVector2 Lerp( const CVector2 &v1, const CVector2 &v2, const float t )
{
  return ( v2 - v1 )*t + v1;
}

const CVector2I Lerp( const CVector2I &v1, const CVector2I &v2, const float t )
{
  return ( v2 - v1 )*t + v1;
}

const CVector3 Lerp( const CVector3 &v1, const CVector3 &v2, const float t )
{
  return ( v2 - v1 )*t + v1;
}

const CVector4 Lerp( const CVector4 &v1, const CVector4 &v2, const float t )
{
  return ( v2 - v1 )*t + v1;
}

float mod( float v, float m )
{
  return m * (int32_t)floor( v / m );
}

int32_t Mod( int32_t a, int32_t b )
{
  return ( ( a%b ) + b ) % b;
}

float Mod( float a, float b )
{
  return fmodf( ( ( fmodf( a, b ) ) + b ), b );
}

float Mod( float a, int32_t b )
{
  return fmodf( ( ( fmodf( a, (float)b ) ) + b ), (float)b );
}

CVector3 CVector3::operator*( const CQuaternion &q ) const
{
  return *this*CMatrix3x3( q );
}

CVector3 &CVector3::operator*=( const CQuaternion &q )
{
  return *this = *this*CMatrix3x3( q );
}

CVector3 CVector3::operator/( const CQuaternion &q ) const
{
  return *this*CMatrix3x3( q ).Transposed(); //rot matrix inverse=transpose
}

CVector3 &CVector3::operator/=( const CQuaternion &q )
{
  return *this = *this*CMatrix3x3( q ).Transposed(); //rot matrix inverse=transpose
}

CVector3 CVector3::operator*( const CMatrix3x3 &q ) const
{
  return q.Apply( *this );
}

CVector3 &CVector3::operator*=( const CMatrix3x3 &q )
{
  return *this = q.Apply( *this );
}

CVector4 CVector3::operator*( const CMatrix4x4 &q ) const
{
  return q.Apply( *this );
}

CVector4 CVector4::operator*( const CMatrix4x4 &q ) const
{
  return q.Apply( *this );
}

CVector4 CVector4::Cross( const CVector4 &v1, const CVector4 &v2, const CVector4 &v3 )
{
  return CVector4(
    v1.y*( v2.z*v3.w - v3.z*v2.w ) - v1.z*( v2.y*v3.w - v3.y*v2.w ) + v1.w*( v2.y*v3.z - v2.z*v3.y ),
    -( v1.x*( v2.z*v3.w - v3.z*v2.w ) - v1.z*( v2.x*v3.w - v3.x*v2.w ) + v1.w*( v2.x*v3.z - v3.x*v2.z ) ),
    v1.x*( v2.y*v3.w - v3.y*v2.w ) - v1.y*( v2.x*v3.w - v3.x*v2.w ) + v1.w*( v2.x*v3.y - v3.x*v2.y ),
    -( v1.x*( v2.y*v3.z - v3.y*v2.z ) - v1.y*( v2.x*v3.z - v3.x*v2.z ) + v1.z*( v2.x*v3.y - v3.x*v2.y ) ) );
}

float CVector4::Dot( const CVector4 &v1, const CVector4 &v2 )
{
  return v1*v2;
}

void CVector4::Homogenize()
{
  *this = Homogenized();
}

CVector4 CVector4::Homogenized() const
{
  return *this / w;
}

void CVector4::Normalize()
{
  *this = Normalized();
}

CVector4 CVector4::Normalized() const
{
  return *this*InvSqrt( LengthSquared() );
}

float CVector4::LengthSquared() const
{
  return x*x + y*y + z*z + w*w;
}

float CVector4::Length() const
{
  return sqrtf( LengthSquared() );
}

CVector4::CVector4( const CVector4 &v )
{
  x = v.x;
  y = v.y;
  z = v.z;
  w = v.w;
}

CVector4::CVector4( const float* v )
{
  x = v[ 0 ];
  y = v[ 1 ];
  z = v[ 2 ];
  w = v[ 3 ];
}

CVector4::CVector4( const float _x, const float _y, const float _z, const float _w )
{
  x = _x;
  y = _y;
  z = _z;
  w = _w;
}

CVector4::CVector4()
{

}

CVector3 CVector3::operator* ( const CPRS &q ) const
{
  return q.Apply( *this );
}

CVector3 &CVector3::operator*= ( const CPRS &q )
{
  return *this = q.Apply( *this );
}

CVector3 CVector3::Cross( const CVector3 &v1, const CVector3 &v2 )
{
  return v1%v2;
}

float CVector3::Dot( const CVector3 &v1, const CVector3 &v2 )
{
  return v1*v2;
}

void CVector3::Normalize()
{
  *this = Normalized();
}

CVector3 CVector3::Normalized() const
{
  return *this*InvSqrt( LengthSquared() );
}

float CVector3::LengthSquared() const
{
  return x*x + y*y + z*z;
}

float CVector3::Length() const
{
  return sqrtf( LengthSquared() );
}

CVector3::CVector3( const CVector3 &v )
{
  x = v.x;
  y = v.y;
  z = v.z;
}

CVector3::CVector3( const float* v )
{
  x = v[ 0 ];
  y = v[ 1 ];
  z = v[ 2 ];
}

CVector3::CVector3( const float _x, const float _y, const float _z )
{
  x = _x;
  y = _y;
  z = _z;
}

CVector3::CVector3()
{

}


float CVector2::Dot( const CVector2 &v1, const CVector2 &v2 )
{
  return v1*v2;
}

void CVector2::Normalize()
{
  *this = Normalized();
}

CVector2 CVector2::Normalized() const
{
  return *this*InvSqrt( LengthSquared() );
}

float CVector2::LengthSquared() const
{
  return x*x + y*y;
}

float CVector2::Length() const
{
  return sqrtf( LengthSquared() );
}

CVector2::CVector2( const CVector2 &v )
{
  x = v.x;
  y = v.y;
}

CVector2::CVector2( const float* v )
{
  x = v[ 0 ];
  y = v[ 1 ];
}

CVector2::CVector2( const float _x, const float _y )
{
  x = _x;
  y = _y;
}

CVector2::CVector2()
{

}

float const CVector2::operator[]( int32_t idx ) const
{
  return ( ( const float* )this )[ idx ];
}

float &CVector2::operator[]( int32_t idx )
{
  return ( ( float* )this )[ idx ];
}

CVector2::operator float* ( )
{
  return (float*)&x;
}

CVector2::operator const float* ( ) const
{
  return (const float*)&x;
}

CVector2 &CVector2::operator+= ( const CVector2 &v )
{
  x += v.x;
  y += v.y;
  return *this;
}

CVector2 &CVector2::operator-= ( const CVector2 &v )
{
  x -= v.x;
  y -= v.y;
  return *this;
}

CVector2 &CVector2::operator*= ( const float f )
{
  x *= f;
  y *= f;
  return *this;
}

CVector2 CVector2::Rotated( const CVector2 &center, float rotation )
{
  CVector2 n = ( *this ) - center;
  return CVector2( n.x*cosf( rotation ) - n.y*sinf( rotation ),
                   n.y*cosf( rotation ) + n.x*sinf( rotation ) ) + center;
}

CVector2 &CVector2::operator/= ( const float f )
{
  float fi = 1 / f;
  x *= fi;
  y *= fi;
  return *this;
}

CVector2 CVector2::operator+ () const
{
  return *this;
}

CVector2 CVector2::operator- () const
{
  return CVector2( -x, -y );
}

CVector2 CVector2::operator+ ( const CVector2 &v ) const
{
  return CVector2( x + v.x, y + v.y );
}

CVector2 CVector2::operator- ( const CVector2 &v ) const
{
  return CVector2( x - v.x, y - v.y );
}

CVector2 CVector2::operator* ( const float f ) const
{
  return CVector2( x*f, y*f );
}

CVector2 CVector2::operator/ ( const float f ) const
{
  float fi = 1 / f;
  return CVector2( x*fi, y*fi );
}

TBOOL CVector2::operator== ( const CVector2 &v ) const
{
  return x == v.x && y == v.y;
}

TBOOL CVector2::operator!= ( const CVector2 &v ) const
{
  return x != v.x || y != v.y;
}

float CVector2::operator* ( const CVector2 &v ) const //dot product
{
  return x*v.x + y*v.y;
}

int32_t CVector2I::Dot( const CVector2I &v1, const CVector2I &v2 )
{
  return v1*v2;
}

void CVector2I::Normalize()
{
  *this = Normalized();
}

CVector2I CVector2I::Normalized() const
{
  return *this*InvSqrt( LengthSquared() );
}

float CVector2I::LengthSquared() const
{
  return (float)( x*x + y*y );
}

float CVector2I::Length() const
{
  return sqrtf( LengthSquared() );
}

CVector2I::CVector2I( const CVector2I &v )
{
  x = v.x;
  y = v.y;
}

CVector2I::CVector2I( const int32_t* v )
{
  x = v[ 0 ];
  y = v[ 1 ];
}

CVector2I::CVector2I( const int32_t _x, const int32_t _y )
{
  x = _x;
  y = _y;
}

CVector2I::CVector2I()
{

}

int32_t const CVector2I::operator[]( int32_t idx ) const
{
  return ( ( const int32_t* )this )[ idx ];
}

int32_t &CVector2I::operator[]( int32_t idx )
{
  return ( ( int32_t* )this )[ idx ];
}

CVector2I::operator int32_t* ( )
{
  return (int32_t*)&x;
}

CVector2I::operator const int32_t* ( ) const
{
  return (const int32_t*)&x;
}

CVector2I &CVector2I::operator+= ( const CVector2I &v )
{
  x += v.x;
  y += v.y;
  return *this;
}

CVector2I &CVector2I::operator-= ( const CVector2I &v )
{
  x -= v.x;
  y -= v.y;
  return *this;
}

CVector2I &CVector2I::operator*= ( const float f )
{
  x = (int32_t)( x*f );
  y = (int32_t)( y*f );
  return *this;
}

CVector2I &CVector2I::operator/= ( const float f )
{
  float fi = 1 / f;
  x = (int32_t)( x*fi );
  y = (int32_t)( y*fi );
  return *this;
}

CVector2I &CVector2I::operator/= ( const int32_t f )
{
  x = x / f;
  y = y / f;
  return *this;
}

CVector2I CVector2I::operator+ () const
{
  return *this;
}

CVector2I CVector2I::operator- () const
{
  return CVector2I( -x, -y );
}

CVector2I CVector2I::operator+ ( const CVector2I &v ) const
{
  return CVector2I( x + v.x, y + v.y );
}

CVector2I CVector2I::operator- ( const CVector2I &v ) const
{
  return CVector2I( x - v.x, y - v.y );
}

CVector2I CVector2I::operator* ( const float f ) const
{
  return CVector2I( (int32_t)( x*f ), (int32_t)( y*f ) );
}

CVector2I CVector2I::operator/ ( const float f ) const
{
  float fi = 1 / f;
  return CVector2I( (int32_t)( x*fi ), (int32_t)( y*fi ) );
}

CVector2I CVector2I::operator/ ( const int32_t f ) const
{
  return CVector2I( x / f, y / f );
}

TBOOL CVector2I::operator== ( const CVector2I &v ) const
{
  return x == v.x && y == v.y;
}

TBOOL CVector2I::operator!= ( const CVector2I &v ) const
{
  return x != v.x || y != v.y;
}

int32_t CVector2I::operator* ( const CVector2I &v ) const //dot product
{
  return x*v.x + y*v.y;
}

float const CVector3::operator[]( int32_t idx ) const
{
  return ( ( const float* )this )[ idx ];
}

float &CVector3::operator[]( int32_t idx )
{
  return ( ( float* )this )[ idx ];
}

CVector3::operator float* ( )
{
  return (float*)&x;
}

CVector3::operator const float* ( ) const
{
  return (const float*)&x;
}

CVector3 &CVector3::operator+= ( const CVector3 &v )
{
  x += v.x;
  y += v.y;
  z += v.z;
  return *this;
}

CVector3 &CVector3::operator-= ( const CVector3 &v )
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
  return *this;
}

CVector3 &CVector3::operator*= ( const float f )
{
  x *= f;
  y *= f;
  z *= f;
  return *this;
}

CVector3 &CVector3::operator/= ( const float f )
{
  float fi = 1 / f;
  x *= fi;
  y *= fi;
  z *= fi;
  return *this;
}

CVector3 CVector3::operator+ () const
{
  return *this;
}

CVector3 CVector3::operator- () const
{
  return CVector3( -x, -y, -z );
}

CVector3 CVector3::operator+ ( const CVector3 &v ) const
{
  return CVector3( x + v.x, y + v.y, z + v.z );
}

CVector3 CVector3::operator- ( const CVector3 &v ) const
{
  return CVector3( x - v.x, y - v.y, z - v.z );
}

CVector3 CVector3::operator* ( const float f ) const
{
  return CVector3( x*f, y*f, z*f );
}

CVector3 CVector3::operator/ ( const float f ) const
{
  float fi = 1 / f;
  return CVector3( x*fi, y*fi, z*fi );
}

TBOOL CVector3::operator== ( const CVector3 &v ) const
{
  return x == v.x && y == v.y && z == v.z;
}

TBOOL CVector3::operator!= ( const CVector3 &v ) const
{
  return x != v.x || y != v.y || z != v.z;
}

float CVector3::operator* ( const CVector3 &v ) const //dot product
{
  return x*v.x + y*v.y + z*v.z;
}

CVector3 CVector3::operator% ( const CVector3 &v ) const //cross product
{
  return CVector3( y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x );
}

float const CVector4::operator[]( int32_t idx ) const
{
  return ( ( const float* )this )[ idx ];
}

float &CVector4::operator[]( int32_t idx )
{
  return ( ( float* )this )[ idx ];
}

CVector4::operator float* ( )
{
  return (float*)&x;
}

CVector4::operator const float* ( ) const
{
  return (const float*)&x;
}

CVector4::operator CVector3 ()
{
  return CVector3( x, y, z );
}

CVector4::operator const CVector3() const
{
  return CVector3( x, y, z );
}

CVector4 &CVector4::operator+= ( const CVector4 &v )
{
  x += v.x;
  y += v.y;
  z += v.z;
  w += v.w;
  return *this;
}

CVector4 &CVector4::operator-= ( const CVector4 &v )
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
  w -= v.w;
  return *this;
}

CVector4 &CVector4::operator*= ( const float f )
{
  x *= f;
  y *= f;
  z *= f;
  w *= f;
  return *this;
}

CVector4 &CVector4::operator/= ( const float f )
{
  float fi = 1 / f;
  x *= fi;
  y *= fi;
  z *= fi;
  w *= fi;
  return *this;
}

CVector4 CVector4::operator+ () const
{
  return *this;
}

CVector4 CVector4::operator- () const
{
  return CVector4( -x, -y, -z, -w );
}

CVector4 CVector4::operator+ ( const CVector4 &v ) const
{
  return CVector4( x + v.x, y + v.y, z + v.z, w + v.w );
}

CVector4 CVector4::operator- ( const CVector4 &v ) const
{
  return CVector4( x - v.x, y - v.y, z - v.z, w - v.w );
}

CVector4 CVector4::operator* ( const float f ) const
{
  return CVector4( x*f, y*f, z*f, w*f );
}

CVector4 CVector4::operator/ ( const float f ) const
{
  float fi = 1 / f;
  return CVector4( x*fi, y*fi, z*fi, w*fi );
}

TBOOL CVector4::operator== ( const CVector4 &v ) const
{
  return x == v.x && y == v.y && z == v.z && w == v.w;
}

TBOOL CVector4::operator!= ( const CVector4 &v ) const
{
  return x != v.x || y != v.y || z != v.z || w != v.w;
}

float CVector4::operator* ( const CVector4 &v ) const //dot product
{
  return x*v.x + y*v.y + z*v.z + w*v.w;
}
