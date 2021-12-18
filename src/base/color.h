#pragma once

#include <cstdint>

class CColor {
  uint8_t b = 0, g = 0, r = 0, a = 0;

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

  static CColor FromFloat(const float _r, const float _g, const float _b,
                          const float _a);

  constexpr bool operator==(const CColor& c) const {
    return r == c.r && g == c.g && b == c.b && a == c.a;
  }
  constexpr bool operator!=(const CColor& c) const {
    return r != c.r || g != c.g || b != c.b || a != c.a;
  }
  uint32_t argb() const;

  uint8_t& R() noexcept { return r; }
  uint8_t& G() noexcept { return g; }
  uint8_t& B() noexcept { return b; }
  uint8_t& A() noexcept { return a; }
  constexpr uint8_t R() const { return r; }
  constexpr uint8_t G() const { return g; }
  constexpr uint8_t B() const { return b; }
  constexpr uint8_t A() const { return a; }
};

const CColor Lerp(const CColor a, const CColor b, const float t);
