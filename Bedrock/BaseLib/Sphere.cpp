#include "Sphere.h"

#include <cmath>

#include "Plane.h"

const bool CSphere::Intersect(const CPlane &p) const {
  return abs(p.Distance(Position)) < Radius;
}

CSphere::CSphere(const CVector3 &p, const float r) : Position(p), Radius(r) {}

CSphere::CSphere() {}

CSphere::~CSphere() {}

const bool CSphere::Contains(const CVector3 &p) const {
  return (Position - p).Length() < Radius;
}
