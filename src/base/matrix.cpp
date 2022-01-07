#include "src/base/matrix.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "src/base/assert.h"
#include "src/base/vector.h"

namespace math {

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
