#include "BaseLib.h"


CVector3 CPlane::Intersect( const CLine &l ) const
{
  float u = ( Normal*l.Point + D ) / ( Normal*l.Direction );
  return l.GetPoint( u );
}

int32_t CPlane::Side( const CVector3 &v ) const
{
  float f = Distance( v );
  if ( f > 0 ) return 1;
  if ( f < 0 ) return -1;
  return 0;
}

float CPlane::Distance( const CVector3 &v ) const
{
  return Normal*v + D; //optimized for a normalized plane
}

CLine CPlane::Project( const CLine &l ) const
{
  CVector3 a = Project( l.Point );
  CVector3 b = Project( l.Point + l.Direction );
  return CLine( a, b - a );
}

CVector3 CPlane::Project( const CVector3 &v ) const
{
  return v - Normal*( Normal*v + D ); //optimized for a normalized plane
}

CPlane::CPlane( const CVector3 &a, const CVector3 &b, const CVector3 &c )
{
  Normal = ( ( b - a ) % ( c - a ) ).Normalized();
  D = -( Normal*a );
  Normalize();
}

CPlane::CPlane( const CVector3 &Point, const CVector3 &n )
{
  Normal = n;
  D = -( Normal*Point );
  Normalize();
}

CPlane::CPlane()
{

}

void CPlane::Normalize()
{
  float l = Normal.Length();
  Normal /= l;
  D /= l;
}
