module;

#include <cmath>
#include <cstdint>

module math;

import :matrix;
import :spec_math;
import :vector;

namespace math {

Vector4 Vector3::operator*(const Matrix4x4& q) const { return q.Apply(*this); }

Vector4 Vector4::operator*(const Matrix4x4& q) const { return q.Apply(*this); }

void Vector4::Normalize() { *this = Normalized(); }

Vector4 Vector4::Normalized() const { return *this * InvSqrt(LengthSquared()); }

float Vector4::Length() const { return sqrtf(LengthSquared()); }

void Vector3::Normalize() { *this = Normalized(); }

Vector3 Vector3::Normalized() const { return *this * InvSqrt(LengthSquared()); }

float Vector3::Length() const { return sqrtf(LengthSquared()); }

void Vector2::Normalize() { *this = Normalized(); }

Vector2 Vector2::Normalized() const { return *this * InvSqrt(LengthSquared()); }

float Vector2::Length() const { return sqrtf(LengthSquared()); }

Vector2 Vector2::Rotated(const Vector2& center, float rotation) {
  Vector2 n = (*this) - center;
  return Vector2(n.x * cosf(rotation) - n.y * sinf(rotation),
                 n.y * cosf(rotation) + n.x * sinf(rotation)) +
         center;
}

void Vector2I::Normalize() { *this = Normalized(); }

Vector2I Vector2I::Normalized() const {
  return *this * InvSqrt(LengthSquared());
}

float Vector2I::Length() const { return sqrtf(LengthSquared()); }

}  // namespace math
