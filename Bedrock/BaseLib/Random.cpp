#include "BaseLib.h"


TF32 CRandom::frand( TF32 min, TF32 max )
{
  return ( max - min )*frand() + min;
}

TF32 CRandom::frand()
{
  int32_t r = rand();
  return r / (TF32)RAND_MAX;
}

uint32_t CRandom::rand()
{
  Seed = Seed * 214013L + 2531011L;
  return ( Seed >> 16 )&RAND_MAX;
}

void CRandom::srand( uint32_t seed )
{
  Seed = seed;
}

CRandom::CRandom( uint32_t seed )
{
  srand( seed );
}

CRandom::CRandom()
{
  srand( 0 );
}
