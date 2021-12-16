#pragma once

#include <cstdint>

struct LIGHTWEIGHT_CRITICALSECTION {
  volatile int32_t threadID = 0;
  volatile int32_t spinCount = 0;
  volatile int32_t threadDibsID = 0;
};

#define THREAD_UNUSED 0

class CLightweightCriticalSection {
  LIGHTWEIGHT_CRITICALSECTION* cs;

 public:
  explicit CLightweightCriticalSection(LIGHTWEIGHT_CRITICALSECTION* cs);
  ~CLightweightCriticalSection();
};
