#pragma once

#include <cmath>

#include "src/base/plane.h"
#include "src/base/vector.h"

namespace math {

class CSphere {
 public:
  constexpr CSphere() = default;
  constexpr CSphere(const CVector3& p, const float r)
      : Radius(r), Position(p) {}

  [[nodiscard]] bool Intersect(const CPlane& p) const {
    return std::abs(p.Distance(Position)) < Radius;
  }
  [[nodiscard]] bool Contains(const CVector3& p) const {
    return (Position - p).Length() < Radius;
  }

  float Radius = 0;
  CVector3 Position = {0, 0, 0};
};

}  // namespace math
