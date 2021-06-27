#include "BaseLib.h"

void CMatrix4x4::SetTransformation( const CVector3 &scaling, const CQuaternion &rotation, const CVector3 &translation )
{
  *this = CMatrix4x4::Scaling( scaling )*CMatrix4x4::Rotation( rotation )*CMatrix4x4::Translation( translation );
}

CVector3 CMatrix4x4::GetScaling() const
{
  return CVector3( CVector3( Row( 0 ) ).Length(),
                   CVector3( Row( 1 ) ).Length(),
                   CVector3( Row( 2 ) ).Length() );
}

CVector3 CMatrix4x4::GetTranslation() const
{
  return Row( 3 );
}

CMatrix4x4 CMatrix4x4::Rotation( const CQuaternion &q )
{
  return CMatrix4x4( q );
}

CMatrix4x4 CMatrix4x4::Scaling( const CVector3 &v )
{
  CMatrix4x4 mx;
  mx.SetIdentity();
  mx._11 = v.x;
  mx._22 = v.y;
  mx._33 = v.z;
  return mx;
}

CMatrix4x4 CMatrix4x4::Translation( const CVector3 &v )
{
  CMatrix4x4 mx;
  mx.SetIdentity();
  mx._41 = v.x;
  mx._42 = v.y;
  mx._43 = v.z;
  return mx;
}

void CMatrix4x4::SetPerspectiveFovRH( const float fovy, const float aspect, const float zn, const float zf )
{
  SetIdentity();
  _11 = 1.0f / ( aspect*tanf( fovy / 2.0f ) );
  _22 = 1.0f / tanf( fovy / 2.0f );
  _33 = zf / ( zn - zf );
  _34 = -1.0f;
  _43 = ( zf*zn ) / ( zn - zf );
  _44 = 0.0f;
}

void CMatrix4x4::SetPerspectiveFovLH( const float fovy, const float aspect, const float zn, const float zf )
{
  SetIdentity();
  _11 = 1.0f / ( aspect*tanf( fovy / 2.0f ) );
  _22 = 1.0f / tanf( fovy / 2.0f );
  _33 = zf / ( zf - zn );
  _34 = 1.0f;
  _43 = ( zf*zn ) / ( zn - zf );
  _44 = 0.0f;
}

void CMatrix4x4::SetOrthoRH( const float w, const float h, const float zn, const float zf )
{
  SetIdentity();
  _11 = 2.0f / w;
  _22 = 2.0f / h;
  _33 = 1.0f / ( zn - zf );
  _43 = zn / ( zn - zf );
}

void CMatrix4x4::SetOrthoLH( const float w, const float h, const float zn, const float zf )
{
  SetIdentity();
  _11 = 2.0f / w;
  _22 = 2.0f / h;
  _33 = 1.0f / ( zf - zn );
  _43 = zn / ( zn - zf );
}

void CMatrix4x4::SetLookAtRH( const CVector3 &Eye, const CVector3 &Target, const CVector3 &Up )
{
  CVector3 &Z = Target - Eye;
  Z.Normalize();
  CVector3 &X = Up%Z;
  CVector3 &Y = Z%X;
  Y.Normalize();
  X.Normalize();

  _11 = -X.x;
  _21 = -X.y;
  _31 = -X.z;
  _41 = X*Eye;
  _12 = Y.x;
  _22 = Y.y;
  _32 = Y.z;
  _42 = -Y*Eye;
  _13 = -Z.x;
  _23 = -Z.y;
  _33 = -Z.z;
  _43 = Z*Eye;
  _14 = 0.0f;
  _24 = 0.0f;
  _34 = 0.0f;
  _44 = 1.0f;
}

void CMatrix4x4::SetLookAtLH( const CVector3 &Eye, const CVector3 &Target, const CVector3 &Up )
{
  CVector3 &Z = Target - Eye;
  Z.Normalize();
  CVector3 &X = Up%Z;
  CVector3 &Y = Z%X;
  Y.Normalize();
  X.Normalize();

  _11 = X.x;
  _21 = X.y;
  _31 = X.z;
  _41 = -X*Eye;
  _12 = Y.x;
  _22 = Y.y;
  _32 = Y.z;
  _42 = -Y*Eye;
  _13 = Z.x;
  _23 = Z.y;
  _33 = Z.z;
  _43 = -Z*Eye;
  _14 = 0.0f;
  _24 = 0.0f;
  _34 = 0.0f;
  _44 = 1.0f;
}

CMatrix4x4 CMatrix4x4::Inverted() const
{
  CMatrix4x4 out;
  CVector4 vec[ 3 ];

  float det = Determinant();

  BASEASSERT( det != 0 );

  for ( int32_t i = 0; i < 4; i++ )
  {
    for ( int32_t j = 0; j < 4; j++ )
      if ( i != j )
        vec[ j > i ? j - 1 : j ] = Row( j );

    CVector4 v = CVector4::Cross( vec[ 0 ], vec[ 1 ], vec[ 2 ] )*( powf( -1.0f, float(i) ) / det );

    out.m[ 0 ][ i ] = v.x;
    out.m[ 1 ][ i ] = v.y;
    out.m[ 2 ][ i ] = v.z;
    out.m[ 3 ][ i ] = v.w;
  }

  return out;
}

void CMatrix4x4::Invert()
{
  *this = Inverted();
}

float CMatrix4x4::Determinant() const
{
  return -Col( 3 )*CVector4::Cross( Col( 0 ), Col( 1 ), Col( 2 ) );
}

CVector4 CMatrix4x4::Apply( const CVector4 &v ) const
{
  CVector4 r;
  r.x = _11*v.x + _21*v.y + _31*v.z + _41*v.w;
  r.y = _12*v.x + _22*v.y + _32*v.z + _42*v.w;
  r.z = _13*v.x + _23*v.y + _33*v.z + _43*v.w;
  r.w = _14*v.x + _24*v.y + _34*v.z + _44*v.w;
  return r;
}

CVector4 CMatrix4x4::Apply( const CVector3 &v ) const
{
  CVector4 r;
  r.x = _11*v.x + _21*v.y + _31*v.z + _41;
  r.y = _12*v.x + _22*v.y + _32*v.z + _42;
  r.z = _13*v.x + _23*v.y + _33*v.z + _43;
  r.w = _14*v.x + _24*v.y + _34*v.z + _44;
  return r;
}

CMatrix4x4 & CMatrix4x4::SetIdentity()
{
  _11 = _22 = _33 = _44 = 1;
  _21 = _31 = _41 = _12 = _32 = _42 = _13 = _23 = _43 = _14 = _24 = _34 = 0;
  return *this;
}

CMatrix4x4 CMatrix4x4::Transposed() const
{
  return CMatrix4x4( _11, _21, _31, _41,
                     _12, _22, _32, _42,
                     _13, _23, _33, _43,
                     _14, _24, _34, _44 );
}

void CMatrix4x4::Transpose()
{
  *this = Transposed();
}

CMatrix4x4::CMatrix4x4( float f11, float f12, float f13, float f14, float f21, float f22, float f23, float f24, float f31, float f32, float f33, float f34, float f41, float f42, float f43, float f44 )
{
  _11 = f11; _12 = f12; _13 = f13; _14 = f14;
  _21 = f21; _22 = f22; _23 = f23; _24 = f24;
  _31 = f31; _32 = f32; _33 = f33; _34 = f34;
  _41 = f41; _42 = f42; _43 = f43; _44 = f44;
}

CMatrix4x4::CMatrix4x4( const CMatrix4x4 &mx )
{
  memcpy( &_11, &mx._11, 16 * sizeof( float ) );
}

CMatrix4x4::CMatrix4x4( const float *f )
{
  BASEASSERT( f );
  if ( !f ) return;
  memcpy( &_11, f, 16 * sizeof( float ) );
}

CMatrix4x4::CMatrix4x4()
{

}

float &CMatrix4x4::operator() ( uint32_t Row, uint32_t Col )
{
  BASEASSERTR( Row >= 0 && Col >= 0 && Row < 4 && Col < 4, m[ 0 ][ 0 ] );
  return m[ Row & 3 ][ Col & 3 ];
}

float CMatrix4x4::operator() ( uint32_t Row, uint32_t Col ) const
{
  BASEASSERTR( Row >= 0 && Col >= 0 && Row < 4 && Col < 4, m[ 0 ][ 0 ] );
  return m[ Row & 3 ][ Col & 3 ];
}

CMatrix4x4::operator float*( )
{
  return &_11;
}

CMatrix4x4::operator const float*( ) const
{
  return (const float*)&_11;
}

CVector4 CMatrix4x4::Row( int32_t x ) const
{
  BASEASSERTR( x >= 0 && x < 4, m[ 0 ] );
  return m[ x & 3 ];
}

CVector4 CMatrix4x4::Col( int32_t x ) const
{
  BASEASSERTR( x >= 0 && x < 4, m[ 0 ] );
  return CVector4( m[ 0 ][ x & 3 ], m[ 1 ][ x & 3 ], m[ 2 ][ x & 3 ], m[ 3 ][ x & 3 ] );
}

CMatrix4x4 &CMatrix4x4::operator*= ( const CMatrix4x4 &mat )
{
  return *this = ( *this )*mat;
}

CMatrix4x4 &CMatrix4x4::operator+= ( const CMatrix4x4 &mat )
{
  _11 += mat._11; _12 += mat._12; _13 += mat._13; _14 += mat._14;
  _21 += mat._21; _22 += mat._22; _23 += mat._23; _24 += mat._24;
  _31 += mat._31; _32 += mat._32; _33 += mat._33; _34 += mat._34;
  _41 += mat._41; _42 += mat._42; _43 += mat._43; _44 += mat._44;
  return *this;
}

CMatrix4x4 &CMatrix4x4::operator-= ( const CMatrix4x4 &mat )
{
  _11 -= mat._11; _12 -= mat._12; _13 -= mat._13; _14 -= mat._14;
  _21 -= mat._21; _22 -= mat._22; _23 -= mat._23; _24 -= mat._24;
  _31 -= mat._31; _32 -= mat._32; _33 -= mat._33; _34 -= mat._34;
  _41 -= mat._41; _42 -= mat._42; _43 -= mat._43; _44 -= mat._44;
  return *this;
}

CMatrix4x4 &CMatrix4x4::operator*= ( const float f )
{
  _11 *= f; _12 *= f; _13 *= f; _14 *= f;
  _21 *= f; _22 *= f; _23 *= f; _24 *= f;
  _31 *= f; _32 *= f; _33 *= f; _34 *= f;
  _41 *= f; _42 *= f; _43 *= f; _44 *= f;
  return *this;
}

CMatrix4x4 &CMatrix4x4::operator/= ( const float f )
{
  float fInv = 1.0f / f;
  _11 *= fInv; _12 *= fInv; _13 *= fInv; _14 *= fInv;
  _21 *= fInv; _22 *= fInv; _23 *= fInv; _24 *= fInv;
  _31 *= fInv; _32 *= fInv; _33 *= fInv; _34 *= fInv;
  _41 *= fInv; _42 *= fInv; _43 *= fInv; _44 *= fInv;
  return *this;
}

CMatrix4x4 CMatrix4x4::operator+ () const
{
  return *this;
}

CMatrix4x4 CMatrix4x4::operator- () const
{
  return CMatrix4x4( -_11, -_12, -_13, -_14,
                     -_21, -_22, -_23, -_24,
                     -_31, -_32, -_33, -_34,
                     -_41, -_42, -_43, -_44 );
}

CMatrix4x4 CMatrix4x4::operator *( const CMatrix4x4& mat ) const
{
  CMatrix4x4 matT;
  for ( int32_t i = 0; i < 4; i++ )
    for ( int32_t j = 0; j < 4; j++ )
      matT.m[ i ][ j ] = m[ i ][ 0 ] * mat.m[ 0 ][ j ] + m[ i ][ 1 ] * mat.m[ 1 ][ j ] + m[ i ][ 2 ] * mat.m[ 2 ][ j ] + m[ i ][ 3 ] * mat.m[ 3 ][ j ];
  return matT;
}

CMatrix4x4 CMatrix4x4::operator +( const CMatrix4x4& mat ) const
{
  return CMatrix4x4( _11 + mat._11, _12 + mat._12, _13 + mat._13, _14 + mat._14,
                     _21 + mat._21, _22 + mat._22, _23 + mat._23, _24 + mat._24,
                     _31 + mat._31, _32 + mat._32, _33 + mat._33, _34 + mat._34,
                     _41 + mat._41, _42 + mat._42, _43 + mat._43, _44 + mat._44 );
}

CMatrix4x4 CMatrix4x4::operator -( const CMatrix4x4& mat ) const
{
  return CMatrix4x4( _11 - mat._11, _12 - mat._12, _13 - mat._13, _14 - mat._14,
                     _21 - mat._21, _22 - mat._22, _23 - mat._23, _24 - mat._24,
                     _31 - mat._31, _32 - mat._32, _33 - mat._33, _34 - mat._34,
                     _41 - mat._41, _42 - mat._42, _43 - mat._43, _44 - mat._44 );
}

CMatrix4x4 CMatrix4x4::operator *( const float f ) const
{
  return CMatrix4x4( _11 * f, _12 * f, _13 * f, _14 * f,
                     _21 * f, _22 * f, _23 * f, _24 * f,
                     _31 * f, _32 * f, _33 * f, _34 * f,
                     _41 * f, _42 * f, _43 * f, _44 * f );
}

CMatrix4x4 CMatrix4x4::operator /( const float f ) const
{
  float fInv = 1.0f / f;
  return CMatrix4x4( _11 * fInv, _12 * fInv, _13 * fInv, _14 * fInv,
                     _21 * fInv, _22 * fInv, _23 * fInv, _24 * fInv,
                     _31 * fInv, _32 * fInv, _33 * fInv, _34 * fInv,
                     _41 * fInv, _42 * fInv, _43 * fInv, _44 * fInv );
}

CMatrix4x4 operator *( const float f, const CMatrix4x4& mat )
{
  return CMatrix4x4( f * mat._11, f * mat._12, f * mat._13, f * mat._14,
                     f * mat._21, f * mat._22, f * mat._23, f * mat._24,
                     f * mat._31, f * mat._32, f * mat._33, f * mat._34,
                     f * mat._41, f * mat._42, f * mat._43, f * mat._44 );
}

TBOOL CMatrix4x4::operator ==( const CMatrix4x4& mat ) const
{
  return 0 == memcmp( &_11, &mat._11, 16 * sizeof( float ) );
}

TBOOL CMatrix4x4::operator !=( const CMatrix4x4& mat ) const
{
  return 0 != memcmp( &_11, &mat._11, 16 * sizeof( float ) );
}

CMatrix4x4::CMatrix4x4( const CQuaternion &q )
{
  _11 = 1.0f - 2.0f*( q.y*q.y + q.z*q.z );
  _12 = 2.0f*( q.x*q.y + q.z*q.s );
  _13 = 2.0f*( q.x*q.z - q.y*q.s );
  _14 = 0;
  _21 = 2.0f*( q.x*q.y - q.z*q.s );
  _22 = 1.0f - 2.0f*( q.x*q.x + q.z*q.z );
  _23 = 2.0f*( q.y*q.z + q.x*q.s );
  _24 = 0;
  _31 = 2.0f*( q.x*q.z + q.y*q.s );
  _32 = 2.0f*( q.y*q.z - q.x*q.s );
  _33 = 1.0f - 2.0f*( q.x*q.x + q.y*q.y );
  _34 = 0;
  _41 = _42 = _43 = 0;
  _44 = 1;
}

void CMatrix4x4::Decompose( CVector3 &Scale, CQuaternion &Rotation, CVector3 &Translation ) const
{
  CMatrix4x4 normalized;

  Scale = CVector3( CVector3( Row( 0 ) ).Length(),
                    CVector3( Row( 1 ) ).Length(),
                    CVector3( Row( 2 ) ).Length() );

  Translation = Row( 3 );

  BASEASSERT( Scale.x != 0.0f || Scale.y != 0.0f || Scale.z != 0.0f );

  normalized._11 = _11 / Scale.x;
  normalized._12 = _12 / Scale.x;
  normalized._13 = _13 / Scale.x;
  normalized._21 = _21 / Scale.y;
  normalized._22 = _22 / Scale.y;
  normalized._23 = _23 / Scale.y;
  normalized._31 = _31 / Scale.z;
  normalized._32 = _32 / Scale.z;
  normalized._33 = _33 / Scale.z;

  Rotation.FromRotationMatrix( normalized );
}

CQuaternion CMatrix4x4::GetRotation() const
{
  CQuaternion q;
  q.FromRotationMatrix( *this );
  return q;
}
