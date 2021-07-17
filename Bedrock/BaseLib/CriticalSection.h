#pragma once

#include <cstdint>

volatile struct LIGHTWEIGHT_CRITICALSECTION {
  volatile int32_t threadID = 0;
  volatile int32_t spinCount = 0;
  volatile int32_t threadDibsID = 0;
};

#define THREAD_UNUSED 0

void InitializeLightweightCS(LIGHTWEIGHT_CRITICALSECTION* cs);

class CLightweightCriticalSection {
  LIGHTWEIGHT_CRITICALSECTION* cs;

 public:
  CLightweightCriticalSection(LIGHTWEIGHT_CRITICALSECTION* cs);
  ~CLightweightCriticalSection();
};
