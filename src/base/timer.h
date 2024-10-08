#pragma once

#include <cstdint>

class CTimer {
 public:
  CTimer();
  virtual ~CTimer();

  void Update();
  void SetSpeed(float Speed);
  void Pause(bool Pause);
  unsigned long GetTime();

  bool isPaused();
  void SkipTime(unsigned long Time);

 private:
  int32_t LastUpdateTime;
  int32_t StartTime;
  float SpeedModifier = 1;
  double TimeExtension = 0;
  bool Paused = false;

  unsigned long Time = 0;
};

extern CTimer globalTimer;
