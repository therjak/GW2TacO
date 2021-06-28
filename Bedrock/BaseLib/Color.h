#pragma once

#include <cstdint>

class CColor
{
  uint8_t b = 0, g = 0, r = 0, a = 0;

public:

  CColor();
  CColor( const uint8_t _r, const uint8_t _g, const uint8_t _b, const uint8_t _a );
  CColor( const uint8_t* c );
  CColor( const uint32_t argb );

  static CColor FromFloat( const float _r, const float _g, const float _b, const float _a );
  static CColor FromARGB( const uint32_t v );
  static CColor FromABGR( const uint32_t v );

  uint8_t const operator[]( int32_t idx ) const;
  uint8_t &operator[]( int32_t idx );
  operator uint8_t* ( );
  operator const uint8_t* ( ) const;
  bool operator== ( const CColor &c ) const;
  bool operator!= ( const CColor &c ) const;
  uint32_t argb() const;

  operator uint32_t() const;
  uint8_t &R();
  uint8_t &G();
  uint8_t &B();
  uint8_t &A();
};

const CColor Lerp( const CColor a, const CColor b, const float t );
