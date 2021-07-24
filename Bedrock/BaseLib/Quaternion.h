#pragma once

#include <cstdint>

#include "Matrix.h"
#include "Vector.h"

class CQuaternion {
 public:
  float x = 0, y = 0, z = 0, s = 0;

  CQuaternion() = default;
  CQuaternion(const float _x, const float _y, const float _z, const float _s);
  CQuaternion(const float _s, const CVector3& v);
  explicit CQuaternion(const float* v);
  CQuaternion(const float _x, const float _y,
              const float _z);              // from euler angles
  explicit CQuaternion(const CVector3& v);  // from euler angles

  void FromEuler(const float _x, const float _y, const float _z);
  CVector3 ToEuler() const;
  static CQuaternion FromAxisAngle(const CVector3& Axis, const float Angle);
  void ToAxisAngle(CVector3& Axis, float& Angle) const;
  void FromRotationMatrix(const CMatrix4x4& m);

  float const operator[](int32_t idx) const;
  float& operator[](int32_t idx);
  operator float*();
  operator const float*() const;
  CQuaternion& operator=(const CQuaternion& q);
  CQuaternion& operator+=(const CQuaternion& v);
  CQuaternion& operator-=(const CQuaternion& v);
  const CQuaternion& operator*=(const CQuaternion& v);
  CQuaternion& operator*=(const float f);
  CQuaternion& operator/=(const float f);
  CQuaternion operator+() const;
  CQuaternion operator-() const;
  CQuaternion operator+(const CQuaternion& v) const;
  CQuaternion operator-(const CQuaternion& v) const;
  CQuaternion operator*(const float f) const;
  CQuaternion operator/(const float f) const;
  bool operator==(const CQuaternion& v) const;
  bool operator!=(const CQuaternion& v) const;
  CQuaternion operator*(const CQuaternion& q) const;
  float Length() const;
  float LengthSquared() const;
  CQuaternion Normalized() const;
  void Conjugate();
  void Invert();
  CQuaternion Inverted() const;
  void Normalize();
  CQuaternion Log() const;
  CQuaternion Exp() const;
  static float Dot(const CQuaternion& v1, const CQuaternion& v2);
};

CQuaternion Lerp(const CQuaternion& v1, const CQuaternion& v2, const float t);
CQuaternion Slerp(const CQuaternion& v1, const CQuaternion& v2, const float t);
CQuaternion SlerpFast(const CQuaternion& v1, const CQuaternion& v2,
                      const float t);
CQuaternion Squad(const CQuaternion& q1, const CQuaternion& S1,
                  const CQuaternion& S2, const CQuaternion& q2, const float t);
void SquadSetup(CQuaternion& OutA, CQuaternion& OutB, CQuaternion& OutC,
                const CQuaternion& Q0, const CQuaternion& Q1,
                const CQuaternion& Q2, const CQuaternion& Q3);
