#pragma once

class CRect
{
public:
  int32_t x1, y1, x2, y2;

  CRect();
  CRect( const int32_t a, const int32_t b, const int32_t c, const int32_t d );
  CRect( const CPoint p1, const CPoint p2 );
  CRect( const CRect &r );
  const TBOOL Contains( const int32_t x, const int32_t y ) const;
  const TBOOL Contains( CPoint&p ) const;
  const TBOOL Contains( const CPoint&p ) const;
  const int32_t Width() const;
  const int32_t Height() const;
  const int32_t Area() const;
  const TBOOL Intersects( const CRect &r ) const;

  CRect operator +( const CRect &a ) const;
  CRect operator -( const CRect &a ) const;
  CRect operator *( const int32_t a ) const;
  CRect &operator += ( const CRect &a ); //inflate by rect
  CRect operator+( const CPoint&p ) const;
  CRect &operator += ( const CPoint&p );
  CRect operator-( const CPoint&p ) const;
  CRect &operator -= ( const CPoint&p );
  const TBOOL operator==( const CRect &r ) const;
  const TBOOL operator!=( const CRect &r ) const;
  CRect operator|( const CRect&r )const;
  CRect &operator |= ( const CRect&r );
  CRect operator&( const CRect&r )const;
  CRect &operator &= ( const CRect&r );

  void Move( int32_t x, int32_t y );
  void Move( const CPoint&p );
  void MoveTo( int32_t x, int32_t y );
  void SetSize( int32_t x, int32_t y );
  void Normalize();
  CPoint TopLeft() const;
  CPoint BottomRight() const;
  CPoint TopRight() const;
  CPoint BottomLeft() const;
  CSize Size() const;
  CPoint Center() const;
  CRect GetCenterRect( CSize &s );
  CRect GetIntersection( const CRect&r )const;
};

bool IntervalIntersection( int32_t a1, int32_t a2, int32_t b1, int32_t b2 );
