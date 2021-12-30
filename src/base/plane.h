#pragma once

#include <cstdint>

#include "src/base/vector.h"

namespace math {

class CPlane {
  void Normalize() {
    const float l = Normal.Length();
    Normal /= l;
    D /= l;
  }

 public:
  CVector3 Normal = {0, 0, 0};
  float D = 0;

  constexpr CPlane() = default;
  CPlane(const CVector3& Point, const CVector3& n) {
    Normal = n;
    D = -(Normal * Point);
    Normalize();
  }
  CPlane(const CVector3& a, const CVector3& b, const CVector3& c) {
    Normal = ((b - a) % (c - a)).Normalized();
    D = -(Normal * a);
    Normalize();
  }

  constexpr CVector3 Project(const CVector3& v) const {
    return v - Normal * (Normal * v + D);  // optimized for a normalized plane
  }
  constexpr float Distance(const CVector3& v) const {
    return Normal * v + D;  // optimized for a normalized plane
  }
  constexpr int32_t Side(const CVector3& v) const {
    const float f = Distance(v);
    if (f > 0) return 1;
    if (f < 0) return -1;
    return 0;
  }
};

}  // namespace math
