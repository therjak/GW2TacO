#pragma once

#include "plane.h"
#include "vector.h"

class CSphere {
 public:
  float Radius = 0;
  CVector3 Position = {0, 0, 0};

  CSphere() = default;
  CSphere(const CVector3& p, const float r);

  const bool Intersect(const CPlane& p) const;
  const bool Contains(const CVector3& p) const;
};
