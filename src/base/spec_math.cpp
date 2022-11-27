#include "src/base/spec_math.h"

#include <cmath>

namespace math {

#ifdef FAST_INVSQRT
float InvSqrt(float x) {
  float xhalf = 0.5f * x;
  int32_t i = *(int32_t*)&x;       // get bits for TF32ing value
  i = 0x5f3759df - (i >> 1);       // gives initial guess y0
  x = *(float*)&i;                 // convert bits back to float
  x = x * (1.5f - xhalf * x * x);  // Newton step, repeating increases accuracy
  return x;
}
#else
float InvSqrt(float x) { return 1 / sqrtf(x); }

float DeGamma(float c) {
  if (c < 0.04045f) {
    return c / 12.92f;
  }
  return powf((c + 0.055f) / 1.055f, 2.4f);
}

#endif

}  // namespace math
