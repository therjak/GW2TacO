#pragma once

class CColor
{
  uint8_t b, g, r, a;

public:

  CColor();
  CColor( const uint8_t _r, const uint8_t _g, const uint8_t _b, const uint8_t _a );
  CColor( const uint8_t* c );
  CColor( const uint32_t argb );

  static CColor FromFloat( const TF32 _r, const TF32 _g, const TF32 _b, const TF32 _a );
  static CColor FromARGB( const uint32_t v );
  static CColor FromABGR( const uint32_t v );

  uint8_t const operator[]( int32_t idx ) const;
  uint8_t &operator[]( int32_t idx );
  operator uint8_t* ( );
  operator const uint8_t* ( ) const;
  TBOOL operator== ( const CColor &c ) const;
  TBOOL operator!= ( const CColor &c ) const;
  uint32_t argb() const;

  operator uint32_t() const;
  uint8_t &R();
  uint8_t &G();
  uint8_t &B();
  uint8_t &A();
};

const CColor Lerp( const CColor a, const CColor b, const TF32 t );
