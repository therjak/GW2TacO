#include "BaseLib.h"


CFPUDouble::~CFPUDouble()
{
#ifndef _WIN64
  if ( OriginalFPUState )
    _controlfp_s( &OriginalFPUState, OriginalFPUState, 0xffffffff );
#endif
}

CFPUDouble::CFPUDouble()
{
  OriginalFPUState = 0;
  //store fpu state
  if ( !_controlfp_s( &OriginalFPUState, 0, 0 ) )
  {
    uint32_t tmp;
    //set double fpu precision
    _controlfp_s( &tmp, _PC_64, _MCW_PC );
  }
}

CFPUAnsiCRounding::~CFPUAnsiCRounding()
{
#ifndef _WIN64
  if ( OriginalFPUState )
    _controlfp_s( &OriginalFPUState, OriginalFPUState, 0xffffffff );
#endif
}

CFPUAnsiCRounding::CFPUAnsiCRounding()
{
  OriginalFPUState = 0;
  //store fpu state
  if ( !_controlfp_s( &OriginalFPUState, 0, 0 ) )
  {
    uint32_t tmp;
    //set rounding mode
    _controlfp_s( &tmp, _RC_CHOP, _MCW_RC );
  }
  else OriginalFPUState = 0;
}
