#pragma once

class CQuaternion;

class CMatrix2x2
{
  union
  {
    struct
    {
      float _11, _12;
      float _21, _22;
    };
    float m[ 2 ][ 2 ];
  };

public:

  CMatrix2x2();
  CMatrix2x2( const float *f );
  CMatrix2x2( const CMatrix2x2 &mx );
  CMatrix2x2( float f11, float f12,
              float f21, float f22 );

  CMatrix2x2( const CQuaternion &q );

  float &operator() ( uint32_t Row, uint32_t Col );
  float operator() ( uint32_t Row, uint32_t Col ) const;
  operator float*( );
  operator const float*( ) const;

  CMatrix2x2 &operator*= ( const CMatrix2x2 &mat );
  CMatrix2x2 &operator+= ( const CMatrix2x2 &mat );
  CMatrix2x2 &operator-= ( const CMatrix2x2 &mat );
  CMatrix2x2 &operator*= ( const float f );
  CMatrix2x2 &operator/= ( const float f );
  CMatrix2x2 operator+ () const;
  CMatrix2x2 operator- () const;
  CMatrix2x2 operator *( const CMatrix2x2& mat ) const;
  CMatrix2x2 operator +( const CMatrix2x2& mat ) const;
  CMatrix2x2 operator -( const CMatrix2x2& mat ) const;
  CMatrix2x2 operator *( const float f ) const;
  CMatrix2x2 operator /( const float f ) const;
  friend CMatrix2x2 operator *( const float f, const CMatrix2x2& mat );
  TBOOL operator ==( const CMatrix2x2& mat ) const;
  TBOOL operator !=( const CMatrix2x2& mat ) const;

  void Transpose();
  CMatrix2x2 Transposed() const;
  CMatrix2x2 &SetIdentity();
  CVector2 Apply( const CVector2 &v ) const;
  void Invert();
  CMatrix2x2 Inverted() const;
};


class CMatrix3x3
{
  union
  {
    struct
    {
      float _11, _12, _13;
      float _21, _22, _23;
      float _31, _32, _33;
    };
    float m[ 3 ][ 3 ];
  };

public:

  CMatrix3x3();;
  CMatrix3x3( const float *f );
  CMatrix3x3( const CMatrix3x3 &mx );
  CMatrix3x3( float f11, float f12, float f13,
              float f21, float f22, float f23,
              float f31, float f32, float f33 );
  CMatrix3x3( const CQuaternion &q );

  float &operator() ( uint32_t Row, uint32_t Col );
  float operator() ( uint32_t Row, uint32_t Col ) const;
  operator float*( );
  operator const float*( ) const;
  CMatrix3x3 &operator*= ( const CMatrix3x3 &mat );
  CMatrix3x3 &operator+= ( const CMatrix3x3 &mat );
  CMatrix3x3 &operator-= ( const CMatrix3x3 &mat );
  CMatrix3x3 &operator*= ( const float f );
  CMatrix3x3 &operator/= ( const float f );
  CMatrix3x3 operator+ () const;
  CMatrix3x3 operator- () const;
  CMatrix3x3 operator *( const CMatrix3x3& mat ) const;
  CMatrix3x3 operator +( const CMatrix3x3& mat ) const;
  CMatrix3x3 operator -( const CMatrix3x3& mat ) const;
  CMatrix3x3 operator *( const float f ) const;
  CMatrix3x3 operator /( const float f ) const;
  friend CMatrix3x3 operator *( const float f, const CMatrix3x3& mat );
  TBOOL operator ==( const CMatrix3x3& mat ) const;
  TBOOL operator !=( const CMatrix3x3& mat ) const;
  void Transpose();
  CMatrix3x3 Transposed() const;
  CMatrix3x3 &SetIdentity();
  CVector3 Apply( const CVector3 &v ) const;
};

class CMatrix4x4
{
  union
  {
    struct
    {
      float _11, _12, _13, _14;
      float _21, _22, _23, _24;
      float _31, _32, _33, _34;
      float _41, _42, _43, _44;
    };
    float m[ 4 ][ 4 ];
  };

public:

  CMatrix4x4();;
  CMatrix4x4( const float *f );
  CMatrix4x4( const CMatrix4x4 &mx );
  CMatrix4x4( float f11, float f12, float f13, float f14,
              float f21, float f22, float f23, float f24,
              float f31, float f32, float f33, float f34,
              float f41, float f42, float f43, float f44 );
  CMatrix4x4( const CQuaternion &q );

  float &operator() ( uint32_t Row, uint32_t Col );
  float operator() ( uint32_t Row, uint32_t Col ) const;
  operator float*( );
  operator const float*( ) const;
  CVector4 Row( int32_t x ) const;
  CVector4 Col( int32_t x ) const;
  CMatrix4x4 &operator*= ( const CMatrix4x4 &mat );
  CMatrix4x4 &operator+= ( const CMatrix4x4 &mat );
  CMatrix4x4 &operator-= ( const CMatrix4x4 &mat );
  CMatrix4x4 &operator*= ( const float f );
  CMatrix4x4 &operator/= ( const float f );
  CMatrix4x4 operator+ () const;
  CMatrix4x4 operator- () const;
  CMatrix4x4 operator *( const CMatrix4x4& mat ) const;
  CMatrix4x4 operator +( const CMatrix4x4& mat ) const;
  CMatrix4x4 operator -( const CMatrix4x4& mat ) const;
  CMatrix4x4 operator *( const float f ) const;
  CMatrix4x4 operator /( const float f ) const;
  friend CMatrix4x4 operator *( const float f, const CMatrix4x4& mat );
  TBOOL operator ==( const CMatrix4x4& mat ) const;
  TBOOL operator !=( const CMatrix4x4& mat ) const;
  void Transpose();
  CMatrix4x4 Transposed() const;
  CMatrix4x4 &SetIdentity();
  CVector4 Apply( const CVector3 &v ) const;
  CVector4 Apply( const CVector4 &v ) const;
  void Decompose( CVector3 &Scale, CQuaternion &Rotation, CVector3 &Translation ) const;
  float Determinant() const;
  void Invert();
  CMatrix4x4 Inverted() const;
  void SetLookAtLH( const CVector3 &Eye, const CVector3 &Target, const CVector3 &Up );
  void SetLookAtRH( const CVector3 &Eye, const CVector3 &Target, const CVector3 &Up );
  void SetOrthoLH( const float w, const float h, const float zn, const float zf );
  void SetOrthoRH( const float w, const float h, const float zn, const float zf );
  void SetPerspectiveFovLH( const float fovy, const float aspect, const float zn, const float zf );
  void SetPerspectiveFovRH( const float fovy, const float aspect, const float zn, const float zf );
  static CMatrix4x4 CMatrix4x4::Translation( const CVector3 &v );
  static CMatrix4x4 CMatrix4x4::Scaling( const CVector3 &v );
  static CMatrix4x4 CMatrix4x4::Rotation( const CQuaternion &q );
  CVector3 GetTranslation() const;
  CVector3 GetScaling() const;
  CQuaternion GetRotation() const;
  void SetTransformation( const CVector3 &scaling, const CQuaternion &rotation, const CVector3 &translation );
};
