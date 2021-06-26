#pragma once

class CSphere
{
public:

  float Radius;
  CVector3 Position;

  CSphere();
  ~CSphere();
  CSphere( const CVector3 &p, const float r );

  const TBOOL Intersect( const CPlane &p ) const;
  const TBOOL Intersect( const CLine &l, float &tmin, float &tmax ) const;
  const TBOOL Contains( const CVector3 &p ) const;
};
