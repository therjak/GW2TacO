#include "src/base/quaternion.h"

#include <cmath>
#include <cstdint>

CQuaternion::CQuaternion(const float _x, const float _y,
                         const float _z)  // from euler angles
{
  FromEuler(_x, _y, _z);
}

CQuaternion CQuaternion::FromAxisAngle(const CVector3& Axis,
                                       const float Angle) {
  return CQuaternion(std::cos(Angle / 2.0f), Axis * std::sin(Angle / 2.0f));
}

void CQuaternion::FromEuler(const float _x, const float _y, const float _z) {
  const float cos_z = std::cos(0.5f * _z);
  const float cos_y = std::cos(0.5f * _y);
  const float cos_x = std::cos(0.5f * _x);

  const float sin_z = std::sin(0.5f * _z);
  const float sin_y = std::sin(0.5f * _y);
  const float sin_x = std::sin(0.5f * _x);

  s = cos_z * cos_y * cos_x + sin_z * sin_y * sin_x;
  x = cos_z * cos_y * sin_x - sin_z * sin_y * cos_x;
  y = cos_z * sin_y * cos_x + sin_z * cos_y * sin_x;
  z = sin_z * cos_y * cos_x - cos_z * sin_y * sin_x;
}
