#pragma once

#include <cstdint>

#include "src/base/quaternion.h"
#include "src/base/vector.h"

class CMatrix4x4 {
  union {
    struct {
      float _11, _12, _13, _14;
      float _21, _22, _23, _24;
      float _31, _32, _33, _34;
      float _41, _42, _43, _44;
    };
    float m[4][4] = {{0}};
  };

 public:
  constexpr CMatrix4x4() : m{{0}} {}

  CMatrix4x4(const CMatrix4x4& mx);
  CMatrix4x4(float f11, float f12, float f13, float f14,  //
             float f21, float f22, float f23, float f24,  //
             float f31, float f32, float f33, float f34,  //
             float f41, float f42, float f43, float f44);
  explicit CMatrix4x4(const CQuaternion& q);

  float& operator()(uint32_t Row, uint32_t Col);
  float operator()(uint32_t Row, uint32_t Col) const;
  explicit operator float*();
  explicit operator const float*() const;
  CVector4 Row(int32_t x) const;
  CVector4 Col(int32_t x) const;
  CMatrix4x4& operator*=(const CMatrix4x4& mat);
  CMatrix4x4& operator+=(const CMatrix4x4& mat);
  CMatrix4x4& operator-=(const CMatrix4x4& mat);
  CMatrix4x4& operator*=(const float f);
  CMatrix4x4& operator/=(const float f);
  CMatrix4x4 operator+() const;
  CMatrix4x4 operator-() const;
  CMatrix4x4 operator*(const CMatrix4x4& mat) const;
  CMatrix4x4 operator+(const CMatrix4x4& mat) const;
  CMatrix4x4 operator-(const CMatrix4x4& mat) const;
  CMatrix4x4 operator*(const float f) const;
  CMatrix4x4 operator/(const float f) const;
  friend CMatrix4x4 operator*(const float f, const CMatrix4x4& mat);
  bool operator==(const CMatrix4x4& mat) const;
  bool operator!=(const CMatrix4x4& mat) const;
  void Transpose();
  CMatrix4x4 Transposed() const;
  CMatrix4x4& SetIdentity();
  CVector4 Apply(const CVector3& v) const;
  CVector4 Apply(const CVector4& v) const;
  float Determinant() const;
  void Invert();
  CMatrix4x4 Inverted() const;
  void SetLookAtLH(const CVector3& Eye, const CVector3& Target,
                   const CVector3& Up);
  void SetLookAtRH(const CVector3& Eye, const CVector3& Target,
                   const CVector3& Up);
  void SetOrthoLH(const float w, const float h, const float zn, const float zf);
  void SetOrthoRH(const float w, const float h, const float zn, const float zf);
  void SetPerspectiveFovLH(const float fovy, const float aspect, const float zn,
                           const float zf);
  void SetPerspectiveFovRH(const float fovy, const float aspect, const float zn,
                           const float zf);
  static CMatrix4x4 Translation(const CVector3& v);
  static CMatrix4x4 Scaling(const CVector3& v);
  static CMatrix4x4 Rotation(const CQuaternion& q);
  void SetTransformation(const CVector3& scaling, const CQuaternion& rotation,
                         const CVector3& translation);
};
