#pragma once

#ifndef _WIN32

#include <unistd.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <utility>

// Common Windows types
using DWORD = uint32_t;
using LPDWORD = DWORD*;
using WORD = uint16_t;
using BYTE = uint8_t;
using BOOL = int;
using UINT = unsigned int;
using INT = int;
using LONG = int32_t;
using ULONG = uint32_t;
using LONGLONG = int64_t;
using ULONGLONG = uint64_t;
using UINT_PTR = uintptr_t;
using LONG_PTR = intptr_t;
using ULONG_PTR = uintptr_t;
using WPARAM = uintptr_t;
using LPARAM = intptr_t;
using LRESULT = intptr_t;
using LPVOID = void*;
using LPCVOID = const void*;
using LPSTR = char*;
using LPCSTR = const char*;
using LPWSTR = wchar_t*;
using LPCWSTR = const wchar_t*;
using HANDLE = void*;
using HINSTANCE = void*;
using HMODULE = void*;
using HWND = void*;
using HICON = void*;
using HCURSOR = void*;
using HMENU = void*;
using HDC = void*;
using HMONITOR = void*;
using HKEY = void*;
using HGLOBAL = void*;
using HRGN = void*;
using HRESULT = int32_t;
using TCHAR = char;
using CHAR = char;

constexpr size_t MAX_PATH = 260;

#ifndef WINAPI
#define WINAPI
#endif
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef _In_
#define _In_
#endif
#ifndef _In_opt_
#define _In_opt_
#endif
#ifndef _Out_
#define _Out_
#endif
#ifndef _T
#define _T(x) x
#endif
#ifndef _TEXT
#define _TEXT(x) x
#endif

using SHORT = int16_t;
using USHORT = uint16_t;
using DWORD_PTR = uintptr_t;
using PVOID = void*;

#ifndef LOWORD
#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#endif
#ifndef HIWORD
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#endif
#ifndef MAKELPARAM
#define MAKELPARAM(l, h)                          \
  ((LPARAM)(((WORD)(((DWORD_PTR)(l)) & 0xffff)) | \
            ((DWORD)((WORD)(((DWORD_PTR)(h)) & 0xffff))) << 16))
#endif
#ifndef MAKEWPARAM
#define MAKEWPARAM(l, h)                          \
  ((WPARAM)(((WORD)(((DWORD_PTR)(l)) & 0xffff)) | \
            ((DWORD)((WORD)(((DWORD_PTR)(h)) & 0xffff))) << 16))
#endif
#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wParam) ((short)HIWORD(wParam))
#endif
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

// Windows Message IDs
constexpr UINT WM_NULL = 0x0000;
constexpr UINT WM_CREATE = 0x0001;
constexpr UINT WM_DESTROY = 0x0002;
constexpr UINT WM_MOVE = 0x0003;
constexpr UINT WM_SIZE = 0x0005;
constexpr UINT WM_ACTIVATE = 0x0006;
constexpr UINT WM_SETFOCUS = 0x0007;
constexpr UINT WM_KILLFOCUS = 0x0008;
constexpr UINT WM_PAINT = 0x000F;
constexpr UINT WM_CLOSE = 0x0010;
constexpr UINT WM_QUIT = 0x0012;
constexpr UINT WM_COPYDATA = 0x004A;
constexpr UINT WM_KEYDOWN = 0x0100;
constexpr UINT WM_KEYUP = 0x0101;
constexpr UINT WM_CHAR = 0x0102;
constexpr UINT WM_SYSKEYDOWN = 0x0104;
constexpr UINT WM_SYSKEYUP = 0x0105;
constexpr UINT WM_SYSCHAR = 0x0106;
constexpr UINT WM_MOUSEMOVE = 0x0200;
constexpr UINT WM_LBUTTONDOWN = 0x0201;
constexpr UINT WM_LBUTTONUP = 0x0202;
constexpr UINT WM_LBUTTONDBLCLK = 0x0203;
constexpr UINT WM_RBUTTONDOWN = 0x0204;
constexpr UINT WM_RBUTTONUP = 0x0205;
constexpr UINT WM_RBUTTONDBLCLK = 0x0206;
constexpr UINT WM_MBUTTONDOWN = 0x0207;
constexpr UINT WM_MBUTTONUP = 0x0208;
constexpr UINT WM_MBUTTONDBLCLK = 0x0209;
constexpr UINT WM_MOUSEWHEEL = 0x020A;

// Virtual Keys
constexpr int VK_LBUTTON = 0x01;
constexpr int VK_RBUTTON = 0x02;
constexpr int VK_CANCEL = 0x03;
constexpr int VK_MBUTTON = 0x04;
constexpr int VK_BACK = 0x08;
constexpr int VK_TAB = 0x09;
constexpr int VK_CLEAR = 0x0C;
constexpr int VK_RETURN = 0x0D;
constexpr int VK_SHIFT = 0x10;
constexpr int VK_CONTROL = 0x11;
constexpr int VK_MENU = 0x12;
constexpr int VK_PAUSE = 0x13;
constexpr int VK_CAPITAL = 0x14;
constexpr int VK_ESCAPE = 0x1B;
constexpr int VK_SPACE = 0x20;
constexpr int VK_PRIOR = 0x21;
constexpr int VK_NEXT = 0x22;
constexpr int VK_END = 0x23;
constexpr int VK_HOME = 0x24;
constexpr int VK_LEFT = 0x25;
constexpr int VK_UP = 0x26;
constexpr int VK_RIGHT = 0x27;
constexpr int VK_DOWN = 0x28;
constexpr int VK_SNAPSHOT = 0x2C;
constexpr int VK_INSERT = 0x2D;
constexpr int VK_DELETE = 0x2E;
constexpr int VK_F1 = 0x70;
constexpr int VK_F2 = 0x71;
constexpr int VK_F3 = 0x72;
constexpr int VK_F4 = 0x73;
constexpr int VK_F5 = 0x74;
constexpr int VK_F6 = 0x75;
constexpr int VK_F7 = 0x76;
constexpr int VK_F8 = 0x77;
constexpr int VK_F9 = 0x78;
constexpr int VK_F10 = 0x79;
constexpr int VK_F11 = 0x7A;
constexpr int VK_F12 = 0x7B;
constexpr int VK_LSHIFT = 0xA0;
constexpr int VK_RSHIFT = 0xA1;
constexpr int VK_LCONTROL = 0xA2;
constexpr int VK_RCONTROL = 0xA3;
constexpr int VK_LMENU = 0xA4;
constexpr int VK_RMENU = 0xA5;

constexpr UINT SPI_GETKEYBOARDDELAY = 0x0016;
constexpr UINT SPI_GETKEYBOARDSPEED = 0x000A;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef S_OK
#define S_OK ((HRESULT)0L)
#endif
#ifndef S_FALSE
#define S_FALSE ((HRESULT)1L)
#endif
#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif
#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#endif

#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef _TRUNCATE
#define _TRUNCATE ((size_t)-1)
#endif

// Structs
struct GUID {
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t Data4[8];

  bool operator==(const GUID& o) const {
    return std::memcmp(this, &o, sizeof(GUID)) == 0;
  }
  bool operator!=(const GUID& o) const { return !(*this == o); }
};

struct POINT {
  LONG x;
  LONG y;
};

struct RECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
};

struct WINDOWPLACEMENT {
  UINT length;
  UINT flags;
  UINT showCmd;
  POINT ptMinPosition;
  POINT ptMaxPosition;
  RECT rcNormalPosition;
};

// CRT secure function mappings
inline int fopen_s(FILE** pFile, const char* filename, const char* mode) {
  if (!pFile || !filename || !mode) return -1;
  *pFile = fopen(filename, mode);
  return (*pFile != nullptr) ? 0 : -1;
}

inline int strncpy_s(char* dest, size_t destsz, const char* src, size_t count) {
  if (!dest || destsz == 0) return -1;
  if (!src) {
    dest[0] = '\0';
    return -1;
  }
  if (count == _TRUNCATE) {
    strncpy(dest, src, destsz - 1);
    dest[destsz - 1] = '\0';
    return 0;
  }
  if (count >= destsz) {
    strncpy(dest, src, destsz - 1);
    dest[destsz - 1] = '\0';
    return -1;
  }
  strncpy(dest, src, count);
  dest[count] = '\0';
  return 0;
}

template <typename... Args>
inline int sscanf_s(const char* buffer, const char* format, Args&&... args) {
  return sscanf(buffer, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline int _snprintf_s(char* buffer, size_t sizeOfBuffer, size_t count,
                       const char* format, Args&&... args) {
  (void)count;
  return snprintf(buffer, sizeOfBuffer, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline int _snprintf_s(char* buffer, size_t sizeOfBuffer, const char* format,
                       Args&&... args) {
  return snprintf(buffer, sizeOfBuffer, format, std::forward<Args>(args)...);
}

inline int localtime_s(struct tm* tmDest, const time_t* sourceTime) {
  if (!tmDest || !sourceTime) return -1;
  return (localtime_r(sourceTime, tmDest) != nullptr) ? 0 : -1;
}

inline int gmtime_s(struct tm* tmDest, const time_t* sourceTime) {
  if (!tmDest || !sourceTime) return -1;
  return (gmtime_r(sourceTime, tmDest) != nullptr) ? 0 : -1;
}

inline HRESULT CoCreateGuid(GUID* pguid) {
  if (!pguid) return -1;
  for (int i = 0; i < 8; ++i) {
    pguid->Data4[i] = static_cast<uint8_t>(std::rand() & 0xff);
  }
  pguid->Data1 = static_cast<uint32_t>(std::rand() ^ (std::rand() << 16));
  pguid->Data2 = static_cast<uint16_t>(std::rand());
  pguid->Data3 = static_cast<uint16_t>(std::rand());
  return 0;
}

inline HMODULE GetModuleHandle(const char* /*lpModuleName*/) { return nullptr; }

inline BOOL SetCurrentDirectory(const char* lpPathName) {
  return chdir(lpPathName) == 0;
}

inline DWORD GetCurrentDirectory(DWORD nBufferLength, char* lpBuffer) {
  if (!lpBuffer || nBufferLength == 0) return 0;
  if (getcwd(lpBuffer, nBufferLength)) {
    return static_cast<DWORD>(strlen(lpBuffer));
  }
  return 0;
}

inline time_t _mkgmtime(struct tm* tm) { return timegm(tm); }

inline HWND FindWindow(const char* /*lpClassName*/,
                       const char* /*lpWindowName*/) {
  return nullptr;
}

constexpr int SW_HIDE = 0;
constexpr int SW_SHOWNORMAL = 1;
constexpr int SW_SHOW = 5;
constexpr int SW_SHOWMAXIMIZED = 3;

inline POINT g_cursor_pos = {0, 0};

inline BOOL GetCursorPos(POINT* pt) {
  if (pt) *pt = g_cursor_pos;
  return TRUE;
}

inline BOOL ScreenToClient(HWND /*hwnd*/, POINT* /*pt*/) { return TRUE; }

inline HWND SetCapture(HWND hwnd) { return hwnd; }
inline BOOL ReleaseCapture() { return TRUE; }

inline SHORT GetKeyState(int /*vKey*/) { return 0; }
inline SHORT GetAsyncKeyState(int /*vKey*/) { return 0; }

inline BOOL SystemParametersInfo(UINT /*uiAction*/, UINT /*uiParam*/,
                                 void* /*pvParam*/, UINT /*fWinIni*/) {
  return FALSE;
}

constexpr UINT CF_TEXT = 1;
constexpr UINT CF_UNICODETEXT = 13;
constexpr UINT GMEM_DDESHARE = 0x2000;
constexpr UINT GHND = 0x0042;

inline DWORD GetLastError() { return 0; }

inline HGLOBAL GlobalAlloc(UINT /*uFlags*/, size_t dwBytes) {
  return std::malloc(dwBytes);
}
inline void* GlobalLock(HGLOBAL hMem) { return hMem; }
inline BOOL GlobalUnlock(HGLOBAL /*hMem*/) { return TRUE; }
inline HGLOBAL GlobalFree(HGLOBAL hMem) {
  std::free(hMem);
  return nullptr;
}
inline BOOL OpenClipboard(HWND /*hWndNewOwner*/) { return FALSE; }
inline BOOL CloseClipboard() { return TRUE; }
inline BOOL EmptyClipboard() { return TRUE; }
inline HANDLE SetClipboardData(UINT /*uFormat*/, HANDLE hMem) { return hMem; }
inline HANDLE GetClipboardData(UINT /*uFormat*/) { return nullptr; }
inline BOOL IsClipboardFormatAvailable(UINT /*format*/) { return FALSE; }

inline BOOL CreateDirectory(const char* lpPathName,
                            void* /*lpSecurityAttributes*/) {
  std::error_code ec;
  return std::filesystem::create_directory(lpPathName, ec);
}

struct COPYDATASTRUCT {
  ULONG_PTR dwData;
  DWORD cbData;
  PVOID lpData;
};

constexpr DWORD MSGFLT_ALLOW = 1;
inline BOOL ChangeWindowMessageFilterEx(HWND /*hwnd*/, UINT /*message*/,
                                        DWORD /*action*/,
                                        void* /*pChangeFilterStruct*/) {
  return TRUE;
}

constexpr UINT MB_OK = 0x00000000;
constexpr UINT MB_ICONERROR = 0x00000010;
constexpr UINT MB_ICONWARNING = 0x00000030;
constexpr UINT MB_ICONINFORMATION = 0x00000040;

inline int MessageBoxA(HWND /*hWnd*/, LPCSTR lpText, LPCSTR lpCaption,
                       UINT /*uType*/) {
  std::fprintf(stderr, "[MessageBox] %s: %s\n", lpCaption ? lpCaption : "Alert",
               lpText ? lpText : "");
  return 1;
}
#define MessageBox MessageBoxA

inline const char* GetCommandLineA() {
  static std::string s_cmdline;
  if (s_cmdline.empty()) {
    FILE* f = fopen("/proc/self/cmdline", "r");
    if (f) {
      char buf[1024];
      size_t n = fread(buf, 1, sizeof(buf) - 1, f);
      fclose(f);
      for (size_t i = 0; i < n; ++i) {
        if (buf[i] == '\0' && i + 1 < n) buf[i] = ' ';
      }
      buf[n] = '\0';
      s_cmdline = buf;
    }
  }
  return s_cmdline.c_str();
}

inline DWORD GetModuleFileName(HMODULE /*hModule*/, char* lpFilename,
                               DWORD nSize) {
  if (!lpFilename || nSize == 0) return 0;
  ssize_t len = readlink("/proc/self/exe", lpFilename, nSize - 1);
  if (len > 0) {
    lpFilename[len] = '\0';
    return static_cast<DWORD>(len);
  }
  return 0;
}

inline HRGN CreateRectRgn(int /*x1*/, int /*y1*/, int /*x2*/, int /*y2*/) {
  return nullptr;
}

inline HMODULE LoadLibraryA(const char* /*lpLibFileName*/) { return nullptr; }
#define LoadLibrary LoadLibraryA
inline BOOL FreeLibrary(HMODULE /*hLibModule*/) { return TRUE; }
inline void* GetProcAddress(HMODULE /*hModule*/, const char* /*lpProcName*/) {
  return nullptr;
}

inline BOOL PostMessage(HWND /*hWnd*/, UINT /*Msg*/, WPARAM /*wParam*/,
                        LPARAM /*lParam*/) {
  return TRUE;
}
inline LRESULT SendMessage(HWND /*hWnd*/, UINT /*Msg*/, WPARAM /*wParam*/,
                           LPARAM /*lParam*/) {
  return 0;
}
inline BOOL ShowWindow(HWND /*hWnd*/, int /*nCmdShow*/) { return TRUE; }
inline void Sleep(DWORD ms) { usleep(ms * 1000); }

inline HWND GetForegroundWindow() { return nullptr; }
inline DWORD GetWindowThreadProcessId(HWND /*hWnd*/, DWORD* lpdwProcessId) {
  if (lpdwProcessId) *lpdwProcessId = 0;
  return 0;
}
inline BOOL GetClientRect(HWND /*hWnd*/, RECT* lpRect) {
  if (lpRect) *lpRect = {0, 0, 800, 600};
  return TRUE;
}
inline BOOL ClientToScreen(HWND /*hWnd*/, POINT* /*lpPoint*/) { return TRUE; }
inline BOOL SetLayeredWindowAttributes(HWND /*hwnd*/, DWORD /*crKey*/,
                                       BYTE /*bAlpha*/, DWORD /*dwFlags*/) {
  return TRUE;
}

constexpr DWORD LWA_ALPHA = 2;
constexpr DWORD WS_EX_TOPMOST = 0x00000008;
constexpr DWORD WS_EX_TRANSPARENT = 0x00000020;
constexpr DWORD WS_EX_TOOLWINDOW = 0x00000080;
constexpr DWORD WS_EX_LAYERED = 0x00080000;
constexpr DWORD WS_EX_NOREDIRECTIONBITMAP = 0x00200000;
constexpr DWORD WS_EX_COMPOSITED = 0x02000000;
constexpr DWORD WS_POPUP = 0x80000000;

inline HICON LoadIcon(HINSTANCE /*hInstance*/, const char* /*lpIconName*/) {
  return nullptr;
}
#define MAKEINTRESOURCE(i) ((char*)(intptr_t)(i))

constexpr DWORD DWM_BB_BLURREGION = 0x00000002;
constexpr DWORD PROCESS_PER_MONITOR_DPI_AWARE = 2;
using PROCESS_DPI_AWARENESS = int;
constexpr DWORD PROCESS_QUERY_INFORMATION = 0x0400;
inline HANDLE OpenProcess(DWORD /*dwDesiredAccess*/, BOOL /*bInheritHandle*/,
                          DWORD /*dwProcessId*/) {
  return nullptr;
}
inline HANDLE GetCurrentProcess() {
  return reinterpret_cast<HANDLE>(intptr_t(-1));
}
inline BOOL CloseHandle(HANDLE /*hObject*/) { return TRUE; }
inline DWORD GetProcessId(HANDLE /*hProcess*/) {
  return static_cast<DWORD>(getpid());
}

inline int GetClassNameA(HWND /*hWnd*/, LPSTR lpClassName, int nMaxCount) {
  if (lpClassName && nMaxCount > 0) lpClassName[0] = '\0';
  return 0;
}

inline BOOL DeleteObject(HRGN /*ho*/) { return TRUE; }
inline HWND SetFocus(HWND /*hWnd*/) { return nullptr; }
inline HWND GetNextWindow(HWND /*hWnd*/, UINT /*wCmd*/) { return nullptr; }
constexpr UINT GW_HWNDPREV = 3;

inline BOOL SetWindowPos(HWND /*hWnd*/, HWND /*hWndInsertAfter*/, int /*X*/,
                         int /*Y*/, int /*cx*/, int /*cy*/, UINT /*uFlags*/) {
  return TRUE;
}
constexpr UINT SWP_NOREPOSITION = 0x0200;
constexpr UINT SWP_NOMOVE = 0x0002;
constexpr UINT SWP_NOSIZE = 0x0001;
#define HWND_TOPMOST ((HWND)(intptr_t)-1)
inline BOOL SetForegroundWindow(HWND /*hWnd*/) { return TRUE; }

// Registry stubs
#define HKEY_CLASSES_ROOT ((HKEY)(intptr_t)0x80000000)
constexpr DWORD REG_OPTION_NON_VOLATILE = 0;
constexpr LONG ERROR_SUCCESS = 0L;
constexpr DWORD REG_SZ = 1;

inline LONG RegCreateKeyEx(HKEY, LPCSTR, DWORD, LPSTR, DWORD, DWORD, void*,
                           HKEY*, DWORD*) {
  return ERROR_SUCCESS;
}
inline LONG RegOpenKey(HKEY, LPCSTR, HKEY*) { return ERROR_SUCCESS; }
inline LONG RegCloseKey(HKEY) { return ERROR_SUCCESS; }
inline LONG RegSetKeyValue(HKEY, LPCSTR, LPCSTR, DWORD, LPCVOID, DWORD) {
  return ERROR_SUCCESS;
}

#endif  // !_WIN32
