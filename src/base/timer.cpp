#include "src/base/timer.h"

#include <windows.h>
// windows needs to be on the top.
#include <mmsystem.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

#include <algorithm>

Timer global_timer;

Timer::Timer() { LastUpdateTime = StartTime = timeGetTime(); }

Timer::~Timer() = default;

void Timer::Update() {
  int32_t CurrentTime = timeGetTime();
  double ElapsedTime = CurrentTime - LastUpdateTime;

  Time = static_cast<int32_t>((CurrentTime - StartTime));
  LastUpdateTime = CurrentTime;
}

int32_t Timer::GetTime() { return Time; }
