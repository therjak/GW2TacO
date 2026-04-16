module;

#include <algorithm>
#include <cstdint>

export module math:rectangle;

import :vector;

export namespace math {

class Rect {
 public:
  constexpr Rect() = default;
  constexpr Rect(const int32_t a, const int32_t b, const int32_t c,
                 const int32_t d)
      : x1(a), y1(b), x2(c), y2(d) {}
  constexpr Rect(const Point p1, const Point p2)
      : x1(p1.x), y1(p1.y), x2(p2.x), y2(p2.y) {}
  [[nodiscard]] constexpr bool Contains(const int32_t x,
                                        const int32_t y) const {
    return x >= x1 && x < x2 && y >= y1 && y < y2;
  }
  [[nodiscard]] constexpr bool Contains(const Point& p) const {
    return Contains(p.x, p.y);
  }
  [[nodiscard]] constexpr int32_t Width() const { return x2 - x1; }
  [[nodiscard]] constexpr int32_t Height() const { return y2 - y1; }
  [[nodiscard]] constexpr int32_t Area() const { return Width() * Height(); }
  [[nodiscard]] constexpr bool Intersects(const Rect& r) const {
    return !(x2 <= r.x1 || x1 >= r.x2) && !(y2 <= r.y1 || y1 >= r.y2);
  }

  constexpr Rect operator+(const Rect& a) const {
    return Rect(x1 - a.x1, y1 - a.y1, x2 + a.x2, y2 + a.y2);
  }
  constexpr Rect operator-(const Rect& a) const {
    return Rect(x1 + a.x1, y1 + a.y1, x2 - a.x2, y2 - a.y2);
  }
  constexpr Rect operator*(const int32_t a) const {
    return Rect(x1 * a, y1 * a, x2 * a, y2 * a);
  }
  // inflate by rect
  constexpr Rect& operator+=(const Rect& a) {
    x1 -= a.x1;
    y1 -= a.y1;
    x2 += a.x2;
    y2 += a.y2;
    return *this;
  }

  constexpr Rect operator+(const Point& p) const {
    return Rect(x1 + p.x, y1 + p.y, x2 + p.x, y2 + p.y);
  }
  constexpr Rect& operator+=(const Point& p) {
    Move(p);
    return *this;
  }
  constexpr Rect operator-(const Point& p) const {
    return Rect(x1 - p.x, y1 - p.y, x2 - p.x, y2 - p.y);
  }
  constexpr Rect& operator-=(const Point& p) {
    Move(-p);
    return *this;
  }
  constexpr friend bool operator==(const Rect& lhs, const Rect& rhs) = default;

  constexpr Rect operator|(const Rect& r) const {
    if (!Intersects(r)) return Rect(1, 1, -1, -1);
    return GetIntersection(r);
  }
  constexpr Rect& operator|=(const Rect& r) {
    *this = *this | r;
    return *this;
  }
  constexpr Rect operator&(const Rect& r) const {
    return Rect(x1 < r.x1 ? x1 : r.x1, y1 < r.y1 ? y1 : r.y1,
                x2 > r.x2 ? x2 : r.x2, y2 > r.y2 ? y2 : r.y2);
  }
  constexpr Rect& operator&=(const Rect& r) {
    *this = *this & r;
    return *this;
  }

  constexpr void Move(int32_t x, int32_t y) {
    x1 += x;
    y1 += y;
    x2 += x;
    y2 += y;
  }
  constexpr void Move(const Point& p) { Move(p.x, p.y); }
  constexpr void MoveTo(int32_t x, int32_t y) {
    x2 -= (x1 - x);
    y2 -= (y1 - y);
    x1 = x;
    y1 = y;
  }
  constexpr void SetSize(int32_t x, int32_t y) {
    x2 = x1 + x;
    y2 = y1 + y;
  }
  constexpr void Normalize() {
    const int32_t _x1 = x1;
    const int32_t _y1 = y1;
    const int32_t _x2 = x2;
    const int32_t _y2 = y2;

    x1 = _x1 < _x2 ? _x1 : _x2;
    x2 = _x1 < _x2 ? _x2 : _x1;
    y1 = _y1 < _y2 ? _y1 : _y2;
    y2 = _y1 < _y2 ? _y2 : _y1;
  }
  [[nodiscard]] constexpr Point TopLeft() const { return Point(x1, y1); }
  [[nodiscard]] constexpr Point BottomRight() const { return Point(x2, y2); }
  [[nodiscard]] constexpr Point TopRight() const { return Point(x2, y1); }
  [[nodiscard]] constexpr Point BottomLeft() const { return Point(x1, y2); }
  [[nodiscard]] constexpr math::Size Size() const {
    return math::Size(Width(), Height());
  }
  [[nodiscard]] constexpr Point Center() const {
    return Point(x1 + x2, y1 + y2) / 2;
  }
  constexpr Rect GetCenterRect(const math::Size& s) {
    return Rect(Center().x - s.x / 2, Center().y - s.y / 2,
                Center().x - s.x / 2 + s.x, Center().y - s.y / 2 + s.y);
  }
  [[nodiscard]] constexpr Rect GetIntersection(const Rect& r) const {
    return Rect(x1 > r.x1 ? x1 : r.x1, y1 > r.y1 ? y1 : r.y1,
                x2 < r.x2 ? x2 : r.x2, y2 < r.y2 ? y2 : r.y2);
  }

  int32_t x1 = 0, y1 = 0, x2 = 0, y2 = 0;
};

constexpr bool IntervalIntersection(int32_t a1, int32_t a2, int32_t b1,
                                    int32_t b2) {
  /*
  const int32_t _a1 = std::min(a1, a2);
  const int32_t _a2 = std::max(a1, a2);
  const int32_t _b1 = std::min(b1, b2);
  const int32_t _b2 = std::max(b1, b2);
  */
  return !((a1 < b1 && a2 < b1) || (a1 > b2 && a2 > b2));
}

}  // namespace math
