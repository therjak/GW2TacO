#include <gtest/gtest.h>

#include <cmath>
import math;

// Helper to compare Vector elements
#define EXPECT_VEC2_EQ(v, ex, ey) \
  EXPECT_FLOAT_EQ((v).x, (ex));   \
  EXPECT_FLOAT_EQ((v).y, (ey));

#define EXPECT_VEC2I_EQ(v, ex, ey) \
  EXPECT_EQ((v).x, (ex));          \
  EXPECT_EQ((v).y, (ey));

#define EXPECT_VEC3_EQ(v, ex, ey, ez) \
  EXPECT_FLOAT_EQ((v).x, (ex));       \
  EXPECT_FLOAT_EQ((v).y, (ey));       \
  EXPECT_FLOAT_EQ((v).z, (ez));

#define EXPECT_VEC4_EQ(v, ex, ey, ez, ew) \
  EXPECT_FLOAT_EQ((v).x, (ex));           \
  EXPECT_FLOAT_EQ((v).y, (ey));           \
  EXPECT_FLOAT_EQ((v).z, (ez));           \
  EXPECT_FLOAT_EQ((v).w, (ew));

TEST(MathTest, Vector2Ops) {
  math::Vector2 v1(1.0f, 2.0f);
  math::Vector2 v2(3.0f, 4.0f);

  EXPECT_VEC2_EQ(v1 + v2, 4.0f, 6.0f);
  EXPECT_VEC2_EQ(v1 - v2, -2.0f, -2.0f);
  EXPECT_VEC2_EQ(v1 * 2.0f, 2.0f, 4.0f);
  EXPECT_VEC2_EQ(v1 / 2.0f, 0.5f, 1.0f);

  EXPECT_FLOAT_EQ(v1 * v2, 11.0f);  // Dot product
  EXPECT_FLOAT_EQ(v1.LengthSquared(), 5.0f);
  EXPECT_NEAR(v1.Length(), sqrtf(5.0f), 0.0001f);

  math::Vector2 v3 = v1.Normalized();
  EXPECT_NEAR(v3.Length(), 1.0f, 0.0001f);
}

TEST(MathTest, Vector2IOps) {
  math::Vector2I v1(1, 2);
  math::Vector2I v2(3, 4);

  EXPECT_VEC2I_EQ(v1 + v2, 4, 6);
  EXPECT_VEC2I_EQ(v1 - v2, -2, -2);
  EXPECT_VEC2I_EQ(v1 * 2.0f, 2, 4);
  EXPECT_VEC2I_EQ(v1 / 2.0f, 0, 1);  // Integer division via float

  EXPECT_EQ(v1 * v2, 11);
  EXPECT_EQ(v1.LengthSquared(), 5.0f);
}

TEST(MathTest, Vector3Ops) {
  math::Vector3 v1(1.0f, 0.0f, 0.0f);
  math::Vector3 v2(0.0f, 1.0f, 0.0f);

  math::Vector3 cross = v1 % v2;
  EXPECT_VEC3_EQ(cross, 0.0f, 0.0f, 1.0f);

  EXPECT_FLOAT_EQ(v1 * v2, 0.0f);
  EXPECT_FLOAT_EQ(v1.Length(), 1.0f);

  math::Vector3 v3(1.0f, 2.0f, 3.0f);
  math::Vector3 v4(4.0f, 5.0f, 6.0f);
  EXPECT_VEC3_EQ(v3 + v4, 5.0f, 7.0f, 9.0f);
}

TEST(MathTest, Vector4Ops) {
  math::Vector4 v1(1.0f, 2.0f, 3.0f, 4.0f);
  EXPECT_VEC4_EQ(v1.Homogenized(), 0.25f, 0.5f, 0.75f, 1.0f);

  math::Vector4 v2(1.0f, 0.0f, 0.0f, 0.0f);
  math::Vector4 v3(0.0f, 1.0f, 0.0f, 0.0f);
  math::Vector4 v4(0.0f, 0.0f, 1.0f, 0.0f);
  math::Vector4 res = math::Vector4::Cross(v2, v3, v4);
  // 4D cross product is complex, but result should be orthogonal to inputs
  EXPECT_FLOAT_EQ(res * v2, 0.0f);
  EXPECT_FLOAT_EQ(res * v3, 0.0f);
  EXPECT_FLOAT_EQ(res * v4, 0.0f);
}

TEST(MathTest, MatrixOps) {
  math::Matrix4x4 m1;
  m1.SetIdentity();
  math::Vector3 v(1.0f, 2.0f, 3.0f);
  math::Vector4 r = m1.Apply(v);
  EXPECT_VEC4_EQ(r, 1.0f, 2.0f, 3.0f, 1.0f);

  math::Matrix4x4 trans = math::Matrix4x4::Translation({10.0f, 20.0f, 30.0f});
  r = trans.Apply(v);
  EXPECT_VEC4_EQ(r, 11.0f, 22.0f, 33.0f, 1.0f);

  math::Matrix4x4 scale = math::Matrix4x4::Scaling({2.0f, 2.0f, 2.0f});
  r = scale.Apply(v);
  EXPECT_VEC4_EQ(r, 2.0f, 4.0f, 6.0f, 1.0f);

  math::Matrix4x4 combined = trans * scale;
  r = combined.Apply(v);
  // trans * scale applies trans first, then scale (v * trans * scale)
  EXPECT_VEC4_EQ(r, 22.0f, 44.0f, 66.0f, 1.0f);
}

TEST(MathTest, RectOps) {
  math::Rect r1(0, 0, 10, 10);
  EXPECT_TRUE(r1.Contains(5, 5));
  EXPECT_FALSE(r1.Contains(15, 5));

  math::Rect r2(5, 5, 15, 15);
  EXPECT_TRUE(r1.Intersects(r2));

  math::Rect intersect = r1.GetIntersection(r2);
  EXPECT_EQ(intersect.x1, 5);
  EXPECT_EQ(intersect.y1, 5);
  EXPECT_EQ(intersect.x2, 10);
  EXPECT_EQ(intersect.y2, 10);

  math::Rect r3 = r1 | r2;
  // | is intersection. & is union.
  EXPECT_EQ(r3, intersect);
}

TEST(MathTest, PlaneOps) {
  math::Plane p({0.0f, 1.0f, 0.0f},
                {0.0f, 1.0f, 0.0f});  // Plane at y=1, normal pointing up
  EXPECT_FLOAT_EQ(p.Distance({0.0f, 2.0f, 0.0f}), 1.0f);
  EXPECT_FLOAT_EQ(p.Distance({0.0f, 0.0f, 0.0f}), -1.0f);
  EXPECT_EQ(p.Side({0.0f, 2.0f, 0.0f}), 1);
  EXPECT_EQ(p.Side({0.0f, 0.0f, 0.0f}), -1);
}

TEST(MathTest, SphereOps) {
  math::CSphere s({0.0f, 0.0f, 0.0f}, 1.0f);
  EXPECT_TRUE(s.Contains({0.5f, 0.0f, 0.0f}));
  EXPECT_FALSE(s.Contains({1.5f, 0.0f, 0.0f}));

  math::Plane p({0.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f});
  EXPECT_TRUE(s.Intersect(p));
}

TEST(MathTest, SpecMath) {
  EXPECT_NEAR(math::InvSqrt(4.0f), 0.5f, 0.001f);
  EXPECT_NEAR(math::DeGamma(0.04045f), 0.04045f / 12.92f, 0.00001f);
}

TEST(MathTest, Lerp) {
  EXPECT_FLOAT_EQ(math::Lerp(0.0f, 10.0f, 0.5f), 5.0f);
  math::Vector2 v1(0, 0), v2(10, 20);
  EXPECT_VEC2_EQ(math::Lerp(v1, v2, 0.5f), 5.0f, 10.0f);
}

TEST(MathTest, MatrixInversion) {
  math::Matrix4x4 m1 = math::Matrix4x4::Translation({10.0f, 0.0f, 0.0f});

  math::Matrix4x4 inv = m1.Inverted();
  EXPECT_FLOAT_EQ(inv.data()[3][0], -10.0f);

  math::Matrix4x4 res = m1 * inv;
  math::Matrix4x4 identity;
  identity.SetIdentity();
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      EXPECT_NEAR(res.data()[i][j], identity.data()[i][j], 0.0001f);
    }
  }
}
