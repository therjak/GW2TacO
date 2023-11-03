#pragma once

#include <array>
#include <cstdint>

#include "src/base/vector.h"

namespace math {

class CMatrix4x4 {
 public:
  constexpr CMatrix4x4() : m{{{0}}} {}

  constexpr CMatrix4x4(const CMatrix4x4& mx) { m = mx.m; }
  constexpr CMatrix4x4(float f11, float f12, float f13, float f14,  //
                       float f21, float f22, float f23, float f24,  //
                       float f31, float f32, float f33, float f34,  //
                       float f41, float f42, float f43, float f44);

  [[nodiscard]] constexpr const std::array<std::array<float, 4>, 4>& data()
      const {
    return m;
  }

  [[nodiscard]] constexpr CVector4 Row(int32_t x) const {
    return CVector4(m[x & 3][0], m[x & 3][1], m[x & 3][2], m[x & 3][3]);
  }
  [[nodiscard]] constexpr CVector4 Col(int32_t x) const {
    return CVector4(m[0][x & 3], m[1][x & 3], m[2][x & 3], m[3][x & 3]);
  }
  constexpr CMatrix4x4& operator*=(const CMatrix4x4& mat) {
    return *this = (*this) * mat;
  }
  constexpr CMatrix4x4& operator+=(const CMatrix4x4& mat);
  constexpr CMatrix4x4& operator-=(const CMatrix4x4& mat);
  constexpr CMatrix4x4& operator*=(const float f);
  constexpr CMatrix4x4& operator/=(const float f);
  constexpr CMatrix4x4 operator+() const;
  constexpr CMatrix4x4 operator-() const;
  constexpr CMatrix4x4 operator*(const CMatrix4x4& mat) const;
  constexpr CMatrix4x4 operator+(const CMatrix4x4& mat) const;
  constexpr CMatrix4x4 operator-(const CMatrix4x4& mat) const;
  constexpr CMatrix4x4 operator*(const float f) const;
  constexpr CMatrix4x4 operator/(const float f) const;
  constexpr friend CMatrix4x4 operator*(const float f, const CMatrix4x4& mat);
  constexpr friend bool operator==(const CMatrix4x4& lhs,
                                   const CMatrix4x4& rhs) = default;

  constexpr void Transpose();
  [[nodiscard]] constexpr CMatrix4x4 Transposed() const;
  constexpr CMatrix4x4& SetIdentity();
  [[nodiscard]] constexpr CVector4 Apply(const CVector3& v) const;
  [[nodiscard]] constexpr CVector4 Apply(const CVector4& v) const;
  [[nodiscard]] constexpr float Determinant() const {
    return -Col(3) * CVector4::Cross(Col(0), Col(1), Col(2));
  }
  void Invert();
  [[nodiscard]] CMatrix4x4 Inverted() const;
  void SetLookAtLH(const CVector3& Eye, const CVector3& Target,
                   const CVector3& Up);
  void SetLookAtRH(const CVector3& Eye, const CVector3& Target,
                   const CVector3& Up);
  constexpr void SetOrthoLH(const float w, const float h, const float zn,
                            const float zf);
  constexpr void SetOrthoRH(const float w, const float h, const float zn,
                            const float zf);
  void SetPerspectiveFovLH(const float fovy, const float aspect, const float zn,
                           const float zf);
  void SetPerspectiveFovRH(const float fovy, const float aspect, const float zn,
                           const float zf);
  constexpr static CMatrix4x4 Translation(const CVector3& v);
  constexpr static CMatrix4x4 Scaling(const CVector3& v);
  static CMatrix4x4 Rotation(const CVector3& Axis, const float Angle);

 private:
  std::array<std::array<float, 4>, 4> m = {{{0}}};
};

constexpr CMatrix4x4::CMatrix4x4(float f11, float f12, float f13, float f14,
                                 float f21, float f22, float f23, float f24,
                                 float f31, float f32, float f33, float f34,
                                 float f41, float f42, float f43, float f44) {
  m[0][0] = f11;
  m[0][1] = f12;
  m[0][2] = f13;
  m[0][3] = f14;
  m[1][0] = f21;
  m[1][1] = f22;
  m[1][2] = f23;
  m[1][3] = f24;
  m[2][0] = f31;
  m[2][1] = f32;
  m[2][2] = f33;
  m[2][3] = f34;
  m[3][0] = f41;
  m[3][1] = f42;
  m[3][2] = f43;
  m[3][3] = f44;
}

constexpr CVector4 CMatrix4x4::Apply(const CVector4& v) const {
  CVector4 r;
  r.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.w;
  r.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.w;
  r.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.w;
  r.w = m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3] * v.w;
  return r;
}

constexpr CVector4 CMatrix4x4::Apply(const CVector3& v) const {
  CVector4 r;
  r.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0];
  r.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1];
  r.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2];
  r.w = m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3];
  return r;
}

constexpr CMatrix4x4& CMatrix4x4::SetIdentity() {
  m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1;
  m[1][0] = m[2][0] = m[3][0] = m[0][1] = m[2][1] = m[3][1] = m[0][2] =
      m[1][2] = m[3][2] = m[0][3] = m[1][3] = m[2][3] = 0;
  return *this;
}

constexpr CMatrix4x4 CMatrix4x4::Transposed() const {
  return {m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1],
          m[2][1], m[3][1], m[0][2], m[1][2], m[2][2], m[3][2],
          m[0][3], m[1][3], m[2][3], m[3][3]};
}

constexpr void CMatrix4x4::Transpose() { *this = Transposed(); }

constexpr CMatrix4x4 CMatrix4x4::Scaling(const CVector3& v) {
  CMatrix4x4 mx;
  mx.SetIdentity();
  mx.m[0][0] = v.x;
  mx.m[1][1] = v.y;
  mx.m[2][2] = v.z;
  return mx;
}

constexpr CMatrix4x4 CMatrix4x4::Translation(const CVector3& v) {
  CMatrix4x4 mx;
  mx.SetIdentity();
  mx.m[3][0] = v.x;
  mx.m[3][1] = v.y;
  mx.m[3][2] = v.z;
  return mx;
}

constexpr void CMatrix4x4::SetOrthoRH(const float w, const float h,
                                      const float zn, const float zf) {
  SetIdentity();
  m[0][0] = 2.0f / w;
  m[1][1] = 2.0f / h;
  m[2][2] = 1.0f / (zn - zf);
  m[3][2] = zn / (zn - zf);
}

constexpr void CMatrix4x4::SetOrthoLH(const float w, const float h,
                                      const float zn, const float zf) {
  SetIdentity();
  m[0][0] = 2.0f / w;
  m[1][1] = 2.0f / h;
  m[2][2] = 1.0f / (zf - zn);
  m[3][2] = zn / (zn - zf);
}

constexpr CMatrix4x4& CMatrix4x4::operator+=(const CMatrix4x4& mat) {
  m[0][0] += mat.m[0][0];
  m[0][1] += mat.m[0][1];
  m[0][2] += mat.m[0][2];
  m[0][3] += mat.m[0][3];
  m[1][0] += mat.m[1][0];
  m[1][1] += mat.m[1][1];
  m[1][2] += mat.m[1][2];
  m[1][3] += mat.m[1][3];
  m[2][0] += mat.m[2][0];
  m[2][1] += mat.m[2][1];
  m[2][2] += mat.m[2][2];
  m[2][3] += mat.m[2][3];
  m[3][0] += mat.m[3][0];
  m[3][1] += mat.m[3][1];
  m[3][2] += mat.m[3][2];
  m[3][3] += mat.m[3][3];
  return *this;
}

constexpr CMatrix4x4& CMatrix4x4::operator-=(const CMatrix4x4& mat) {
  m[0][0] -= mat.m[0][0];
  m[0][1] -= mat.m[0][1];
  m[0][2] -= mat.m[0][2];
  m[0][3] -= mat.m[0][3];
  m[1][0] -= mat.m[1][0];
  m[1][1] -= mat.m[1][1];
  m[1][2] -= mat.m[1][2];
  m[1][3] -= mat.m[1][3];
  m[2][0] -= mat.m[2][0];
  m[2][1] -= mat.m[2][1];
  m[2][2] -= mat.m[2][2];
  m[2][3] -= mat.m[2][3];
  m[3][0] -= mat.m[3][0];
  m[3][1] -= mat.m[3][1];
  m[3][2] -= mat.m[3][2];
  m[3][3] -= mat.m[3][3];
  return *this;
}

constexpr CMatrix4x4& CMatrix4x4::operator*=(const float f) {
  m[0][0] *= f;
  m[0][1] *= f;
  m[0][2] *= f;
  m[0][3] *= f;
  m[1][0] *= f;
  m[1][1] *= f;
  m[1][2] *= f;
  m[1][3] *= f;
  m[2][0] *= f;
  m[2][1] *= f;
  m[2][2] *= f;
  m[2][3] *= f;
  m[3][0] *= f;
  m[3][1] *= f;
  m[3][2] *= f;
  m[3][3] *= f;
  return *this;
}

constexpr CMatrix4x4& CMatrix4x4::operator/=(const float f) {
  float fInv = 1.0f / f;
  m[0][0] *= fInv;
  m[0][1] *= fInv;
  m[0][2] *= fInv;
  m[0][3] *= fInv;
  m[1][0] *= fInv;
  m[1][1] *= fInv;
  m[1][2] *= fInv;
  m[1][3] *= fInv;
  m[2][0] *= fInv;
  m[2][1] *= fInv;
  m[2][2] *= fInv;
  m[2][3] *= fInv;
  m[3][0] *= fInv;
  m[3][1] *= fInv;
  m[3][2] *= fInv;
  m[3][3] *= fInv;
  return *this;
}

constexpr CMatrix4x4 CMatrix4x4::operator+() const { return *this; }

constexpr CMatrix4x4 CMatrix4x4::operator-() const {
  return {-m[0][0], -m[0][1], -m[0][2], -m[0][3], -m[1][0], -m[1][1],
          -m[1][2], -m[1][3], -m[2][0], -m[2][1], -m[2][2], -m[2][3],
          -m[3][0], -m[3][1], -m[3][2], -m[3][3]};
}

constexpr CMatrix4x4 CMatrix4x4::operator*(const CMatrix4x4& mat) const {
  CMatrix4x4 matT;
  for (int32_t i = 0; i < 4; i++) {
    for (int32_t j = 0; j < 4; j++) {
      matT.m[i][j] = m[i][0] * mat.m[0][j] + m[i][1] * mat.m[1][j] +
                     m[i][2] * mat.m[2][j] + m[i][3] * mat.m[3][j];
    }
  }
  return matT;
}

constexpr CMatrix4x4 CMatrix4x4::operator+(const CMatrix4x4& mat) const {
  return {m[0][0] + mat.m[0][0], m[0][1] + mat.m[0][1], m[0][2] + mat.m[0][2],
          m[0][3] + mat.m[0][3], m[1][0] + mat.m[1][0], m[1][1] + mat.m[1][1],
          m[1][2] + mat.m[1][2], m[1][3] + mat.m[1][3], m[2][0] + mat.m[2][0],
          m[2][1] + mat.m[2][1], m[2][2] + mat.m[2][2], m[2][3] + mat.m[2][3],
          m[3][0] + mat.m[3][0], m[3][1] + mat.m[3][1], m[3][2] + mat.m[3][2],
          m[3][3] + mat.m[3][3]};
}

constexpr CMatrix4x4 CMatrix4x4::operator-(const CMatrix4x4& mat) const {
  return {m[0][0] - mat.m[0][0], m[0][1] - mat.m[0][1], m[0][2] - mat.m[0][2],
          m[0][3] - mat.m[0][3], m[1][0] - mat.m[1][0], m[1][1] - mat.m[1][1],
          m[1][2] - mat.m[1][2], m[1][3] - mat.m[1][3], m[2][0] - mat.m[2][0],
          m[2][1] - mat.m[2][1], m[2][2] - mat.m[2][2], m[2][3] - mat.m[2][3],
          m[3][0] - mat.m[3][0], m[3][1] - mat.m[3][1], m[3][2] - mat.m[3][2],
          m[3][3] - mat.m[3][3]};
}

constexpr CMatrix4x4 CMatrix4x4::operator*(const float f) const {
  return {m[0][0] * f, m[0][1] * f, m[0][2] * f, m[0][3] * f,
          m[1][0] * f, m[1][1] * f, m[1][2] * f, m[1][3] * f,
          m[2][0] * f, m[2][1] * f, m[2][2] * f, m[2][3] * f,
          m[3][0] * f, m[3][1] * f, m[3][2] * f, m[3][3] * f};
}

constexpr CMatrix4x4 CMatrix4x4::operator/(const float f) const {
  float fInv = 1.0f / f;
  return {m[0][0] * fInv, m[0][1] * fInv, m[0][2] * fInv, m[0][3] * fInv,
          m[1][0] * fInv, m[1][1] * fInv, m[1][2] * fInv, m[1][3] * fInv,
          m[2][0] * fInv, m[2][1] * fInv, m[2][2] * fInv, m[2][3] * fInv,
          m[3][0] * fInv, m[3][1] * fInv, m[3][2] * fInv, m[3][3] * fInv};
}

constexpr CMatrix4x4 operator*(const float f, const CMatrix4x4& mat) {
  return {f * mat.m[0][0], f * mat.m[0][1], f * mat.m[0][2], f * mat.m[0][3],
          f * mat.m[1][0], f * mat.m[1][1], f * mat.m[1][2], f * mat.m[1][3],
          f * mat.m[2][0], f * mat.m[2][1], f * mat.m[2][2], f * mat.m[2][3],
          f * mat.m[3][0], f * mat.m[3][1], f * mat.m[3][2], f * mat.m[3][3]};
}

}  // namespace math
