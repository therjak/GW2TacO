#include "BaseLib.h"

CBBox::CBBox()
{
  lo.x = 1;
  hi.x = -1;
}

CBBox::CBBox( const CBBox &aabb )
{
  lo = aabb.lo;
  hi = aabb.hi;
}

CBBox::CBBox( const CVector3 &v )
{
  lo = hi = v;
}

CBBox::CBBox( const CVector3 &min, const CVector3 &max )
{
  lo.x = 1;
  hi.x = -1;
  Expand( min );
  Expand( max );
}

TBOOL CBBox::isEmpty()
{
  return lo.x > hi.x;
}

void CBBox::Expand( const CVector3 &v )
{
  if ( isEmpty() )
  {
    lo = hi = v;
    return;
  }

  for ( int32_t i = 0; i < 3; i++ )
  {
    lo[ i ] = lo[ i ] < v[ i ] ? lo[ i ] : v[ i ];
    hi[ i ] = hi[ i ] > v[ i ] ? hi[ i ] : v[ i ];
  }
}

void CBBox::Expand( const CBBox &v )
{
  Expand( v.lo );
  Expand( v.hi );
}

CBBox CBBox::operator+( const CVector3 &v ) const
{
  CBBox aabb( *this );
  aabb.Expand( v );
  return aabb;
}

CBBox &CBBox::operator+=( const CVector3 &v )
{
  Expand( v );
  return *this;
}

CBBox CBBox::operator+( const CBBox &v ) const
{
  CBBox aabb( *this );
  aabb.Expand( v );
  return aabb;
}

CBBox &CBBox::operator+=( const CBBox &v )
{
  Expand( v );
  return *this;
}

TBOOL CBBox::Intersect( const CPlane &pl )
{
  return pl.Distance( CVector3( pl.Normal.x >= 0 ? hi.x : lo.x,
                                pl.Normal.y >= 0 ? hi.y : lo.y,
                                pl.Normal.z >= 0 ? hi.z : lo.z ) ) < 0;
}
