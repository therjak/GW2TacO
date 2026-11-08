#include "src/base/timer.h"

#ifdef _WIN32
#include <windows.h>
// windows needs to be on the top.
#include <mmsystem.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
#else
#include <chrono>
#endif

#include <algorithm>

Timer global_timer;

#ifndef _WIN32
static int32_t GetCurrentMilliseconds() {
  using namespace std::chrono;
  static const auto start = steady_clock::now();
  return static_cast<int32_t>(
      duration_cast<milliseconds>(steady_clock::now() - start).count());
}
#endif

Timer::Timer() {
#ifdef _WIN32
  LastUpdateTime = StartTime = timeGetTime();
#else
  LastUpdateTime = StartTime = GetCurrentMilliseconds();
#endif
}

Timer::~Timer() = default;

void Timer::Update() {
#ifdef _WIN32
  int32_t CurrentTime = timeGetTime();
#else
  int32_t CurrentTime = GetCurrentMilliseconds();
#endif
  double ElapsedTime = CurrentTime - LastUpdateTime;

  Time = static_cast<int32_t>((CurrentTime - StartTime));
  LastUpdateTime = CurrentTime;
}

int32_t Timer::GetTime() { return Time; }
