#pragma once

#define RAND_MAX 0x7fff

class CRandom
{
  uint32_t Seed;

public:

  CRandom();
  CRandom( uint32_t seed );
  void srand( uint32_t seed );
  uint32_t rand();
  TF32 frand();
  TF32 frand( TF32 min, TF32 max );
};
