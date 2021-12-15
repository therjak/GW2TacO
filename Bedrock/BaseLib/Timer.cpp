#include "Timer.h"

#include <windows.h>
// windows needs to be on the top.
#include <mmsystem.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

#include <algorithm>

CTimer globalTimer;

CTimer::CTimer() { LastUpdateTime = StartTime = timeGetTime(); }

CTimer::~CTimer() = default;

void CTimer::Update() {
  int32_t CurrentTime = timeGetTime();
  double ElapsedTime = CurrentTime - LastUpdateTime;

  if (!Paused)
    TimeExtension += ElapsedTime * (1 - SpeedModifier);
  else
    TimeExtension += ElapsedTime;

  Time = static_cast<int32_t>((CurrentTime - StartTime) - TimeExtension);
  LastUpdateTime = CurrentTime;
}

void CTimer::SetSpeed(float Speed) { SpeedModifier = std::max(0.f, Speed); }

void CTimer::Pause(bool Pause) { Paused = Pause; }

unsigned long CTimer::GetTime() { return Time; }

bool CTimer::isPaused() { return Paused; }

void CTimer::SkipTime(unsigned long Time) { StartTime -= Time; }
