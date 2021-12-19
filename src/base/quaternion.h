#pragma once

#include <cstdint>

#include "src/base/vector.h"

class CQuaternion {
  void FromEuler(const float _x, const float _y, const float _z);

 public:
  float x = 0, y = 0, z = 0, s = 0;

  constexpr CQuaternion() = default;
  constexpr CQuaternion(const float _x, const float _y, const float _z,
                        const float _s);
  constexpr CQuaternion(const float _s, const CVector3& v);
  constexpr explicit CQuaternion(const float* v);
  // from euler angles
  CQuaternion(const float _x, const float _y, const float _z);

  static CQuaternion FromAxisAngle(const CVector3& Axis, const float Angle);
};

constexpr CQuaternion::CQuaternion(const float* v)
    : x(v[0]), y(v[1]), z(v[2]), s(v[3]) {}

constexpr CQuaternion::CQuaternion(const float _s, const CVector3& v)
    : x(v.x), y(v.y), z(v.z), s(_s) {}

constexpr CQuaternion::CQuaternion(const float _x, const float _y,
                                   const float _z, const float _s)
    : x(_x), y(_y), z(_z), s(_s) {}
