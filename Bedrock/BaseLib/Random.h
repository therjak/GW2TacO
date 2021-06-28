#pragma once

#include <cstdint>

class CRandom {
  uint32_t Seed;

 public:
  CRandom();
  CRandom(uint32_t seed);
  void srand(uint32_t seed);
  uint32_t rand();
  float frand();
  float frand(float min, float max);
};
