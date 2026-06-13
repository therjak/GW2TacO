#pragma once

#include <cstdint>

class Timer {
 public:
  Timer();
  virtual ~Timer();

  void Update();
  int32_t GetTime();

 private:
  int32_t LastUpdateTime;
  int32_t StartTime;

  int32_t Time = 0;
};

extern Timer global_timer;
