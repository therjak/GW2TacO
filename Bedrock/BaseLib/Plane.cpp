#include "BaseLib.h"


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
