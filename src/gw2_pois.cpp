﻿#include <dwmapi.h>
#include <process.h>
#include <windowsx.h>

// Needs windows
#include <TlHelp32.h>
#include <imm.h>
#include <winhttp.h>

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include "src/base/logger.h"
#include "src/base/string_format.h"
#include "src/base/timer.h"
#include "src/build_info.h"
#include "src/gw2_api.h"
#include "src/gw2_taco.h"
#include "src/gw2_tactical.h"
#include "src/hp_grid.h"
#include "src/language.h"
#include "src/locational_timer.h"
#include "src/map_timer.h"
#include "src/mouse_highlight.h"
#include "src/mumble_link.h"
#include "src/overlay_application.h"
#include "src/overlay_config.h"
#include "src/overlay_window.h"
#include "src/pro_font.h"
#include "src/range_display.h"
#include "src/resource.h"
#include "src/special_gui_items.h"
#include "src/tactical_compass.h"
#include "src/trail_logger.h"
#include "src/util/miniz.h"

#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Shell32.lib")

using math::CRect;

std::unique_ptr<CWBApplication> App;
HWND gw2Window;
HWND gw2WindowFromPid = nullptr;

bool disableHooks = false;

bool InitGUI(CWBApplication* App) {
  CreateUniFontOutlined(App, "UniFontOutlined");
  CreateUniFontOutlined(App, "ProFontOutlined");
  CreateUniFont(App, "UniFont");
  CreateProFont(App, "ProFont");

  if (!App->LoadSkinFromFile("UI.wbs", localization->GetUsedGlyphs())) {
    MessageBox(nullptr,
               "TacO can't find the UI.wbs ui skin file!\nPlease make sure you "
               "extracted all the files from the archive to a separate folder!",
               "Missing File!", MB_ICONERROR);
    return false;
  }
  if (!App->LoadXMLLayoutFromFile("UI.xml")) {
    MessageBox(
        nullptr,
        "TacO can't find the UI.xml ui layout file!\nPlease make sure you "
        "extracted all the files from the archive to a separate folder!",
        "Missing File!", MB_ICONERROR);
    return false;
  }
  if (!App->LoadCSSFromFile(UIFileNames[GetConfigValue("InterfaceSize")])) {
    MessageBox(
        nullptr,
        "TacO can't find a required UI css style file!\nPlease make sure you "
        "extracted all the files from the archive to a separate folder!",
        "Missing File!", MB_ICONERROR);
    return false;
  }
  App->RegisterUIFactoryCallback("GW2TacticalDisplay",
                                 GW2TacticalDisplay::Factory);
  App->RegisterUIFactoryCallback("GW2TrailDisplay", GW2TrailDisplay::Factory);
  App->RegisterUIFactoryCallback("GW2MapTimer", GW2MapTimer::Factory);
  App->RegisterUIFactoryCallback("MouseHighlight", GW2MouseHighlight::Factory);
  App->RegisterUIFactoryCallback("GW2TacO", GW2TacO::Factory);
  App->RegisterUIFactoryCallback("OverlayWindow", OverlayWindow::Factory);
  App->RegisterUIFactoryCallback("TimerDisplay", TimerDisplay::Factory);
  App->RegisterUIFactoryCallback("GW2TacticalCompass",
                                 GW2TacticalCompass::Factory);
  App->RegisterUIFactoryCallback("GW2RangeDisplay", GW2RangeDisplay::Factory);
  App->RegisterUIFactoryCallback("HPGrid", GW2HPGrid::Factory);
  App->RegisterUIFactoryCallback("clickthroughbutton",
                                 ClickThroughButton::Factory);

  App->GenerateGUI(App->GetRoot(), "gw2pois");

  App->ReApplyStyle();

  return true;
}

void OpenWindows(CWBApplication* App) {
  auto root = App->GetRoot();
  auto* taco =
      dynamic_cast<GW2TacO*>(root->FindChildByID("tacoroot", "GW2TacO"));
  if (!taco) {
    return;
  }

  if (HasWindowData("MapTimer") && IsWindowOpen("MapTimer")) {
    taco->OpenWindow("MapTimer");
  }

  if (HasWindowData("TS3Control") && IsWindowOpen("TS3Control")) {
    taco->OpenWindow("TS3Control");
  }

  if (HasWindowData("MarkerEditor") && IsWindowOpen("MarkerEditor")) {
    taco->OpenWindow("MarkerEditor");
  }

  if (HasWindowData("Notepad") && IsWindowOpen("Notepad")) {
    taco->OpenWindow("Notepad");
  }

  if (HasWindowData("RaidProgress") && IsWindowOpen("RaidProgress")) {
    taco->OpenWindow("RaidProgress");
  }

  if (HasWindowData("DungeonProgress") && IsWindowOpen("DungeonProgress")) {
    taco->OpenWindow("DungeonProgress");
  }

  if (HasWindowData("TPTracker") && IsWindowOpen("TPTracker")) {
    taco->OpenWindow("TPTracker");
  }
}

bool ShiftState = false;

LRESULT __stdcall MyKeyboardProc(int ccode, WPARAM wParam, LPARAM lParam) {
  if (disableHooks) {
    return CallNextHookEx(nullptr, ccode, wParam, lParam);
  }

  if (ccode == HC_ACTION) {
    auto* pkbdllhook = (KBDLLHOOKSTRUCT*)lParam;
    HKL dwhkl = nullptr;
    BYTE dbKbdState[256];
    TCHAR szCharBuf[32] = {};
    static KBDLLHOOKSTRUCT lastState = {0};

    GetKeyboardState(dbKbdState);
    dwhkl = GetKeyboardLayout(
        GetWindowThreadProcessId(GetForegroundWindow(), nullptr));

    if (ToAsciiEx(pkbdllhook->vkCode, pkbdllhook->scanCode, dbKbdState,
                  reinterpret_cast<LPWORD>(szCharBuf), 0, dwhkl) == -1) {
      // PostMessage( (HWND)App->GetHandle(), WM_DEADCHAR, pkbdllhook->vkCode, 1
      // | ( pkbdllhook->scanCode << 16 ) + ( pkbdllhook->flags << 24 ) );

      // Save the current keyboard state.
      lastState = *pkbdllhook;
      // You might also need to hang onto the dbKbdState array... I'm thinking
      // not. Clear out the buffer to return to the previous state - wait for
      // ToAsciiEx to return a value other than -1 by passing the same key
      // again. It should happen after 1 call.
      while (ToAsciiEx(pkbdllhook->vkCode, pkbdllhook->scanCode, dbKbdState,
                       reinterpret_cast<LPWORD>(szCharBuf), 0, dwhkl) < 0) {
        ;
      }
    } else {
      // Do something with szCharBuf here since this will overwrite it...
      // If we have a saved vkCode from last call, it was a dead key we need to
      // place back in the buffer.
      if (lastState.vkCode != 0) {
        // Safest to just clear this.
        memset(dbKbdState, 0, 256);
        // Put the old vkCode back into the locale's buffer.
        ToAsciiEx(lastState.vkCode, lastState.scanCode, dbKbdState,
                  reinterpret_cast<LPWORD>(szCharBuf), 0, dwhkl);
        // Set vkCode to 0, we can use this as a flag as a vkCode of 0 is
        // invalid.
        lastState.vkCode = 0;
      }
    }
  }

  if (ccode < 0) {
    return CallNextHookEx(nullptr, ccode, wParam, lParam);
  }

  if (wParam != WM_KEYDOWN && wParam != WM_KEYUP && wParam != WM_CHAR &&
      wParam != WM_DEADCHAR && wParam != WM_UNICHAR) {
    return CallNextHookEx(nullptr, ccode, wParam, lParam);
  }

  auto wnd = GetForegroundWindow();
  if (ccode != HC_ACTION || !lParam ||
      (wnd != gw2Window && App && wnd != App->GetHandle())) {
    return CallNextHookEx(nullptr, ccode, wParam, lParam);
  }

  if (App && wnd == App->GetHandle()) {
    return CallNextHookEx(nullptr, ccode, wParam, lParam);
  }

  auto* kbdat = (KBDLLHOOKSTRUCT*)lParam;
  PostMessage(App->GetHandle(), wParam, kbdat->vkCode,
              1 | (kbdat->scanCode << 16) + (kbdat->flags << 24));

  return (CallNextHookEx(nullptr, ccode, wParam, lParam));
}

LRESULT __stdcall KeyboardHook(int code, WPARAM wParam, LPARAM lParam) {
  if (disableHooks || mumbleLink.textboxHasFocus) {
    return CallNextHookEx(nullptr, code, wParam, lParam);
  }

  // !!!!!!!!!!!!!
  // https://stackoverflow.com/questions/3548932/keyboard-hook-changes-the-behavior-of-keys

  if (code < 0) {
    return CallNextHookEx(nullptr, code, wParam, lParam);
  }

  if (wParam != WM_KEYDOWN && wParam != WM_KEYUP && wParam != WM_CHAR &&
      wParam != WM_DEADCHAR && wParam != WM_UNICHAR) {
    return CallNextHookEx(nullptr, code, wParam, lParam);
  }

  auto wnd = GetForegroundWindow();
  if (code != HC_ACTION || !lParam ||
      (wnd != gw2Window && App && wnd != App->GetHandle())) {
    return CallNextHookEx(nullptr, code, wParam, lParam);
  }

  if (App && wnd == App->GetHandle()) {
    return CallNextHookEx(nullptr, code, wParam, lParam);
  }

  auto* kbdat = (KBDLLHOOKSTRUCT*)lParam;
  UINT mapped = MapVirtualKey(kbdat->vkCode, MAPVK_VK_TO_CHAR);

  bool inFocus =
      App->GetFocusItem() && App->GetFocusItem()->InstanceOf("textbox");

  if (mapped & (1 << 31) && !inFocus) {
    return CallNextHookEx(nullptr, 0, wParam, lParam);
  }

  if (!inFocus) {
    if (wParam == WM_KEYDOWN) {
      App->InjectMessage(WM_CHAR, mapped, 0);
      return CallNextHookEx(nullptr, 0, WM_KEYDOWN, lParam);
    }
    return CallNextHookEx(nullptr, 0, wParam, lParam);
  }

  PostMessage(App->GetHandle(), wParam, kbdat->vkCode,
              1 | (kbdat->scanCode << 16) + (kbdat->flags << 24));
  return 1;
}

LRESULT __stdcall MouseHook(int code, WPARAM wParam, LPARAM lParam) {
  if (disableHooks) {
    return CallNextHookEx(nullptr, code, wParam, lParam);
  }

  auto wnd = GetForegroundWindow();
  if (code < 0 || !lParam ||
      (wnd != gw2Window && App && wnd != App->GetHandle())) {
    return CallNextHookEx(nullptr, code, wParam, lParam);
  }

  auto* mousedat = (MSLLHOOKSTRUCT*)lParam;

  POINT ap = mousedat->pt;
  ScreenToClient(App->GetHandle(), &ap);

  if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN) {
    PostMessage(App->GetHandle(), wParam, 0, ap.x + (ap.y << 16));
    if (App->GetMouseItem() && App->GetMouseItem() != App->GetRoot() &&
        App->GetMouseItem()->GetType() != "clickthroughbutton") {
      return 1;
    }
  }

  if (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP ||
      wParam == WM_MOUSEMOVE) {
    PostMessage(App->GetHandle(), wParam, 0, ap.x + (ap.y << 16));
    // let these through so we don't mess up dragging etc
    return CallNextHookEx(nullptr, code, wParam, lParam);
  }

  return CallNextHookEx(nullptr, code, wParam, lParam);
}

DWORD GetProcessIntegrityLevel(HANDLE hProcess) {
  HANDLE hToken = nullptr;

  DWORD dwLengthNeeded = 0;
  DWORD dwError = ERROR_SUCCESS;

  PTOKEN_MANDATORY_LABEL pTIL = nullptr;
  DWORD dwIntegrityLevel = 0;

  if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
    // Get the Integrity level.
    if (!GetTokenInformation(hToken, TokenIntegrityLevel, nullptr, 0,
                             &dwLengthNeeded)) {
      dwError = GetLastError();
      if (dwError == ERROR_INSUFFICIENT_BUFFER) {
        pTIL =
            static_cast<PTOKEN_MANDATORY_LABEL>(LocalAlloc(0, dwLengthNeeded));
        if (pTIL != nullptr) {
          if (GetTokenInformation(hToken, TokenIntegrityLevel, pTIL,
                                  dwLengthNeeded, &dwLengthNeeded)) {
            dwIntegrityLevel = *GetSidSubAuthority(
                pTIL->Label.Sid,
                static_cast<DWORD>(static_cast<UCHAR>(
                    *GetSidSubAuthorityCount(pTIL->Label.Sid) - 1)));
          }
          LocalFree(pTIL);
        }
      }
    } else {
      return -1;
    }
    CloseHandle(hToken);
  } else {
    return -1;
  }

  return dwIntegrityLevel;
}

bool IsProcessRunning(DWORD pid) {
  bool procRunning = false;

  HANDLE hProcessSnap = nullptr;
  PROCESSENTRY32 pe32 = {};
  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if (hProcessSnap == INVALID_HANDLE_VALUE) {
    return false;
  }

  pe32.dwSize = sizeof(PROCESSENTRY32);
  if (Process32First(hProcessSnap, &pe32)) {
    if (pe32.th32ProcessID == pid) {
      CloseHandle(hProcessSnap);
      return true;
    }

    while (Process32Next(hProcessSnap, &pe32)) {
      if (pe32.th32ProcessID == pid) {
        CloseHandle(hProcessSnap);
        return true;
      }
    }
    CloseHandle(hProcessSnap);
  }

  return procRunning;
}

std::string FetchHTTPS(std::string_view url, std::string_view path) {
  auto wurl = string2wstring(url);
  auto wpath = string2wstring(path);

  Log_Nfo("[GW2TacO] Fetching URL: {:s}/{:s}", url, path);

  DWORD dwSize = 0;
  DWORD dwDownloaded = 0;

  BOOL bResults = FALSE;
  HINTERNET hSession = nullptr, hConnect = nullptr, hRequest = nullptr;

  hSession =
      WinHttpOpen(L"WinHTTPS Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                  WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

  if (hSession) {
    hConnect = WinHttpConnect(hSession, wurl.c_str(), INTERNET_DEFAULT_PORT, 0);
  }

  if (hConnect) {
    hRequest =
        WinHttpOpenRequest(hConnect, L"GET", wpath.c_str(), nullptr,
                           WINHTTP_NO_REFERER, nullptr, WINHTTP_FLAG_SECURE);
  }

  if (hRequest) {
    bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                  WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
  }

  if (bResults) {
    bResults = WinHttpReceiveResponse(hRequest, nullptr);
  }

  if (!bResults) {
    return "";
  }

  CStreamWriterMemory data;

  do {
    dwSize = 0;
    if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
      return "";
    }

    auto pszOutBuffer = std::make_unique<char[]>(size_t(dwSize) + 1);

    if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer.get(), dwSize,
                         &dwDownloaded)) {
      return "";
    }

    data.Write(std::string_view(pszOutBuffer.get(), dwSize));

  } while (dwSize > 0);

  if (hRequest) {
    WinHttpCloseHandle(hRequest);
  }
  if (hConnect) {
    WinHttpCloseHandle(hConnect);
  }
  if (hSession) {
    WinHttpCloseHandle(hSession);
  }

  return std::string(reinterpret_cast<const char*>(data.GetData()),
                     data.GetLength());
}

#include <Urlmon.h>  // URLOpenBlockingStreamW()
#pragma comment(lib, "Urlmon.lib")

bool HooksInitialized = false;

volatile int mainLoopCounter = 0;
volatile int lastCnt = 0;
int lastMainLoopTime = 0;
#include <ShellScalingAPI.h>

#include <thread>

#include "src/util/jsonxx.h"

using namespace jsonxx;

#include <tlhelp32.h>

#include "wvw.h"

void GetFileName(CHAR pfname[MAX_PATH]) {
  DWORD dwOwnPID = GetProcessId(GetCurrentProcess());

  HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  auto processInfo = std::make_unique<PROCESSENTRY32>();
  processInfo->dwSize = sizeof(PROCESSENTRY32);
  while (Process32Next(hSnapShot, processInfo.get()) != FALSE) {
    if (processInfo->th32ProcessID == dwOwnPID) {
      memcpy(pfname, processInfo->szExeFile, MAX_PATH);
      break;
    }
  }
  CloseHandle(hSnapShot);
}

BOOL AppIsAllreadyRunning() {
  CHAR pfname[MAX_PATH];
  memset(pfname, 0, MAX_PATH);
  GetFileName(pfname);

  BOOL bRunning = FALSE;

  DWORD dwOwnPID = GetProcessId(GetCurrentProcess());

  HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  auto processInfo = std::make_unique<PROCESSENTRY32>();
  processInfo->dwSize = sizeof(PROCESSENTRY32);
  while (Process32Next(hSnapShot, processInfo.get()) != FALSE) {
    if (!strcmp(processInfo->szExeFile, pfname)) {
      if (processInfo->th32ProcessID != dwOwnPID) {
        bRunning = TRUE;
        break;
      }
    }
  }
  CloseHandle(hSnapShot);
  return bRunning;
}

bool keyboardHookActive = false;
bool mouseHookActive = false;

bool SetupTacoProtocolHandling() {
  TCHAR szFileName[MAX_PATH + 1];
  GetModuleFileName(nullptr, szFileName, MAX_PATH + 1);

  HKEY key = nullptr;

  if (RegCreateKeyEx(HKEY_CLASSES_ROOT, "gw2taco", 0, nullptr,
                     REG_OPTION_NON_VOLATILE, 0, nullptr, &key,
                     nullptr) != ERROR_SUCCESS) {
    if (RegOpenKey(HKEY_CLASSES_ROOT, "gw2taco", &key) != ERROR_SUCCESS) {
      return false;
    }
  } else {
    RegCloseKey(key);
    if (RegOpenKey(HKEY_CLASSES_ROOT, "gw2taco", &key) != ERROR_SUCCESS) {
      return false;
    }
  }

  const char* urldesc = "URL:gw2taco protocol";

  if (RegSetKeyValue(key, nullptr, nullptr, REG_SZ, urldesc, strlen(urldesc)) !=
      ERROR_SUCCESS) {
    return false;
  }

  if (RegSetKeyValue(key, nullptr, "URL Protocol", REG_SZ, nullptr, 0)) {
    return false;
  }

  if (RegSetKeyValue(key, "DefaultIcon", nullptr, REG_SZ, szFileName,
                     strlen(szFileName)) != ERROR_SUCCESS) {
    return false;
  }

  auto openMask = std::string("\"") + szFileName + "\" -fromurl %1";

  if (RegSetKeyValue(key, "shell\\open\\command", nullptr, REG_SZ,
                     openMask.c_str(), openMask.size()) != ERROR_SUCCESS) {
    return false;
  }

  RegCloseKey(key);
  return true;
}

bool DownloadFile(std::string_view url, CStreamWriterMemory& mem) {
  LPSTREAM stream = nullptr;

  HRESULT hr = URLOpenBlockingStream(
      nullptr, ("https://" + std::string(url)).c_str(), &stream, 0, nullptr);
  if (FAILED(hr)) {
    hr = URLOpenBlockingStream(nullptr, ("http://" + std::string(url)).c_str(),
                               &stream, 0, nullptr);
  }

  if (FAILED(hr)) {
    return false;
  }

  char buffer[4096];
  do {
    DWORD bytesRead = 0;
    hr = stream->Read(buffer, sizeof(buffer), &bytesRead);
    mem.Write(std::string_view(buffer, bytesRead));
  } while (SUCCEEDED(hr) && hr != S_FALSE);

  stream->Release();

  if (FAILED(hr)) {
    return false;
  }

  return true;
}

void ImportMarkerPack(CWBApplication* App, std::string_view zipFile);

void FlushZipDict();

uint32_t lastSlowEventTime = 0;
int gw2WindowCount = 0;

BOOL __stdcall gw2WindowCountFunc(HWND hwnd, LPARAM lParam) {
  TCHAR name[400];
  memset(name, 0, 400);
  GetWindowText(hwnd, name, 199);
  if (!strcmp(name, "Guild Wars 2")) {
    memset(name, 0, 400);
    GetClassName(hwnd, name, 199);
    if (!strcmp(name, "ArenaNet_Dx_Window_Class") ||
        !strcmp(name, "ArenaNet_Gr_Window_Class")) {
      gw2WindowCount++;
    }
  }
  return true;
}

BOOL __stdcall gw2WindowFromPIDFunction(HWND hWnd, LPARAM a2) {
  DWORD dwProcessId = 0;  // [esp+4h] [ebp-198h]
  CHAR ClassName[400];    // [esp+8h] [ebp-194h]

  memset(&ClassName, 0, 400);
  GetClassNameA(hWnd, ClassName, 199);
  if (!strcmp(ClassName, "ArenaNet_Dx_Window_Class") ||
      !strcmp(ClassName, "ArenaNet_Gr_Window_Class")) {
    dwProcessId = 0;
    GetWindowThreadProcessId(hWnd, &dwProcessId);
    if (a2 == dwProcessId) {
      gw2WindowFromPid = hWnd;
    }
  }
  return 1;
}

INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR lpCmdLine, _In_ INT nCmdShow) {
  ImmDisableIME(-1);
  lastSlowEventTime = globalTimer.GetTime();

  Logger.AddOutput(std::make_unique<CLoggerOutput_File>("GW2TacO.log"));
  Logger.SetVerbosity(LOGVERBOSITY::LOG_DEBUG);

  Logger.Log(LOGVERBOSITY::LOG_INFO, false, false, "");
  Logger.Log(LOGVERBOSITY::LOG_INFO, false, false,
             "----------------------------------------------");
  std::string cmdLine(GetCommandLineA());
  Log_Nfo("[GW2TacO] CommandLine: {:s}", cmdLine);

  if (cmdLine.find("-fromurl") != cmdLine.npos) {
    TCHAR szFileName[MAX_PATH + 1];
    GetModuleFileName(nullptr, szFileName, MAX_PATH + 1);
    std::string s(szFileName);
    for (int32_t x = s.size() - 1; x >= 0; x--) {
      if (s[x] == '\\' || s[x] == '/') {
        s[x] = 0;
        break;
      }
    }
    SetCurrentDirectory(s.c_str());

    auto TacoWindow = FindWindow("CoRE2", "Guild Wars 2 Tactical Overlay");
    Log_Nfo("[GW2TacO] TacO window id: {:d}",
            reinterpret_cast<int>(TacoWindow));
    if (TacoWindow) {
      COPYDATASTRUCT MyCDS = {};
      MyCDS.dwData = 0;
      MyCDS.cbData = cmdLine.size();
      MyCDS.lpData = (PVOID)cmdLine.c_str();

      SendMessage(TacoWindow, WM_COPYDATA, (WPARAM)(HWND) nullptr,
                  (LPARAM)(LPVOID)&MyCDS);

      Log_Nfo("[GW2TacO] WM_COPYDATA sent. Result code: {:d}", GetLastError());
      return 0;
    }
  }

  if (cmdLine.find("-forcenewinstance") == cmdLine.npos) {
    if (AppIsAllreadyRunning()) {
      return 0;
    }
  }

  auto mumblePos = cmdLine.find("-mumble");
  if (mumblePos != cmdLine.npos) {
    auto sub = cmdLine.substr(mumblePos);
    auto cmds = SplitByWhitespace(sub);
    if (cmds.size() > 1) {
      mumbleLink.mumblePath = cmds[1];
    }
  }

  if (!SetupTacoProtocolHandling()) {
    Log_Err("[GW2TacO] Failed to register gw2taco:// protocol with windows.");
  }

  typedef HRESULT(WINAPI *
                  SetProcessDpiAwareness)(_In_ PROCESS_DPI_AWARENESS value);
  typedef BOOL (*SetProcessDPIAwareFunc)();

  LoadConfig();
  InitConfig();

  if (cmdLine.find("-forcedpiaware") != cmdLine.npos ||
      (HasConfigValue("ForceDPIAware") && GetConfigValue("ForceDPIAware"))) {
    bool dpiSet = false;

    HMODULE hShCore = LoadLibrary("Shcore.dll");
    if (hShCore) {
      auto setDPIAwareness = reinterpret_cast<SetProcessDpiAwareness>(
          GetProcAddress(hShCore, "SetProcessDpiAwareness"));
      if (setDPIAwareness) {
        setDPIAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
        dpiSet = true;
        Log_Nfo("[GW2TacO] DPI Awareness set through SetProcessDpiAwareness");
      }
      FreeLibrary(hShCore);
    }

    if (!dpiSet) {
      HMODULE hUser32 = LoadLibrary("user32.dll");
      auto setDPIAware = reinterpret_cast<SetProcessDPIAwareFunc>(
          GetProcAddress(hUser32, "SetProcessDPIAware"));
      if (setDPIAware) {
        setDPIAware();
        Log_Nfo("[GW2TacO] DPI Awareness set through SetProcessDpiAware");
        dpiSet = true;
      }
      FreeLibrary(hUser32);
    }

    if (!dpiSet) {
      Log_Err("[GW2TacO] DPI Awareness NOT set");
    }
  }

  localization = std::make_unique<Localization>();
  localization->Import();

  Log_Nfo("[GW2TacO] build ID: {:s}", ("GW2 TacO " + TacOBuild));

  bool hasDComp = false;
  HMODULE dComp = LoadLibraryA("dcomp.dll");
  if (dComp) {
    hasDComp = true;
    FreeLibrary(dComp);
  }

  App = std::make_unique<COverlayApp>();

  int32_t width = 1;
  int32_t height = 1;

  CCoreWindowParameters p(GetModuleHandle(nullptr), false, width, height,
                          "Guild Wars 2 Tactical Overlay",
                          LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2)));
  p.OverrideWindowStyle = WS_POPUP;
  p.OverrideWindowStyleEx =
      WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TRANSPARENT |
      WS_EX_TOOLWINDOW;  // | WS_EX_TOPMOST;// | WS_EX_TOOLWINDOW;
  if (dComp) {
    p.OverrideWindowStyleEx = WS_EX_TOPMOST | WS_EX_TRANSPARENT |
                              WS_EX_TOOLWINDOW | WS_EX_LAYERED |
                              WS_EX_NOREDIRECTIONBITMAP;
  }

  if (!App->Initialize(p)) {
    MessageBox(
        nullptr,
        "Failed to initialize GW2 TacO!\nIf you want this error fixed,\nplease "
        "report the generated GW2TacO.log and a dxdiag log at "
        "https://github.com/therjak/GW2TacO",
        "Fail.", MB_ICONERROR);
    App.reset();
    return false;
  }

  if (!ChangeWindowMessageFilterEx(App->GetHandle(), WM_COPYDATA, MSGFLT_ALLOW,
                                   nullptr)) {
    Log_Err(
        "[GW2TacO] Failed to change message filters for WM_COPYDATA - "
        "gw2taco:// protocol messages will NOT be processed!");
  }

  DWORD hookThreadID = 0;

  App->SetScreenshotName("GW2TacO");
  App->SetClearColor(CColor(0, 0, 0, 0));

  ImportPOIS(App.get());

  mumbleLink.Update();

  if (!InitGUI(App.get())) {
    Log_Err("[GW2TacO] Missing file during init, exiting!");
    return -1;
  }

  extern WBATLASHANDLE DefaultIconHandle;
  if (DefaultIconHandle == -1) {
    auto skinItem = App->GetSkin()->GetElementID("defaulticon");
    DefaultIconHandle = App->GetSkin()->GetElement(skinItem)->GetHandle();
  }

  ImportPOIActivationData();
  ImportLocationalTimers();

  LoadWvWObjectives();

  OpenWindows(App.get());

  HWND handle = App->GetHandle();

  SetConfigValue("LogTrails", 0);

  SetLayeredWindowAttributes(handle, 0, 255, LWA_ALPHA);

  ShowWindow(handle, nCmdShow);

  bool FoundGW2Window = false;

  App->SetVSync(GetConfigValue("Vsync"));
  CRect pos;

  DWORD GW2Pid = 0;

  auto* taco = dynamic_cast<GW2TacO*>(
      App->GetRoot()->FindChildByID("tacoroot", "GW2TacO"));

  if (!taco) {
    return 0;
  }

  taco->InitScriptEngines();

  auto lastRenderTime = globalTimer.GetTime();

  bool frameThrottling = GetConfigValue("FrameThrottling") != 0;

  int32_t hideOnLoadingScreens = GetConfigValue("HideOnLoadingScreens");

  bool hadRetrace = false;

  while (App->HandleMessages()) {
#ifdef _DEBUG
    if (GetAsyncKeyState(VK_F11)) {
      break;
    }
#endif

    if (globalTimer.GetTime() - lastSlowEventTime > 1000) {
      if (GetConfigValue("CloseWithGW2")) {
        if (!gw2Window && FoundGW2Window) {
          if (!IsProcessRunning(GW2Pid)) {
            App->SetDone(true);
          }
        }
      }
    }

    for (int x = 0; x < (frameThrottling ? 32 : 1); x++) {
      if (mumbleLink.Update()) {
        break;
      }
      if (frameThrottling) {
        Sleep(1);
      }
    }

    bool shortTick = (GetTime() - mumbleLink.LastFrameTime) < 333;

    if (!hideOnLoadingScreens) {
      shortTick = true;
    }

    if (!FoundGW2Window) {
      // if (mumbleLink.mumblePath != "MumbleLink")
      {
        if (!mumbleLink.IsValid() && GetTime() > 60000) {
          Log_Err(
              "[GW2TacO] Closing TacO because GW2 with mumble link '{:s}' was "
              "not found in under a minute",
              mumbleLink.mumblePath);
          App->SetDone(true);
        }
      }
    }

    if (App->DeviceOK()) {
      extern bool frameTriggered;

      auto currTime = globalTimer.GetTime();

      if (currTime - lastSlowEventTime > 1000) {
        hideOnLoadingScreens = GetConfigValue("HideOnLoadingScreens");
        lastSlowEventTime = globalTimer.GetTime();
        gw2WindowCount = 0;
        gw2WindowFromPid = nullptr;
        EnumWindows(gw2WindowFromPIDFunction, mumbleLink.lastGW2ProcessID);
        gw2Window = gw2WindowFromPid;

        if (!gw2Window) {
          gw2Window = FindWindow("ArenaNet_Dx_Window_Class", nullptr);
        }
        if (!gw2Window) {
          gw2Window = FindWindow("ArenaNet_Gr_Window_Class", nullptr);
        }
      }

      if (!mumbleLink.IsValid() || !gw2Window) {
        Sleep(1000);
        continue;
      }

      /* if (!frameThrottling || frameTriggered ||
          lastRenderTime + 200 < currTime) */
      {
        if (gw2Window) {
          if (!FoundGW2Window) {
            GetWindowThreadProcessId(gw2Window, &GW2Pid);

            DWORD currentProcessIntegrity =
                GetProcessIntegrityLevel(GetCurrentProcess());
            DWORD gw2ProcessIntegrity = GetProcessIntegrityLevel(
                OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, GW2Pid));

            Log_Dbg("[GW2TacO] Taco integrity: {:x}, GW2 integrity: {:x}",
                    currentProcessIntegrity, gw2ProcessIntegrity);

            if (gw2ProcessIntegrity > currentProcessIntegrity ||
                gw2ProcessIntegrity == -1) {
              MessageBox(nullptr,
                         "GW2 seems to have more elevated rights than GW2 "
                         "TacO.\nThis will probably result in TacO not being "
                         "interactive when GW2 is in focus.\nIf this is an "
                         "issue for you, restart TacO in Administrator mode.",
                         "Warning", MB_ICONWARNING);
            }
          }
          FoundGW2Window = true;

          RECT GW2ClientRect;
          POINT p = {0, 0};
          GetClientRect(gw2Window, &GW2ClientRect);
          ClientToScreen(gw2Window, &p);

          if (GW2ClientRect.right - GW2ClientRect.left != pos.Width() ||
              GW2ClientRect.bottom - GW2ClientRect.top != pos.Height() ||
              p.x != pos.x1 || p.y != pos.y1) {
            Log_Nfo(
                "[GW2TacO] gw2 window size change: {:d} {:d} {:d} {:d} ({:d} "
                "{:d})",
                GW2ClientRect.left, GW2ClientRect.top, GW2ClientRect.right,
                GW2ClientRect.bottom, GW2ClientRect.right - GW2ClientRect.left,
                GW2ClientRect.bottom - GW2ClientRect.top);
            bool NeedsResize =
                GW2ClientRect.right - GW2ClientRect.left != pos.Width() ||
                GW2ClientRect.bottom - GW2ClientRect.top != pos.Height();
            pos = CRect(GW2ClientRect.left + p.x, GW2ClientRect.top + p.y,
                        GW2ClientRect.left + p.x + GW2ClientRect.right -
                            GW2ClientRect.left,
                        GW2ClientRect.top + p.y + GW2ClientRect.bottom -
                            GW2ClientRect.top);

            ::SetWindowPos(handle, nullptr, pos.x1, pos.y1, pos.Width(),
                           pos.Height(), SWP_NOREPOSITION);

            if (NeedsResize) {
              App->HandleResize();
              MARGINS marg = {-1, -1, -1, -1};
              DwmExtendFrameIntoClientArea(handle, &marg);
            }
          }

          auto foregroundWindow = GetForegroundWindow();

          if (foregroundWindow == gw2Window && App->GetFocusItem() &&
              App->GetFocusItem()->InstanceOf("textbox")) {
            SetForegroundWindow(App->GetHandle());
            SetFocus(App->GetHandle());
            ::SetWindowPos(App->GetHandle(), HWND_TOPMOST, 0, 0, 0, 0,
                           SWP_NOMOVE | SWP_NOSIZE);
          }

          if (foregroundWindow == App->GetHandle() &&
              !(App->GetFocusItem() &&
                App->GetFocusItem()->InstanceOf("textbox"))) {
            SetForegroundWindow(gw2Window);
            SetFocus(gw2Window);
          }

          bool EditedButNotSelected =
              (foregroundWindow != gw2Window &&
               foregroundWindow != App->GetHandle() && App->GetFocusItem() &&
               App->GetFocusItem()->InstanceOf("textbox"));
          if (EditedButNotSelected) {
            App->GetRoot()->SetFocus();
          }

          if (gw2Window && (!((App->GetFocusItem() &&
                               App->GetFocusItem()->InstanceOf("textbox")) ||
                              EditedButNotSelected))) {
            HWND wnd = ::GetNextWindow(gw2Window, GW_HWNDPREV);
            if (wnd != handle) {
              if (wnd) {
                ::SetWindowPos(handle, wnd, 0, 0, 0, 0,
                               SWP_NOMOVE | SWP_NOSIZE);
              } else {
                ::SetWindowPos(handle, HWND_TOPMOST, 0, 0, 0, 0,
                               SWP_NOMOVE | SWP_NOSIZE);
              }
            }

            App->GetRoot()->Hide(
                !shortTick ||
                ((GW2ClientRect.right - GW2ClientRect.left == 1120) &&
                 (GW2ClientRect.bottom - GW2ClientRect.top == 976) &&
                 (p.x != 0 || p.y != 0)));
          }
        } else {
          App->GetRoot()->Hide(!shortTick);
        }

        taco->TickScriptEngine();
        AutoSaveConfig();
        App->Display();
        // App->GetDevice()->WaitRetrace();
        frameTriggered = false;
        lastRenderTime = currTime;
        hadRetrace = false;
      }

      if (!HooksInitialized) {
        // enabling this blocks mouse input while the debugger is present
        // if (!IsDebuggerPresent()) {
        {
          auto hookThread = CreateThread(
              nullptr,  // default security attributes
              0,        // use default stack size
              [](LPVOID data) {
                auto keyboardHook =
                    SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, nullptr, 0);
                auto mouseHook =
                    SetWindowsHookEx(WH_MOUSE_LL, MouseHook, nullptr, 0);
                MSG msg;

                keyboardHookActive = true;
                mouseHookActive = true;

                while (GetMessage(&msg, nullptr, 0, 0) > 0) {
                  if (msg.message == WM_QUIT) {
                    break;
                  }
                  TranslateMessage(&msg);
                }
                DispatchMessage(&msg);

                UnhookWindowsHookEx(keyboardHook);
                UnhookWindowsHookEx(mouseHook);

                keyboardHookActive = false;
                mouseHookActive = false;

                return static_cast<DWORD>(0);
              },
              nullptr,  // argument to thread function
              0,        // use default creation flags
              &hookThreadID);
        }
        HooksInitialized = true;
      }
    } else {
      Log_Err("[GW2TacO] Device fail");
    }

    mainLoopCounter++;
    lastMainLoopTime = GetTime();
  }

  FlushZipDict();

  if (hookThreadID) {
    PostThreadMessage(hookThreadID, WM_QUIT, 0, 0);
  }

  ShowWindow(handle, SW_HIDE);

  SaveConfig();

  while (keyboardHookActive) {
    ;
  }
  while (mouseHookActive) {
    ;
  }

  extern std::thread wvwPollThread;
  extern std::thread wvwUpdatThread;

  if (wvwPollThread.joinable()) {
    wvwPollThread.join();
    Sleep(1000);
  }
  if (wvwUpdatThread.joinable()) {
    wvwUpdatThread.join();
  }

  trails.clear();

  // cleanup
  App.reset();

  localization.reset();

  return true;
}
