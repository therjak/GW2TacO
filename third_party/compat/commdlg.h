#pragma once

#include "src/base/win_compat.h"

struct OPENFILENAMEA {
  DWORD lStructSize;
  HWND hwndOwner;
  HINSTANCE hInstance;
  LPCSTR lpstrFilter;
  LPSTR lpstrCustomFilter;
  DWORD nMaxCustFilter;
  DWORD nFilterIndex;
  LPSTR lpstrFile;
  DWORD nMaxFile;
  LPSTR lpstrFileTitle;
  DWORD nMaxFileTitle;
  LPCSTR lpstrInitialDir;
  LPCSTR lpstrTitle;
  DWORD Flags;
  WORD nFileOffset;
  WORD nFileExtension;
  LPCSTR lpstrDefExt;
  LPARAM lCustData;
  void* lpfnHook;
  LPCSTR lpTemplateName;
  void* pvReserved;
  DWORD dwReserved;
  DWORD FlagsEx;
};
using OPENFILENAME = OPENFILENAMEA;
using LPOPENFILENAME = OPENFILENAMEA*;

constexpr DWORD OFN_OVERWRITEPROMPT = 0x00000002;
constexpr DWORD OFN_HIDEREADONLY = 0x00000004;
constexpr DWORD OFN_ALLOWMULTISELECT = 0x00000200;
constexpr DWORD OFN_PATHMUSTEXIST = 0x00000800;
constexpr DWORD OFN_FILEMUSTEXIST = 0x00001000;
constexpr DWORD OFN_NONETWORKBUTTON = 0x00020000;
constexpr DWORD OFN_EXPLORER = 0x00080000;

inline BOOL GetOpenFileNameA(LPOPENFILENAME /*lpofn*/) { return FALSE; }
inline BOOL GetSaveFileNameA(LPOPENFILENAME /*lpofn*/) { return FALSE; }
#define GetOpenFileName GetOpenFileNameA
#define GetSaveFileName GetSaveFileNameA
inline DWORD CommDlgExtendedError() { return 0; }
