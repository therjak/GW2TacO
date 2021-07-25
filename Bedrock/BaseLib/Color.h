#pragma once

#include <cstdint>

class CColor {
  uint8_t b = 0, g = 0, r = 0, a = 0;

 public:
  CColor();
  CColor(const uint8_t _r, const uint8_t _g, const uint8_t _b,
         const uint8_t _a);
  // uint32 order is ARGB
  explicit CColor(uint32_t argb);

  static CColor FromFloat(const float _r, const float _g, const float _b,
                          const float _a);

  bool operator==(const CColor& c) const;
  bool operator!=(const CColor& c) const;
  uint32_t argb() const;

  uint8_t& R();
  uint8_t& G();
  uint8_t& B();
  uint8_t& A();
  uint8_t R() const;
  uint8_t G() const;
  uint8_t B() const;
  uint8_t A() const;
};

const CColor Lerp(const CColor a, const CColor b, const float t);
