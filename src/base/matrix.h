#pragma once

#include <array>
#include <cstdint>

#include "src/base/vector.h"

namespace math {

class CMatrix4x4 {
  std::array<std::array<float, 4>, 4> m = {{0}};

 public:
  constexpr CMatrix4x4() : m{{0}} {}

  CMatrix4x4(const CMatrix4x4& mx);
  CMatrix4x4(float f11, float f12, float f13, float f14,  //
             float f21, float f22, float f23, float f24,  //
             float f31, float f32, float f33, float f34,  //
             float f41, float f42, float f43, float f44);

  constexpr const std::array<std::array<float, 4>, 4>& data() const {
    return m;
  }
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
  static CMatrix4x4 Rotation(const CVector3& Axis, const float Angle);
};

}  // namespace math
