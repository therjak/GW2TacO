#pragma once

#include "src/base/win_compat.h"

inline HRESULT URLOpenBlockingStreamA(void* /*pCaller*/, const char* /*szURL*/,
                                     void** /*ppStream*/, DWORD /*dwReserved*/,
                                     void* /*lpfnCB*/) {
  return -1;
}

#define URLOpenBlockingStream URLOpenBlockingStreamA
