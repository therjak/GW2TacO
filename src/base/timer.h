#pragma once

#include <cstdint>

class CTimer {
 public:
  CTimer();
  virtual ~CTimer();

  void Update();
  int32_t GetTime();

 private:
  int32_t LastUpdateTime;
  int32_t StartTime;

  int32_t Time = 0;
};

extern CTimer globalTimer;
