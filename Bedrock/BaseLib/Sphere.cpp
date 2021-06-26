#include "BaseLib.h"

const TBOOL CSphere::Intersect( const CLine &l, float &tmin, float &tmax ) const
{
  CVector3 oc = l.Point - Position;

  float a = l.Direction*l.Direction;
  float b = 2 * ( l.Direction*oc );
  float c = oc*oc - Radius*Radius;

  float det = b*b - 4 * a*c;
  if ( det < 0 ) return false;
  det = sqrtf( det );
  float t1 = ( -b - det ) / ( 2 * a );
  float t2 = ( -b + det ) / ( 2 * a );
  tmin = min( t1, t2 );
  tmax = max( t1, t2 );

  return true;// l.Distance(Position) < Radius;
}

const TBOOL CSphere::Intersect( const CPlane &p ) const
{
  return abs( p.Distance( Position ) ) < Radius;
}

CSphere::CSphere( const CVector3 &p, const float r )
{
  Position = p;
  Radius = r;
}

CSphere::CSphere()
{

}

CSphere::~CSphere()
{

}

const TBOOL CSphere::Contains( const CVector3 &p ) const
{
  return ( Position - p ).Length() < Radius;
}
