#include "Random.h"

#include <cstdint>
#include <cstdlib>

float CRandom::frand(float min, float max) {
  return (max - min) * frand() + min;
}

float CRandom::frand() {
  int32_t r = rand();
  return r / (float)RAND_MAX;
}

uint32_t CRandom::rand() {
  Seed = Seed * 214013L + 2531011L;
  return (Seed >> 16) & RAND_MAX;
}

void CRandom::srand(uint32_t seed) { Seed = seed; }

CRandom::CRandom(uint32_t seed) { srand(seed); }

CRandom::CRandom() { srand(0); }
