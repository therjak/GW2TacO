#pragma once

class CPlane
{
  void Normalize();

public:

  CVector3 Normal;
  float D;

  CPlane();
  CPlane( const CVector3 &Point, const CVector3 &n );
  CPlane( const CVector3 &a, const CVector3 &b, const CVector3 &c );

  CVector3 Project( const CVector3 &v ) const;
  float Distance( const CVector3 &v ) const;
  int32_t Side( const CVector3 &v ) const;

};
