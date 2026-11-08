#pragma once

#include <cstdlib>
#include <cstring>
#include <string>

#include "src/base/win_compat.h"

inline HINSTANCE ShellExecuteA(HWND /*hwnd*/, LPCSTR lpOperation, LPCSTR lpFile,
                               LPCSTR /*lpParameters*/, LPCSTR /*lpDirectory*/,
                               INT /*nShowCmd*/) {
  if (lpFile && (lpOperation == nullptr || std::strcmp(lpOperation, "open") == 0)) {
    std::string cmd = std::string("xdg-open '") + lpFile + "' >/dev/null 2>&1 &";
    std::system(cmd.c_str());
    return reinterpret_cast<HINSTANCE>(42);
  }
  return nullptr;
}
#define ShellExecute ShellExecuteA
