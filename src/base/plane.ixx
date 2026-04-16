module;

#include <cstdint>

export module math:plane;

import :vector;

export namespace math {

class Plane {
 public:
  constexpr Plane() = default;
  Plane(const Vector3& Point, const Vector3& n) {
    Normal = n;
    D = -(Normal * Point);
    Normalize();
  }
  Plane(const Vector3& a, const Vector3& b, const Vector3& c) {
    Normal = ((b - a) % (c - a)).Normalized();
    D = -(Normal * a);
    Normalize();
  }

  [[nodiscard]] constexpr Vector3 Project(const Vector3& v) const {
    return v - Normal * (Normal * v + D);  // optimized for a normalized plane
  }
  [[nodiscard]] constexpr float Distance(const Vector3& v) const {
    return Normal * v + D;  // optimized for a normalized plane
  }
  [[nodiscard]] constexpr int32_t Side(const Vector3& v) const {
    const float f = Distance(v);
    if (f > 0) return 1;
    if (f < 0) return -1;
    return 0;
  }

  Vector3 Normal = {0, 0, 0};
  float D = 0;

 private:
  void Normalize() {
    const float l = Normal.Length();
    Normal /= l;
    D /= l;
  }
};

}  // namespace math
