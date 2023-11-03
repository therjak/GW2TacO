#pragma once

#include <cstdint>

constexpr float clamp(float l, float v, float r) {
  if (v < l) {
    return l;
  }
  if (v > r) {
    return r;
  }
  return v;
}

class CColor {
 public:
  constexpr CColor() = default;
  constexpr CColor(const uint8_t _r, const uint8_t _g, const uint8_t _b,
                   const uint8_t _a)
      : b(_b), g(_g), r(_r), a(_a){};
  // uint32 order is ARGB
  constexpr explicit CColor(uint32_t argb)
      : b(static_cast<uint8_t>(argb >> 0)),
        g(static_cast<uint8_t>(argb >> 8)),
        r(static_cast<uint8_t>(argb >> 16)),
        a(static_cast<uint8_t>(argb >> 24)) {}

  constexpr CColor(const CColor&) = default;

  static constexpr CColor FromFloat(const float _r, const float _g,
                                    const float _b, const float _a) {
    return CColor(static_cast<uint8_t>(clamp(0, _r, 1) * 255),
                  static_cast<uint8_t>(clamp(0, _g, 1) * 255),
                  static_cast<uint8_t>(clamp(0, _b, 1) * 255),
                  static_cast<uint8_t>(clamp(0, _a, 1) * 255));
  }

  constexpr friend bool operator==(const CColor& lhs,
                                   const CColor& rhs) = default;

  [[nodiscard]] constexpr uint32_t argb() const {
    return (a << 24) | (r << 16) | (g << 8) | b;
  }

  constexpr uint8_t& R() noexcept { return r; }
  constexpr uint8_t& G() noexcept { return g; }
  constexpr uint8_t& B() noexcept { return b; }
  constexpr uint8_t& A() noexcept { return a; }
  [[nodiscard]] constexpr uint8_t R() const { return r; }
  [[nodiscard]] constexpr uint8_t G() const { return g; }
  [[nodiscard]] constexpr uint8_t B() const { return b; }
  [[nodiscard]] constexpr uint8_t A() const { return a; }

 private:
  uint8_t b = 0, g = 0, r = 0, a = 0;
};

constexpr CColor Lerp(const CColor a, const CColor b, const float t) {
  return CColor(static_cast<uint8_t>((b.R() - a.R()) * t + a.R()),
                static_cast<uint8_t>((b.G() - a.G()) * t + a.G()),
                static_cast<uint8_t>((b.B() - a.B()) * t + a.B()),
                static_cast<uint8_t>((b.A() - a.A()) * t + a.A()));
}
