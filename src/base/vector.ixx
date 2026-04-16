module;

#include <cstdint>

export module math:vector;

export namespace math {

class Matrix4x4;

class Vector2 {
 public:
  constexpr Vector2() = default;
  constexpr Vector2(const float _x, const float _y) : x(_x), y(_y) {}
  constexpr explicit Vector2(const float* v) : x(v[0]), y(v[1]) {}

  Vector2(const Vector2&) = default;
  Vector2(Vector2&&) = default;
  Vector2& operator=(const Vector2&) = default;
  Vector2& operator=(Vector2&&) = default;
  ~Vector2() = default;

  constexpr Vector2& operator+=(const Vector2& v) {
    x += v.x;
    y += v.y;
    return *this;
  }
  constexpr Vector2& operator-=(const Vector2& v) {
    x -= v.x;
    y -= v.y;
    return *this;
  }
  constexpr Vector2& operator*=(const float f) {
    x *= f;
    y *= f;
    return *this;
  }
  constexpr Vector2& operator/=(const float f) {
    const float fi = 1 / f;
    x *= fi;
    y *= fi;
    return *this;
  }
  constexpr Vector2 operator+() const { return *this; }
  constexpr Vector2 operator-() const { return Vector2(-x, -y); }
  constexpr Vector2 operator+(const Vector2& v) const {
    return Vector2(x + v.x, y + v.y);
  }
  constexpr Vector2 operator-(const Vector2& v) const {
    return Vector2(x - v.x, y - v.y);
  }
  constexpr Vector2 operator*(const float f) const {
    return Vector2(x * f, y * f);
  }
  constexpr Vector2 operator/(const float f) const {
    const float fi = 1 / f;
    return Vector2(x * fi, y * fi);
  }
  constexpr friend bool operator==(const Vector2& lhs,
                                   const Vector2& rhs) = default;

  // dot product
  constexpr float operator*(const Vector2& v) const {
    return x * v.x + y * v.y;
  }

  Vector2 Rotated(const Vector2& center, float rotation);

  [[nodiscard]] float Length() const;
  [[nodiscard]] constexpr float LengthSquared() const { return x * x + y * y; }
  [[nodiscard]] Vector2 Normalized() const;
  void Normalize();
  static constexpr float Dot(const Vector2& v1, const Vector2& v2) {
    return v1 * v2;
  }

  float x = 0, y = 0;
};

class Vector2I {
 public:
  constexpr Vector2I() = default;
  constexpr Vector2I(const int32_t _x, const int32_t _y) : x(_x), y(_y) {}
  constexpr explicit Vector2I(const int32_t* v) : x(v[0]), y(v[1]) {}

  constexpr Vector2I(const Vector2I&) = default;
  constexpr Vector2I(Vector2I&&) = default;
  constexpr Vector2I& operator=(const Vector2I&) = default;
  constexpr Vector2I& operator=(Vector2I&&) = default;
  ~Vector2I() = default;

  constexpr Vector2I& operator+=(const Vector2I& v) {
    x += v.x;
    y += v.y;
    return *this;
  }
  constexpr Vector2I& operator-=(const Vector2I& v) {
    x -= v.x;
    y -= v.y;
    return *this;
  }
  constexpr Vector2I& operator*=(const float f) {
    x = static_cast<int32_t>(x * f);
    y = static_cast<int32_t>(y * f);
    return *this;
  }
  constexpr Vector2I& operator/=(const float f) {
    const float fi = 1 / f;
    x = static_cast<int32_t>(x * fi);
    y = static_cast<int32_t>(y * fi);
    return *this;
  }
  constexpr Vector2I& operator/=(const int32_t f) {
    x = x / f;
    y = y / f;
    return *this;
  }
  constexpr Vector2I operator+() const { return *this; }
  constexpr Vector2I operator-() const { return Vector2I(-x, -y); }
  constexpr Vector2I operator+(const Vector2I& v) const {
    return Vector2I(x + v.x, y + v.y);
  }
  constexpr Vector2I operator-(const Vector2I& v) const {
    return Vector2I(x - v.x, y - v.y);
  }
  constexpr Vector2I operator*(const float f) const {
    return Vector2I(static_cast<int32_t>(x * f), static_cast<int32_t>(y * f));
  }
  constexpr Vector2I operator/(const float f) const {
    const float fi = 1 / f;
    return Vector2I(static_cast<int32_t>(x * fi), static_cast<int32_t>(y * fi));
  }
  constexpr Vector2I operator/(const int32_t f) const {
    return Vector2I(x / f, y / f);
  }
  constexpr friend bool operator==(const Vector2I& lhs,
                                   const Vector2I& rhs) = default;

  // dot product
  constexpr int32_t operator*(const Vector2I& v) const {
    return x * v.x + y * v.y;
  }
  [[nodiscard]] float Length() const;
  [[nodiscard]] constexpr float LengthSquared() const {
    return static_cast<float>(x * x + y * y);
  }
  [[nodiscard]] Vector2I Normalized() const;
  void Normalize();
  static constexpr int32_t Dot(const Vector2I& v1, const Vector2I& v2) {
    return v1 * v2;
  }

  int32_t x = 0, y = 0;
};

using Point = Vector2I;
using Size = Vector2I;
class Vector4;

class Vector3 {
 public:
  constexpr Vector3() = default;
  constexpr Vector3(const float _x, const float _y, const float _z)
      : x(_x), y(_y), z(_z) {}
  constexpr explicit Vector3(const float* v) : x(v[0]), y(v[1]), z(v[2]) {}
  constexpr explicit Vector3(const Vector4& v);

  constexpr Vector3(const Vector3&) = default;
  constexpr Vector3(Vector3&&) = default;
  constexpr Vector3& operator=(const Vector3&) = default;
  constexpr Vector3& operator=(Vector3&&) = default;
  ~Vector3() = default;

  constexpr Vector3& operator+=(const Vector3& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }
  constexpr Vector3& operator-=(const Vector3& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
  }
  constexpr Vector3& operator*=(const float f) {
    x *= f;
    y *= f;
    z *= f;
    return *this;
  }
  constexpr Vector3& operator/=(const float f) {
    const float fi = 1 / f;
    x *= fi;
    y *= fi;
    z *= fi;
    return *this;
  }
  constexpr Vector3 operator+() const { return *this; }
  constexpr Vector3 operator-() const { return Vector3(-x, -y, -z); }
  constexpr Vector3 operator+(const Vector3& v) const {
    return Vector3(x + v.x, y + v.y, z + v.z);
  }
  constexpr Vector3 operator-(const Vector3& v) const {
    return Vector3(x - v.x, y - v.y, z - v.z);
  }
  constexpr Vector3 operator*(const float f) const {
    return Vector3(x * f, y * f, z * f);
  }
  constexpr Vector3 operator/(const float f) const {
    const float fi = 1 / f;
    return Vector3(x * fi, y * fi, z * fi);
  }
  constexpr friend bool operator==(const Vector3& lhs,
                                   const Vector3& rhs) = default;

  // dot product
  constexpr float operator*(const Vector3& v) const {
    return x * v.x + y * v.y + z * v.z;
  }
  // cross product
  constexpr Vector3 operator%(const Vector3& v) const {
    return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
  }
  Vector4 operator*(const Matrix4x4& q) const;  // transformation

  [[nodiscard]] float Length() const;
  [[nodiscard]] constexpr float LengthSquared() const {
    return x * x + y * y + z * z;
  }
  [[nodiscard]] Vector3 Normalized() const;
  void Normalize();
  static constexpr float Dot(const Vector3& v1, const Vector3& v2) {
    return v1 * v2;
  }
  static constexpr Vector3 Cross(const Vector3& v1, const Vector3& v2) {
    return v1 % v2;
  }

  float x = 0, y = 0, z = 0;
};

class Vector4 {
 public:
  constexpr Vector4() = default;
  constexpr Vector4(const float _x, const float _y, const float _z,
                    const float _w)
      : x(_x), y(_y), z(_z), w(_w) {}
  constexpr explicit Vector4(const float* v)
      : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

  constexpr Vector4(const Vector4&) = default;
  constexpr Vector4(Vector4&&) = default;
  constexpr Vector4& operator=(const Vector4&) = default;
  constexpr Vector4& operator=(Vector4&&) = default;
  ~Vector4() = default;

  constexpr Vector4& operator+=(const Vector4& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
  }
  constexpr Vector4& operator-=(const Vector4& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
  }
  constexpr Vector4& operator*=(const float f) {
    x *= f;
    y *= f;
    z *= f;
    w *= f;
    return *this;
  }
  constexpr Vector4& operator/=(const float f) {
    const float fi = 1 / f;
    x *= fi;
    y *= fi;
    z *= fi;
    w *= fi;
    return *this;
  }
  constexpr Vector4 operator+() const { return *this; }
  constexpr Vector4 operator-() const { return Vector4(-x, -y, -z, -w); }
  constexpr Vector4 operator+(const Vector4& v) const {
    return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
  }
  constexpr Vector4 operator-(const Vector4& v) const {
    return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
  }
  constexpr Vector4 operator*(const float f) const {
    return Vector4(x * f, y * f, z * f, w * f);
  }
  constexpr Vector4 operator/(const float f) const {
    const float fi = 1 / f;
    return Vector4(x * fi, y * fi, z * fi, w * fi);
  }
  constexpr friend bool operator==(const Vector4& lhs,
                                   const Vector4& rhs) = default;

  // dot product
  constexpr float operator*(const Vector4& v) const {
    return x * v.x + y * v.y + z * v.z + w * v.w;
  }
  Vector4 operator*(const Matrix4x4& q) const;  // transformation
  [[nodiscard]] float Length() const;
  [[nodiscard]] constexpr float LengthSquared() const {
    return x * x + y * y + z * z + w * w;
  }
  [[nodiscard]] Vector4 Normalized() const;
  void Normalize();
  [[nodiscard]] constexpr Vector4 Homogenized() const { return *this / w; }
  constexpr void Homogenize() { *this = Homogenized(); }
  static constexpr float Dot(const Vector4& v1, const Vector4& v2) {
    return v1 * v2;
  }
  static constexpr Vector4 Cross(const Vector4& v1, const Vector4& v2,
                                 const Vector4& v3) {
    return Vector4(v1.y * (v2.z * v3.w - v3.z * v2.w) -
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

constexpr Vector3::Vector3(const Vector4& v) : x(v.x), y(v.y), z(v.z) {}

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
constexpr Vector2 Lerp(const Vector2& v1, const Vector2& v2, const float t) {
  return (v2 - v1) * t + v1;
}
constexpr Vector2I Lerp(const Vector2I& v1, const Vector2I& v2, const float t) {
  return (v2 - v1) * t + v1;
}
constexpr Vector3 Lerp(const Vector3& v1, const Vector3& v2, const float t) {
  return (v2 - v1) * t + v1;
}
constexpr Vector4 Lerp(const Vector4& v1, const Vector4& v2, const float t) {
  return (v2 - v1) * t + v1;
}

}  // namespace math
