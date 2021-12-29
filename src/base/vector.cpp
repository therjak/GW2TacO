#include "src/base/vector.h"

#include <cmath>
#include <cstdint>

#include "src/base/matrix.h"
#include "src/base/spec_math.h"

CVector4 CVector3::operator*(const CMatrix4x4& q) const {
  return q.Apply(*this);
}

CVector4 CVector4::operator*(const CMatrix4x4& q) const {
  return q.Apply(*this);
}

void CVector4::Normalize() { *this = Normalized(); }

CVector4 CVector4::Normalized() const {
  return *this * InvSqrt(LengthSquared());
}

float CVector4::Length() const { return sqrtf(LengthSquared()); }

void CVector3::Normalize() { *this = Normalized(); }

CVector3 CVector3::Normalized() const {
  return *this * InvSqrt(LengthSquared());
}

float CVector3::Length() const { return sqrtf(LengthSquared()); }

void CVector2::Normalize() { *this = Normalized(); }

CVector2 CVector2::Normalized() const {
  return *this * InvSqrt(LengthSquared());
}

float CVector2::Length() const { return sqrtf(LengthSquared()); }

CVector2 CVector2::Rotated(const CVector2& center, float rotation) {
  CVector2 n = (*this) - center;
  return CVector2(n.x * cosf(rotation) - n.y * sinf(rotation),
                  n.y * cosf(rotation) + n.x * sinf(rotation)) +
         center;
}

void CVector2I::Normalize() { *this = Normalized(); }

CVector2I CVector2I::Normalized() const {
  return *this * InvSqrt(LengthSquared());
}

float CVector2I::Length() const { return sqrtf(LengthSquared()); }
