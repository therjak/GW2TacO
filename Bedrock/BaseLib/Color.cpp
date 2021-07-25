#include "Color.h"

#include <cstdint>

const CColor Lerp(const CColor a, const CColor b, const float t) {
  return CColor(static_cast<uint8_t>((b.R() - a.R()) * t + a.R()),
                static_cast<uint8_t>((b.G() - a.G()) * t + a.G()),
                static_cast<uint8_t>((b.B() - a.B()) * t + a.B()),
                static_cast<uint8_t>((b.A() - a.A()) * t + a.A()));
}

uint8_t& CColor::A() { return a; }

uint8_t& CColor::B() { return b; }

uint8_t& CColor::G() { return g; }

uint8_t& CColor::R() { return r; }

uint8_t CColor::A() const { return a; }

uint8_t CColor::B() const { return b; }

uint8_t CColor::G() const { return g; }

uint8_t CColor::R() const { return r; }

uint32_t CColor::argb() const { return (a << 24) | (r << 16) | (g << 8) | b; }

float clamp(float l, float v, float r) {
  if (v < l) {
    return l;
  }
  if (v > r) {
    return r;
  }
  return v;
}

CColor CColor::FromFloat(const float _r, const float _g, const float _b,
                         const float _a) {
  return CColor(static_cast<uint8_t>(clamp(0, _r, 1) * 255),
                static_cast<uint8_t>(clamp(0, _g, 1) * 255),
                static_cast<uint8_t>(clamp(0, _b, 1) * 255),
                static_cast<uint8_t>(clamp(0, _a, 1) * 255));
}

CColor::CColor(const uint32_t argb)
    : r(static_cast<uint8_t>(argb >> 16)),
      g(static_cast<uint8_t>(argb >> 8)),
      b(static_cast<uint8_t>(argb >> 0)),
      a(static_cast<uint8_t>(argb >> 24)) {}

CColor::CColor(const uint8_t _r, const uint8_t _g, const uint8_t _b,
               const uint8_t _a)
    : r(_r), g(_g), b(_b), a(_a) {}

CColor::CColor() = default;

bool CColor::operator==(const CColor& c) const {
  return r == c.r && g == c.g && b == c.b && a == c.a;
}

bool CColor::operator!=(const CColor& c) const {
  return r != c.r || g != c.g || b != c.b || a != c.a;
}
