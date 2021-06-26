#pragma once

//profiling class - measures the length the instance has been alive

class CProfile
{
  uint32_t Time;
  uint64_t Key;
  CString Label;

public:
  CProfile( uint64_t, const CString &label );
  ~CProfile( void );

};
