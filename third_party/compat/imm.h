#pragma once

#include "src/base/win_compat.h"

inline BOOL ImmDisableIME(DWORD /*idThread*/) {
  return TRUE;
}
