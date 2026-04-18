#include <gtest/gtest.h>

import math;

TEST(MathTest, Vector2Addition) {
  math::Vector2 v1(1.0f, 2.0f);
  math::Vector2 v2(3.0f, 4.0f);
  math::Vector2 v3 = v1 + v2;
  EXPECT_FLOAT_EQ(v3.x, 4.0f);
  EXPECT_FLOAT_EQ(v3.y, 6.0f);
}

TEST(MathTest, Vector3Addition) {
  math::Vector3 v1(1.0f, 2.0f, 3.0f);
  math::Vector3 v2(4.0f, 5.0f, 6.0f);
  math::Vector3 v3 = v1 + v2;
  EXPECT_FLOAT_EQ(v3.x, 5.0f);
  EXPECT_FLOAT_EQ(v3.y, 7.0f);
  EXPECT_FLOAT_EQ(v3.z, 9.0f);
}

TEST(MathTest, Constants) {
  EXPECT_NEAR(math::PI, 3.14159f, 0.0001f);
}
