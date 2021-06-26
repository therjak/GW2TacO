#include "BaseLib.h"
#include <MMSystem.h>
#pragma comment(lib,"winmm.lib")

CTimer globalTimer;

CTimer::CTimer()
{
  LastUpdateTime = StartTime = timeGetTime();
  SpeedModifier = 1;
  Paused = false;
  TimeExtension = 0;
}

CTimer::~CTimer()
{

}

void CTimer::Update()
{
  int32_t CurrentTime = timeGetTime();
  double ElapsedTime = CurrentTime - LastUpdateTime;

  if ( !Paused )
    TimeExtension += ElapsedTime*( 1 - SpeedModifier );
  else
    TimeExtension += ElapsedTime;

  Time = (int32_t)( ( CurrentTime - StartTime ) - TimeExtension );
  LastUpdateTime = CurrentTime;
}

void CTimer::SetSpeed( float Speed )
{
  SpeedModifier = max( 0, Speed );
}

void CTimer::Pause( TBOOL Pause )
{
  Paused = Pause;
}

unsigned long CTimer::GetTime()
{
  return Time;
}

TBOOL CTimer::isPaused()
{
  return Paused;
}

void CTimer::SkipTime( unsigned long Time )
{
  StartTime -= Time;
}

//float GetAccurateTime() 
//{
//
//	LARGE_INTEGER count, freq;
//	if (!GeCCo_LastPCV.QuadPart) {
//		QueryPerformanceCounter(&GeCCo_LastPCV);
//	}
//	QueryPerformanceCounter(&count);
//	QueryPerformanceFrequency(&freq);
//
//	GeCCo_SwitchFPUPrecisionMedium();
//
//	GeCCo_CurrentTime += (double)(count.QuadPart-GeCCo_LastPCV.QuadPart) / (double)(freq.QuadPart);
//
//	GeCCo_LastPCV=count;
//
//	return (float)(GeCCo_CurrentTime * 1000.0f);
//}
