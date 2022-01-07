#include "src/base/matrix.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "src/base/assert.h"
#include "src/base/vector.h"

namespace math {

CMatrix4x4 CMatrix4x4::Scaling(const CVector3& v) {
  CMatrix4x4 mx;
  mx.SetIdentity();
  mx.m[0][0] = v.x;
  mx.m[1][1] = v.y;
  mx.m[2][2] = v.z;
  return mx;
}

CMatrix4x4 CMatrix4x4::Translation(const CVector3& v) {
  CMatrix4x4 mx;
  mx.SetIdentity();
  mx.m[3][0] = v.x;
  mx.m[3][1] = v.y;
  mx.m[3][2] = v.z;
  return mx;
}

void CMatrix4x4::SetPerspectiveFovRH(const float fovy, const float aspect,
                                     const float zn, const float zf) {
  SetIdentity();
  m[0][0] = 1.0f / (aspect * tanf(fovy / 2.0f));
  m[1][1] = 1.0f / tanf(fovy / 2.0f);
  m[2][2] = zf / (zn - zf);
  m[2][3] = -1.0f;
  m[3][2] = (zf * zn) / (zn - zf);
  m[3][3] = 0.0f;
}

void CMatrix4x4::SetPerspectiveFovLH(const float fovy, const float aspect,
                                     const float zn, const float zf) {
  SetIdentity();
  m[0][0] = 1.0f / (aspect * tanf(fovy / 2.0f));
  m[1][1] = 1.0f / tanf(fovy / 2.0f);
  m[2][2] = zf / (zf - zn);
  m[2][3] = 1.0f;
  m[3][2] = (zf * zn) / (zn - zf);
  m[3][3] = 0.0f;
}

void CMatrix4x4::SetOrthoRH(const float w, const float h, const float zn,
                            const float zf) {
  SetIdentity();
  m[0][0] = 2.0f / w;
  m[1][1] = 2.0f / h;
  m[2][2] = 1.0f / (zn - zf);
  m[3][2] = zn / (zn - zf);
}

void CMatrix4x4::SetOrthoLH(const float w, const float h, const float zn,
                            const float zf) {
  SetIdentity();
  m[0][0] = 2.0f / w;
  m[1][1] = 2.0f / h;
  m[2][2] = 1.0f / (zf - zn);
  m[3][2] = zn / (zn - zf);
}

void CMatrix4x4::SetLookAtRH(const CVector3& Eye, const CVector3& Target,
                             const CVector3& Up) {
  CVector3 Z = Target - Eye;
  Z.Normalize();
  CVector3 X = Up % Z;
  CVector3 Y = Z % X;
  Y.Normalize();
  X.Normalize();

  m[0][0] = -X.x;
  m[1][0] = -X.y;
  m[2][0] = -X.z;
  m[3][0] = X * Eye;
  m[0][1] = Y.x;
  m[1][1] = Y.y;
  m[2][1] = Y.z;
  m[3][1] = -Y * Eye;
  m[0][2] = -Z.x;
  m[1][2] = -Z.y;
  m[2][2] = -Z.z;
  m[3][2] = Z * Eye;
  m[0][3] = 0.0f;
  m[1][3] = 0.0f;
  m[2][3] = 0.0f;
  m[3][3] = 1.0f;
}

void CMatrix4x4::SetLookAtLH(const CVector3& Eye, const CVector3& Target,
                             const CVector3& Up) {
  CVector3 Z = Target - Eye;
  Z.Normalize();
  CVector3 X = Up % Z;
  CVector3 Y = Z % X;
  Y.Normalize();
  X.Normalize();

  m[0][0] = X.x;
  m[1][0] = X.y;
  m[2][0] = X.z;
  m[3][0] = -X * Eye;
  m[0][1] = Y.x;
  m[1][1] = Y.y;
  m[2][1] = Y.z;
  m[3][1] = -Y * Eye;
  m[0][2] = Z.x;
  m[1][2] = Z.y;
  m[2][2] = Z.z;
  m[3][2] = -Z * Eye;
  m[0][3] = 0.0f;
  m[1][3] = 0.0f;
  m[2][3] = 0.0f;
  m[3][3] = 1.0f;
}

CMatrix4x4 CMatrix4x4::Inverted() const {
  CMatrix4x4 out;
  CVector4 vec[3];

  float det = Determinant();

  BASEASSERT(det != 0);

  for (int32_t i = 0; i < 4; i++) {
    for (int32_t j = 0; j < 4; j++)
      if (i != j) vec[j > i ? j - 1 : j] = Row(j);

    CVector4 v =
        CVector4::Cross(vec[0], vec[1], vec[2]) * (powf(-1.0f, float(i)) / det);

    out.m[0][i] = v.x;
    out.m[1][i] = v.y;
    out.m[2][i] = v.z;
    out.m[3][i] = v.w;
  }

  return out;
}

void CMatrix4x4::Invert() { *this = Inverted(); }

float CMatrix4x4::Determinant() const {
  return -Col(3) * CVector4::Cross(Col(0), Col(1), Col(2));
}

CVector4 CMatrix4x4::Apply(const CVector4& v) const {
  CVector4 r;
  r.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.w;
  r.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.w;
  r.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.w;
  r.w = m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3] * v.w;
  return r;
}

CVector4 CMatrix4x4::Apply(const CVector3& v) const {
  CVector4 r;
  r.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0];
  r.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1];
  r.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2];
  r.w = m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3];
  return r;
}

CMatrix4x4& CMatrix4x4::SetIdentity() {
  m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1;
  m[1][0] = m[2][0] = m[3][0] = m[0][1] = m[2][1] = m[3][1] = m[0][2] =
      m[1][2] = m[3][2] = m[0][3] = m[1][3] = m[2][3] = 0;
  return *this;
}

CMatrix4x4 CMatrix4x4::Transposed() const {
  return CMatrix4x4(m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1],
                    m[2][1], m[3][1], m[0][2], m[1][2], m[2][2], m[3][2],
                    m[0][3], m[1][3], m[2][3], m[3][3]);
}

void CMatrix4x4::Transpose() { *this = Transposed(); }

CMatrix4x4::CMatrix4x4(float f11, float f12, float f13, float f14, float f21,
                       float f22, float f23, float f24, float f31, float f32,
                       float f33, float f34, float f41, float f42, float f43,
                       float f44) {
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

CMatrix4x4::CMatrix4x4(const CMatrix4x4& mx) { m = mx.m; }

CVector4 CMatrix4x4::Row(int32_t x) const {
  return CVector4(m[x & 3][0], m[x & 3][1], m[x & 3][2], m[x & 3][3]);
}

CVector4 CMatrix4x4::Col(int32_t x) const {
  return CVector4(m[0][x & 3], m[1][x & 3], m[2][x & 3], m[3][x & 3]);
}

CMatrix4x4& CMatrix4x4::operator*=(const CMatrix4x4& mat) {
  return *this = (*this) * mat;
}

CMatrix4x4& CMatrix4x4::operator+=(const CMatrix4x4& mat) {
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

CMatrix4x4& CMatrix4x4::operator-=(const CMatrix4x4& mat) {
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

CMatrix4x4& CMatrix4x4::operator*=(const float f) {
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

CMatrix4x4& CMatrix4x4::operator/=(const float f) {
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

CMatrix4x4 CMatrix4x4::operator+() const { return *this; }

CMatrix4x4 CMatrix4x4::operator-() const {
  return CMatrix4x4(-m[0][0], -m[0][1], -m[0][2], -m[0][3], -m[1][0], -m[1][1],
                    -m[1][2], -m[1][3], -m[2][0], -m[2][1], -m[2][2], -m[2][3],
                    -m[3][0], -m[3][1], -m[3][2], -m[3][3]);
}

CMatrix4x4 CMatrix4x4::operator*(const CMatrix4x4& mat) const {
  CMatrix4x4 matT;
  for (int32_t i = 0; i < 4; i++)
    for (int32_t j = 0; j < 4; j++)
      matT.m[i][j] = m[i][0] * mat.m[0][j] + m[i][1] * mat.m[1][j] +
                     m[i][2] * mat.m[2][j] + m[i][3] * mat.m[3][j];
  return matT;
}

CMatrix4x4 CMatrix4x4::operator+(const CMatrix4x4& mat) const {
  return CMatrix4x4(
      m[0][0] + mat.m[0][0], m[0][1] + mat.m[0][1], m[0][2] + mat.m[0][2],
      m[0][3] + mat.m[0][3], m[1][0] + mat.m[1][0], m[1][1] + mat.m[1][1],
      m[1][2] + mat.m[1][2], m[1][3] + mat.m[1][3], m[2][0] + mat.m[2][0],
      m[2][1] + mat.m[2][1], m[2][2] + mat.m[2][2], m[2][3] + mat.m[2][3],
      m[3][0] + mat.m[3][0], m[3][1] + mat.m[3][1], m[3][2] + mat.m[3][2],
      m[3][3] + mat.m[3][3]);
}

CMatrix4x4 CMatrix4x4::operator-(const CMatrix4x4& mat) const {
  return CMatrix4x4(
      m[0][0] - mat.m[0][0], m[0][1] - mat.m[0][1], m[0][2] - mat.m[0][2],
      m[0][3] - mat.m[0][3], m[1][0] - mat.m[1][0], m[1][1] - mat.m[1][1],
      m[1][2] - mat.m[1][2], m[1][3] - mat.m[1][3], m[2][0] - mat.m[2][0],
      m[2][1] - mat.m[2][1], m[2][2] - mat.m[2][2], m[2][3] - mat.m[2][3],
      m[3][0] - mat.m[3][0], m[3][1] - mat.m[3][1], m[3][2] - mat.m[3][2],
      m[3][3] - mat.m[3][3]);
}

CMatrix4x4 CMatrix4x4::operator*(const float f) const {
  return CMatrix4x4(m[0][0] * f, m[0][1] * f, m[0][2] * f, m[0][3] * f,
                    m[1][0] * f, m[1][1] * f, m[1][2] * f, m[1][3] * f,
                    m[2][0] * f, m[2][1] * f, m[2][2] * f, m[2][3] * f,
                    m[3][0] * f, m[3][1] * f, m[3][2] * f, m[3][3] * f);
}

CMatrix4x4 CMatrix4x4::operator/(const float f) const {
  float fInv = 1.0f / f;
  return CMatrix4x4(
      m[0][0] * fInv, m[0][1] * fInv, m[0][2] * fInv, m[0][3] * fInv,
      m[1][0] * fInv, m[1][1] * fInv, m[1][2] * fInv, m[1][3] * fInv,
      m[2][0] * fInv, m[2][1] * fInv, m[2][2] * fInv, m[2][3] * fInv,
      m[3][0] * fInv, m[3][1] * fInv, m[3][2] * fInv, m[3][3] * fInv);
}

CMatrix4x4 operator*(const float f, const CMatrix4x4& mat) {
  return CMatrix4x4(
      f * mat.m[0][0], f * mat.m[0][1], f * mat.m[0][2], f * mat.m[0][3],
      f * mat.m[1][0], f * mat.m[1][1], f * mat.m[1][2], f * mat.m[1][3],
      f * mat.m[2][0], f * mat.m[2][1], f * mat.m[2][2], f * mat.m[2][3],
      f * mat.m[3][0], f * mat.m[3][1], f * mat.m[3][2], f * mat.m[3][3]);
}

bool CMatrix4x4::operator==(const CMatrix4x4& mat) const { return m == mat.m; }

bool CMatrix4x4::operator!=(const CMatrix4x4& mat) const { return m != mat.m; }

CMatrix4x4 CMatrix4x4::Rotation(const CVector3& Axis, const float Angle) {
  const float s = std::cos(Angle / 2.0f);
  const CVector3 v = Axis * std::sin(Angle / 2.0f);
  const float x = v.x;
  const float y = v.y;
  const float z = v.z;

  return CMatrix4x4(                                                         //
      1 - 2 * (y * y + z * z), 2 * (x * y + z * s), 2 * (x * z - y * s), 0,  //
      2 * (x * y - z * s), 1 - 2 * (x * x + z * z), 2 * (y * z + x * s), 0,  //
      2 * (x * z + y * s), 2 * (y * z - x * s), 1 - 2 * (x * x + y * y), 0,  //
      0, 0, 0, 1);
}

}  // namespace math
