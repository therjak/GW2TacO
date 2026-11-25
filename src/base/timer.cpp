#include "src/base/timer.h"

#include <algorithm>
#include <chrono>

Timer global_timer;

static int32_t GetCurrentMilliseconds() {
  using namespace std::chrono;
  static const auto start = steady_clock::now();
  return static_cast<int32_t>(
      duration_cast<milliseconds>(steady_clock::now() - start).count());
}

Timer::Timer() { LastUpdateTime = StartTime = GetCurrentMilliseconds(); }

Timer::~Timer() = default;

void Timer::Update() {
  int32_t CurrentTime = GetCurrentMilliseconds();
  double ElapsedTime = CurrentTime - LastUpdateTime;

  Time = static_cast<int32_t>((CurrentTime - StartTime));
  LastUpdateTime = CurrentTime;
}

int32_t Timer::GetTime() { return Time; }
