#pragma once

#include "src/base/win_compat.h"

struct DWM_BLURBEHIND {
  DWORD dwFlags;
  BOOL fEnable;
  void* hRgnBlur;
  BOOL fTransitionOnMaximized;
};

struct MARGINS {
  int cxLeftWidth;
  int cxRightWidth;
  int cyTopHeight;
  int cyBottomHeight;
};

constexpr DWORD DWM_BB_ENABLE = 0x00000001;

inline HRESULT DwmEnableBlurBehindWindow(HWND /*hwnd*/,
                                        const DWM_BLURBEHIND* /*pBlurBehind*/) {
  return 0;
}

inline HRESULT DwmExtendFrameIntoClientArea(HWND /*hwnd*/,
                                           const MARGINS* /*pMarInset*/) {
  return 0;
}
