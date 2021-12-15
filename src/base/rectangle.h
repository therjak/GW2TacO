#pragma once

#include <cstdint>

#include "src/base/vector.h"

class CRect {
 public:
  int32_t x1 = 0, y1 = 0, x2 = 0, y2 = 0;

  CRect() = default;
  CRect(const int32_t a, const int32_t b, const int32_t c, const int32_t d);
  CRect(const CPoint p1, const CPoint p2);
  const bool Contains(const int32_t x, const int32_t y) const;
  const bool Contains(CPoint& p) const;
  const bool Contains(const CPoint& p) const;
  const int32_t Width() const;
  const int32_t Height() const;
  const int32_t Area() const;
  const bool Intersects(const CRect& r) const;

  CRect operator+(const CRect& a) const;
  CRect operator-(const CRect& a) const;
  CRect operator*(const int32_t a) const;
  CRect& operator+=(const CRect& a);  // inflate by rect
  CRect operator+(const CPoint& p) const;
  CRect& operator+=(const CPoint& p);
  CRect operator-(const CPoint& p) const;
  CRect& operator-=(const CPoint& p);
  const bool operator==(const CRect& r) const;
  const bool operator!=(const CRect& r) const;
  CRect operator|(const CRect& r) const;
  CRect& operator|=(const CRect& r);
  CRect operator&(const CRect& r) const;
  CRect& operator&=(const CRect& r);

  void Move(int32_t x, int32_t y);
  void Move(const CPoint& p);
  void MoveTo(int32_t x, int32_t y);
  void SetSize(int32_t x, int32_t y);
  void Normalize();
  CPoint TopLeft() const;
  CPoint BottomRight() const;
  CPoint TopRight() const;
  CPoint BottomLeft() const;
  CSize Size() const;
  CPoint Center() const;
  CRect GetCenterRect(CSize& s);
  CRect GetIntersection(const CRect& r) const;
};

bool IntervalIntersection(int32_t a1, int32_t a2, int32_t b1, int32_t b2);
