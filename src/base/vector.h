#pragma once

#include <cstdint>

namespace math {

class CMatrix4x4;

class CVector2 {
 public:
  constexpr CVector2() = default;
  constexpr CVector2(const float _x, const float _y) : x(_x), y(_y) {}
  constexpr explicit CVector2(const float* v) : x(v[0]), y(v[1]) {}

  CVector2(const CVector2&) = default;
  CVector2(CVector2&&) = default;
  CVector2& operator=(const CVector2&) = default;
  CVector2& operator=(CVector2&&) = default;
  ~CVector2() = default;

  constexpr CVector2& operator+=(const CVector2& v) {
    x += v.x;
    y += v.y;
    return *this;
  }
  constexpr CVector2& operator-=(const CVector2& v) {
    x -= v.x;
    y -= v.y;
    return *this;
  }
  constexpr CVector2& operator*=(const float f) {
    x *= f;
    y *= f;
    return *this;
  }
  constexpr CVector2& operator/=(const float f) {
    const float fi = 1 / f;
    x *= fi;
    y *= fi;
    return *this;
  }
  constexpr CVector2 operator+() const { return *this; }
  constexpr CVector2 operator-() const { return CVector2(-x, -y); }
  constexpr CVector2 operator+(const CVector2& v) const {
    return CVector2(x + v.x, y + v.y);
  }
  constexpr CVector2 operator-(const CVector2& v) const {
    return CVector2(x - v.x, y - v.y);
  }
  constexpr CVector2 operator*(const float f) const {
    return CVector2(x * f, y * f);
  }
  constexpr CVector2 operator/(const float f) const {
    const float fi = 1 / f;
    return CVector2(x * fi, y * fi);
  }
  constexpr bool operator==(const CVector2& v) const {
    return x == v.x && y == v.y;
  }
  constexpr bool operator!=(const CVector2& v) const {
    return x != v.x || y != v.y;
  }
  // dot product
  constexpr float operator*(const CVector2& v) const {
    return x * v.x + y * v.y;
  }

  CVector2 Rotated(const CVector2& center, float rotation);

  [[nodiscard]] float Length() const;
  [[nodiscard]] constexpr float LengthSquared() const { return x * x + y * y; }
  [[nodiscard]] CVector2 Normalized() const;
  void Normalize();
  static constexpr float Dot(const CVector2& v1, const CVector2& v2) {
    return v1 * v2;
  }

  float x = 0, y = 0;
};

class CVector2I {
 public:
  constexpr CVector2I() = default;
  constexpr CVector2I(const int32_t _x, const int32_t _y) : x(_x), y(_y) {}
  constexpr explicit CVector2I(const int32_t* v) : x(v[0]), y(v[1]) {}

  constexpr CVector2I(const CVector2I&) = default;
  constexpr CVector2I(CVector2I&&) = default;
  constexpr CVector2I& operator=(const CVector2I&) = default;
  constexpr CVector2I& operator=(CVector2I&&) = default;
  ~CVector2I() = default;

  constexpr CVector2I& operator+=(const CVector2I& v) {
    x += v.x;
    y += v.y;
    return *this;
  }
  constexpr CVector2I& operator-=(const CVector2I& v) {
    x -= v.x;
    y -= v.y;
    return *this;
  }
  constexpr CVector2I& operator*=(const float f) {
    x = static_cast<int32_t>(x * f);
    y = static_cast<int32_t>(y * f);
    return *this;
  }
  constexpr CVector2I& operator/=(const float f) {
    const float fi = 1 / f;
    x = static_cast<int32_t>(x * fi);
    y = static_cast<int32_t>(y * fi);
    return *this;
  }
  constexpr CVector2I& operator/=(const int32_t f) {
    x = x / f;
    y = y / f;
    return *this;
  }
  constexpr CVector2I operator+() const { return *this; }
  constexpr CVector2I operator-() const { return CVector2I(-x, -y); }
  constexpr CVector2I operator+(const CVector2I& v) const {
    return CVector2I(x + v.x, y + v.y);
  }
  constexpr CVector2I operator-(const CVector2I& v) const {
    return CVector2I(x - v.x, y - v.y);
  }
  constexpr CVector2I operator*(const float f) const {
    return CVector2I(static_cast<int32_t>(x * f), static_cast<int32_t>(y * f));
  }
  constexpr CVector2I operator/(const float f) const {
    const float fi = 1 / f;
    return CVector2I(static_cast<int32_t>(x * fi),
                     static_cast<int32_t>(y * fi));
  }
  constexpr CVector2I operator/(const int32_t f) const {
    return CVector2I(x / f, y / f);
  }
  constexpr bool operator==(const CVector2I& v) const {
    return x == v.x && y == v.y;
  }
  constexpr bool operator!=(const CVector2I& v) const {
    return x != v.x || y != v.y;
  }
  // dot product
  constexpr int32_t operator*(const CVector2I& v) const {
    return x * v.x + y * v.y;
  }
  [[nodiscard]] float Length() const;
  [[nodiscard]] constexpr float LengthSquared() const {
    return static_cast<float>(x * x + y * y);
  }
  [[nodiscard]] CVector2I Normalized() const;
  void Normalize();
  static constexpr int32_t Dot(const CVector2I& v1, const CVector2I& v2) {
    return v1 * v2;
  }

  int32_t x = 0, y = 0;
};

using CPoint = CVector2I;
using CSize = CVector2I;
class CVector4;

class CVector3 {
 public:
  constexpr CVector3() = default;
  constexpr CVector3(const float _x, const float _y, const float _z)
      : x(_x), y(_y), z(_z) {}
  constexpr explicit CVector3(const float* v) : x(v[0]), y(v[1]), z(v[2]) {}
  constexpr explicit CVector3(const CVector4& v);

  constexpr CVector3(const CVector3&) = default;
  constexpr CVector3(CVector3&&) = default;
  constexpr CVector3& operator=(const CVector3&) = default;
  constexpr CVector3& operator=(CVector3&&) = default;
  ~CVector3() = default;

  constexpr CVector3& operator+=(const CVector3& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }
  constexpr CVector3& operator-=(const CVector3& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
  }
  constexpr CVector3& operator*=(const float f) {
    x *= f;
    y *= f;
    z *= f;
    return *this;
  }
  constexpr CVector3& operator/=(const float f) {
    const float fi = 1 / f;
    x *= fi;
    y *= fi;
    z *= fi;
    return *this;
  }
  constexpr CVector3 operator+() const { return *this; }
  constexpr CVector3 operator-() const { return CVector3(-x, -y, -z); }
  constexpr CVector3 operator+(const CVector3& v) const {
    return CVector3(x + v.x, y + v.y, z + v.z);
  }
  constexpr CVector3 operator-(const CVector3& v) const {
    return CVector3(x - v.x, y - v.y, z - v.z);
  }
  constexpr CVector3 operator*(const float f) const {
    return CVector3(x * f, y * f, z * f);
  }
  constexpr CVector3 operator/(const float f) const {
    const float fi = 1 / f;
    return CVector3(x * fi, y * fi, z * fi);
  }
  constexpr bool operator==(const CVector3& v) {
    return x == v.x && y == v.y && z == v.z;
  }
  constexpr bool operator!=(const CVector3& v) const {
    return x != v.x || y != v.y || z != v.z;
  }
  // dot product
  constexpr float operator*(const CVector3& v) const {
    return x * v.x + y * v.y + z * v.z;
  }
  // cross product
  constexpr CVector3 operator%(const CVector3& v) const {
    return CVector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
  }
  CVector4 operator*(const CMatrix4x4& q) const;  // transformation

  [[nodiscard]] float Length() const;
  [[nodiscard]] constexpr float LengthSquared() const {
    return x * x + y * y + z * z;
  }
  [[nodiscard]] CVector3 Normalized() const;
  void Normalize();
  static constexpr float Dot(const CVector3& v1, const CVector3& v2) {
    return v1 * v2;
  }
  static constexpr CVector3 Cross(const CVector3& v1, const CVector3& v2) {
    return v1 % v2;
  }

  float x = 0, y = 0, z = 0;
};

class CVector4 {
 public:
  constexpr CVector4() = default;
  constexpr CVector4(const float _x, const float _y, const float _z,
                     const float _w)
      : x(_x), y(_y), z(_z), w(_w) {}
  constexpr explicit CVector4(const float* v)
      : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

  constexpr CVector4(const CVector4&) = default;
  constexpr CVector4(CVector4&&) = default;
  constexpr CVector4& operator=(const CVector4&) = default;
  constexpr CVector4& operator=(CVector4&&) = default;
  ~CVector4() = default;

  constexpr CVector4& operator+=(const CVector4& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
  }
  constexpr CVector4& operator-=(const CVector4& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
  }
  constexpr CVector4& operator*=(const float f) {
    x *= f;
    y *= f;
    z *= f;
    w *= f;
    return *this;
  }
  constexpr CVector4& operator/=(const float f) {
    const float fi = 1 / f;
    x *= fi;
    y *= fi;
    z *= fi;
    w *= fi;
    return *this;
  }
  constexpr CVector4 operator+() const { return *this; }
  constexpr CVector4 operator-() const { return CVector4(-x, -y, -z, -w); }
  constexpr CVector4 operator+(const CVector4& v) const {
    return CVector4(x + v.x, y + v.y, z + v.z, w + v.w);
  }
  constexpr CVector4 operator-(const CVector4& v) const {
    return CVector4(x - v.x, y - v.y, z - v.z, w - v.w);
  }
  constexpr CVector4 operator*(const float f) const {
    return CVector4(x * f, y * f, z * f, w * f);
  }
  constexpr CVector4 operator/(const float f) const {
    const float fi = 1 / f;
    return CVector4(x * fi, y * fi, z * fi, w * fi);
  }
  constexpr bool operator==(const CVector4& v) const {
    return x == v.x && y == v.y && z == v.z && w == v.w;
  }
  constexpr bool operator!=(const CVector4& v) const {
    return x != v.x || y != v.y || z != v.z || w != v.w;
  }
  // dot product
  constexpr float operator*(const CVector4& v) const {
    return x * v.x + y * v.y + z * v.z + w * v.w;
  }
  CVector4 operator*(const CMatrix4x4& q) const;  // transformation
  [[nodiscard]] float Length() const;
  [[nodiscard]] constexpr float LengthSquared() const {
    return x * x + y * y + z * z + w * w;
  }
  [[nodiscard]] CVector4 Normalized() const;
  void Normalize();
  [[nodiscard]] constexpr CVector4 Homogenized() const { return *this / w; }
  constexpr void Homogenize() { *this = Homogenized(); }
  static constexpr float Dot(const CVector4& v1, const CVector4& v2) {
    return v1 * v2;
  }
  static constexpr CVector4 Cross(const CVector4& v1, const CVector4& v2,
                                  const CVector4& v3) {
    return CVector4(v1.y * (v2.z * v3.w - v3.z * v2.w) -
                        v1.z * (v2.y * v3.w - v3.y * v2.w) +
                        v1.w * (v2.y * v3.z - v2.z * v3.y),
                    -(v1.x * (v2.z * v3.w - v3.z * v2.w) -
                      v1.z * (v2.x * v3.w - v3.x * v2.w) +
                      v1.w * (v2.x * v3.z - v3.x * v2.z)),
                    v1.x * (v2.y * v3.w - v3.y * v2.w) -
                        v1.y * (v2.x * v3.w - v3.x * v2.w) +
                        v1.w * (v2.x * v3.y - v3.x * v2.y),
                    -(v1.x * (v2.y * v3.z - v3.y * v2.z) -
                      v1.y * (v2.x * v3.z - v3.x * v2.z) +
                      v1.z * (v2.x * v3.y - v3.x * v2.y)));
  }

  float x = 0, y = 0, z = 0, w = 0;
};

constexpr CVector3::CVector3(const CVector4& v) : x(v.x), y(v.y), z(v.z) {}

constexpr int32_t Lerp(const int32_t v1, const int32_t v2, const float t) {
  return static_cast<int32_t>((v2 - v1) * t + v1);
}
constexpr float Lerp(const float v1, const float v2, const float t) {
  return (v2 - v1) * t + v1;
}
constexpr uint32_t Lerp(const uint32_t v1, const uint32_t v2, const float t) {
  return static_cast<uint32_t>((v2 - v1) * t + v1);
}
constexpr int64_t Lerp(const int64_t v1, const int64_t v2, const float t) {
  return static_cast<int64_t>((v2 - v1) * t + v1);
}
constexpr double Lerp(const double v1, const double v2, const float t) {
  return (v2 - v1) * t + v1;
}
constexpr uint64_t Lerp(const uint64_t v1, const uint64_t v2, const float t) {
  return static_cast<uint64_t>((v2 - v1) * t + v1);
}
constexpr CVector2 Lerp(const CVector2& v1, const CVector2& v2, const float t) {
  return (v2 - v1) * t + v1;
}
constexpr CVector2I Lerp(const CVector2I& v1, const CVector2I& v2,
                         const float t) {
  return (v2 - v1) * t + v1;
}
constexpr CVector3 Lerp(const CVector3& v1, const CVector3& v2, const float t) {
  return (v2 - v1) * t + v1;
}
constexpr CVector4 Lerp(const CVector4& v1, const CVector4& v2, const float t) {
  return (v2 - v1) * t + v1;
}

}  // namespace math
