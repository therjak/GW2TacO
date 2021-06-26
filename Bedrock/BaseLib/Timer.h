#pragma once

class CTimer
{
  int32_t LastUpdateTime;
  int32_t StartTime;
  TF32 SpeedModifier;
  double TimeExtension;
  TBOOL Paused;

  unsigned long Time;
public:

  CTimer();
  virtual ~CTimer();

  void Update();
  void SetSpeed( TF32 Speed );
  void Pause( TBOOL Pause );
  unsigned long GetTime();

  TBOOL isPaused();
  void SkipTime( unsigned long Time );
};

extern CTimer globalTimer;
//TF32 GetAccurateTime();
