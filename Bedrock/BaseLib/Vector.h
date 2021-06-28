#pragma once

#include <cstdint>

class CQuaternion;
class CMatrix4x4;

class CVector2 {
 public:
  float x = 0, y = 0;

  CVector2();
  CVector2(const float _x, const float _y);
  CVector2(const float *v);
  CVector2(const CVector2 &v);

  float const operator[](int32_t idx) const;
  float &operator[](int32_t idx);
  operator float *();
  operator const float *() const;
  CVector2 &operator+=(const CVector2 &v);
  CVector2 &operator-=(const CVector2 &v);
  CVector2 &operator*=(const float f);
  CVector2 &operator/=(const float f);
  CVector2 operator+() const;
  CVector2 operator-() const;
  CVector2 operator+(const CVector2 &v) const;
  CVector2 operator-(const CVector2 &v) const;
  CVector2 operator*(const float f) const;
  CVector2 operator/(const float f) const;
  bool operator==(const CVector2 &v) const;
  bool operator!=(const CVector2 &v) const;
  float operator*(const CVector2 &v) const;  // dot product

  CVector2 Rotated(const CVector2 &center, float rotation);

  float Length() const;
  float LengthSquared() const;
  CVector2 Normalized() const;
  void Normalize();
  static float Dot(const CVector2 &v1, const CVector2 &v2);
};

class CVector2I {
 public:
  int32_t x = 0, y = 0;

  CVector2I();
  CVector2I(const int32_t _x, const int32_t _y);
  CVector2I(const int32_t *v);
  CVector2I(const CVector2I &v);

  int32_t const operator[](int32_t idx) const;
  int32_t &operator[](int32_t idx);
  operator int32_t *();
  operator const int32_t *() const;
  CVector2I &operator+=(const CVector2I &v);
  CVector2I &operator-=(const CVector2I &v);
  CVector2I &operator*=(const float f);
  CVector2I &operator/=(const float f);
  CVector2I &operator/=(const int32_t f);
  CVector2I operator+() const;
  CVector2I operator-() const;
  CVector2I operator+(const CVector2I &v) const;
  CVector2I operator-(const CVector2I &v) const;
  CVector2I operator*(const float f) const;
  CVector2I operator/(const float f) const;
  CVector2I operator/(const int32_t f) const;
  bool operator==(const CVector2I &v) const;
  bool operator!=(const CVector2I &v) const;
  int32_t operator*(const CVector2I &v) const;  // dot product
  float Length() const;
  float LengthSquared() const;
  CVector2I Normalized() const;
  void Normalize();
  static int32_t Dot(const CVector2I &v1, const CVector2I &v2);
};

typedef CVector2I CPoint;
typedef CVector2I CSize;
class CVector4;

class CVector3 {
 public:
  float x = 0, y = 0, z = 0;

  CVector3();
  CVector3(const float _x, const float _y, const float _z);
  CVector3(const float *v);
  CVector3(const CVector3 &v);

  float const operator[](int32_t idx) const;
  float &operator[](int32_t idx);
  operator float *();
  operator const float *() const;
  CVector3 &operator+=(const CVector3 &v);
  CVector3 &operator-=(const CVector3 &v);
  CVector3 &operator*=(const float f);
  CVector3 &operator/=(const float f);
  CVector3 operator+() const;
  CVector3 operator-() const;
  CVector3 operator+(const CVector3 &v) const;
  CVector3 operator-(const CVector3 &v) const;
  CVector3 operator*(const float f) const;
  CVector3 operator/(const float f) const;
  bool operator==(const CVector3 &v) const;
  bool operator!=(const CVector3 &v) const;
  float operator*(const CVector3 &v) const;        // dot product
  CVector3 operator%(const CVector3 &v) const;     // cross product
  CVector3 operator*(const CQuaternion &q) const;  // rotation
  CVector3 &operator*=(const CQuaternion &q);      // rotation
  CVector3 operator/(const CQuaternion &q) const;  // reverse rotation
  CVector3 &operator/=(const CQuaternion &q);      // reverse rotation
  CVector4 operator*(const CMatrix4x4 &q) const;   // transformation

  float Length() const;
  float LengthSquared() const;
  CVector3 Normalized() const;
  void Normalize();
  static float Dot(const CVector3 &v1, const CVector3 &v2);
  static CVector3 Cross(const CVector3 &v1, const CVector3 &v2);
};

class CVector4 {
 public:
  float x = 0, y = 0, z = 0, w = 0;

  CVector4();
  CVector4(const float _x, const float _y, const float _z, const float _w);
  CVector4(const float *v);
  CVector4(const CVector4 &v);

  float const operator[](int32_t idx) const;
  float &operator[](int32_t idx);
  operator float *();
  operator const float *() const;
  operator CVector3();
  operator const CVector3() const;
  CVector4 &operator+=(const CVector4 &v);
  CVector4 &operator-=(const CVector4 &v);
  CVector4 &operator*=(const float f);
  CVector4 &operator/=(const float f);
  CVector4 operator+() const;
  CVector4 operator-() const;
  CVector4 operator+(const CVector4 &v) const;
  CVector4 operator-(const CVector4 &v) const;
  CVector4 operator*(const float f) const;
  CVector4 operator/(const float f) const;
  bool operator==(const CVector4 &v) const;
  bool operator!=(const CVector4 &v) const;
  float operator*(const CVector4 &v) const;       // dot product
  CVector4 operator*(const CMatrix4x4 &q) const;  // transformation
  float Length() const;
  float LengthSquared() const;
  CVector4 Normalized() const;
  void Normalize();
  CVector4 Homogenized() const;
  void Homogenize();
  static float Dot(const CVector4 &v1, const CVector4 &v2);
  static CVector4 Cross(const CVector4 &v1, const CVector4 &v2,
                        const CVector4 &v3);
};

const int32_t Lerp(const int32_t v1, const int32_t v2, const float t);
const float Lerp(const float v1, const float v2, const float t);
const uint32_t Lerp(const uint32_t v1, const uint32_t v2, const float t);
const int64_t Lerp(const int64_t v1, const int64_t v2, const float t);
const double Lerp(const double v1, const double v2, const float t);
const uint64_t Lerp(const uint64_t v1, const uint64_t v2, const float t);
const CVector2 Lerp(const CVector2 &v1, const CVector2 &v2, const float t);
const CVector2I Lerp(const CVector2I &v1, const CVector2I &v2, const float t);
const CVector3 Lerp(const CVector3 &v1, const CVector3 &v2, const float t);
const CVector4 Lerp(const CVector4 &v1, const CVector4 &v2, const float t);
float mod(float v, float m);
int32_t Mod(int32_t a, int32_t b);
float Mod(float a, float b);
float Mod(float a, int32_t b);
