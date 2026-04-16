module;

#include <cmath>

export module math:sphere;

import :plane;
import :vector;

export namespace math {

class CSphere {
 public:
  constexpr CSphere() = default;
  constexpr CSphere(const Vector3& p, const float r)
      : Radius(r), Position(p) {}

  [[nodiscard]] bool Intersect(const Plane& p) const {
    return std::abs(p.Distance(Position)) < Radius;
  }
  [[nodiscard]] bool Contains(const Vector3& p) const {
    return (Position - p).Length() < Radius;
  }

  float Radius = 0;
  Vector3 Position = {0, 0, 0};
};

}  // namespace math
