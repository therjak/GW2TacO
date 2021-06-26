#include "BaseLib.h"


const CColor Lerp( const CColor a, const CColor b, const float t )
{
  CColor c;
  for ( int32_t x = 0; x < 4; x++ )
    c[ x ] = (uint8_t)( ( b[ x ] - a[ x ] )*t + a[ x ] );
  return c;
}

uint8_t & CColor::A()
{
  return a;
}

uint8_t & CColor::B()
{
  return b;
}

uint8_t & CColor::G()
{
  return g;
}

uint8_t & CColor::R()
{
  return r;
}

CColor::operator uint32_t() const
{
  return argb();
}

uint32_t CColor::argb() const
{
  return ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;
}

CColor CColor::FromABGR( const uint32_t v )
{
  CColor res;
  res.a = (uint8_t)( v >> 24 );
  res.b = (uint8_t)( v >> 16 );
  res.g = (uint8_t)( v >> 8 );
  res.r = (uint8_t)( v >> 0 );
  return res;
}

CColor CColor::FromARGB( const uint32_t v )
{
  CColor res;
  res.a = (uint8_t)( v >> 24 );
  res.r = (uint8_t)( v >> 16 );
  res.g = (uint8_t)( v >> 8 );
  res.b = (uint8_t)( v >> 0 );
  return res;
}

CColor CColor::FromFloat( const float _r, const float _g, const float _b, const float _a )
{
  CColor res;

  float c[ 4 ];
  c[ 0 ] = _r;
  c[ 1 ] = _g;
  c[ 2 ] = _b;
  c[ 3 ] = _a;
  for ( int32_t x = 0; x < 4; x++ )
  {
    if ( c[ x ] < 0 ) c[ x ] = 0;
    if ( c[ x ] > 1 ) c[ x ] = 1;
    res[ x ] = (uint8_t)( c[ x ] * 255 );
  }
  return res;
}

CColor::CColor( const uint32_t argb )
{
  r = (uint8_t)( argb >> 16 );
  g = (uint8_t)( argb >> 8 );
  b = (uint8_t)( argb >> 0 );
  a = (uint8_t)( argb >> 24 );
}

CColor::CColor( const uint8_t* c )
{
  r = c[ 0 ];
  g = c[ 1 ];
  b = c[ 2 ];
  a = c[ 3 ];
}

CColor::CColor( const uint8_t _r, const uint8_t _g, const uint8_t _b, const uint8_t _a )
{
  r = _r;
  g = _g;
  b = _b;
  a = _a;
}

CColor::CColor()
{

}

uint8_t const CColor::operator[]( int32_t idx ) const
{
  return ( ( const uint8_t* )this )[ idx ]; // nem fog ez kesobb fajni?!
}

uint8_t &CColor::operator[]( int32_t idx )
{
  return ( ( uint8_t* )this )[ idx ];
}

CColor::operator uint8_t* ( )
{
  return ( uint8_t* )this;
}

CColor::operator const uint8_t* ( ) const
{
  return ( const uint8_t* )this;
}

TBOOL CColor::operator== ( const CColor &c ) const
{
  return r == c.r && g == c.g && b == c.b && a == c.a;
}

TBOOL CColor::operator!= ( const CColor &c ) const
{
  return r != c.r || g != c.g || b != c.b || a != c.a;
}
