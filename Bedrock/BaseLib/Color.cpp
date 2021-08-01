#include "Color.h"

#include <cstdint>

const CColor Lerp(const CColor a, const CColor b, const float t) {
  return CColor(static_cast<uint8_t>((b.R() - a.R()) * t + a.R()),
                static_cast<uint8_t>((b.G() - a.G()) * t + a.G()),
                static_cast<uint8_t>((b.B() - a.B()) * t + a.B()),
                static_cast<uint8_t>((b.A() - a.A()) * t + a.A()));
}

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
