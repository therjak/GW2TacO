#include "src/core2/window_handler.h"

#include "src/base/logger.h"
#include "src/base/rectangle.h"
#include "src/base/timer.h"
#include "src/core2/dx11_device.h"

using math::CPoint;
using math::CRect;

typedef CCoreDX11Device CCore;

//////////////////////////////////////////////////////////////////////////
// window init parameter structure

CCoreWindowParameters::CCoreWindowParameters() = default;

CCoreWindowParameters::CCoreWindowParameters(HINSTANCE hinst, bool fs,
                                             int32_t x, int32_t y,
                                             const TCHAR* title, HICON icon,
                                             bool maximize, bool noresize)
    : hInstance(hinst),
      FullScreen(fs),
      XRes(x),
      YRes(y),
      WindowTitle(title),
      Icon(icon),
      Maximized(maximize),
      ResizeDisabled(noresize) {}

std::unique_ptr<CCoreDevice> CCoreWindowParameters::CreateDevice() const {
  return std::make_unique<CCore>();
}

//////////////////////////////////////////////////////////////////////////
// windowhandler baseclass

CCoreWindowHandler::CCoreWindowHandler() {
  LastRenderedFrame = globalTimer.GetTime();
}

CCoreWindowHandler::~CCoreWindowHandler() { Destroy(); }

void CCoreWindowHandler::Destroy() { Done = true; }

int32_t CCoreWindowHandler::GetXRes() { return XRes; }

int32_t CCoreWindowHandler::GetYRes() { return YRes; }

CCoreWindowParameters& CCoreWindowHandler::GetInitParameters() {
  return InitParameters;
}

void CCoreWindowHandler::SelectMouseCursor(COREMOUSECURSOR m) {
  CurrentMouseCursor = m;
}

CPoint CCoreWindowHandler::GetMousePos() { return MousePos; }

CPoint CCoreWindowHandler::GetLeftDownPos() { return LeftDownPos; }

CPoint CCoreWindowHandler::GetRightDownPos() { return RightDownPos; }

CPoint CCoreWindowHandler::GetMidDownPos() { return MidDownPos; }

void CCoreWindowHandler::SetInactiveFrameLimiter(bool set) {
  InactiveFrameLimiter = set;
}

//////////////////////////////////////////////////////////////////////////
// windows windowhandler

CCoreWindowHandlerWin::CCoreWindowHandlerWin() : CCoreWindowHandler() {
  WindowPlacement.length = sizeof(WINDOWPLACEMENT);
}

CCoreWindowHandlerWin::~CCoreWindowHandlerWin() {
  for (auto m : MouseCursors) {
    DeleteObject(m);
  }
}

bool CCoreWindowHandlerWin::Initialize(const CCoreWindowParameters& wp) {
  XRes = wp.XRes;
  YRes = wp.YRes;
  InitParameters = wp;

  WNDCLASS wc;
  memset(&wc, 0, sizeof(wc));
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
  wc.lpfnWndProc = WndProcProxy;
  wc.hInstance = wp.hInstance;
  wc.hIcon = wp.Icon;
  wc.lpszClassName = "CoRE2";
  RegisterClass(&wc);

  RECT WindowRect;
  WindowRect.left = 0;
  WindowRect.right = XRes;
  WindowRect.top = 0;
  WindowRect.bottom = YRes;

  if (!wp.FullScreen) {
    dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_OVERLAPPED |
              WS_MINIMIZEBOX |
              ((WS_MAXIMIZEBOX | WS_SIZEBOX) * (!wp.ResizeDisabled)) |
              (WS_MAXIMIZE * wp.Maximized);
    FullScreenX = GetSystemMetrics(SM_CXSCREEN);
    FullScreenY = GetSystemMetrics(SM_CYSCREEN);
  } else {
    dwStyle = WS_POPUP | WS_OVERLAPPED;
    FullScreenX = XRes;
    FullScreenY = YRes;
  }

  if (!wp.OverrideWindowStyleEx) {
    dwStyle = dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    if (wp.OverrideWindowStyle) dwStyle = wp.OverrideWindowStyle;
    AdjustWindowRect(&WindowRect, dwStyle, FALSE);
    hWnd = CreateWindow("CoRE2", wp.WindowTitle, dwStyle, CW_USEDEFAULT,
                        CW_USEDEFAULT, WindowRect.right - WindowRect.left,
                        WindowRect.bottom - WindowRect.top, nullptr, nullptr,
                        wp.hInstance, this);
  } else {
    dwStyle = wp.OverrideWindowStyle;
    AdjustWindowRect(&WindowRect, dwStyle, FALSE);
    hWnd = CreateWindowEx(wp.OverrideWindowStyleEx, "CoRE2", wp.WindowTitle,
                          dwStyle, CW_USEDEFAULT, CW_USEDEFAULT,
                          WindowRect.right - WindowRect.left,
                          WindowRect.bottom - WindowRect.top, nullptr, nullptr,
                          wp.hInstance, this);
  }

  Device = wp.CreateDevice();

  if (!Device) {
    Log_Err("[init] Device object is NULL during init.");
    return false;
  }

  if (!Device->Initialize(this)) {
    Device.reset();
    return false;
  }

  ShowWindow(hWnd, Maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
  SetForegroundWindow(hWnd);
  SetFocus(hWnd);

  MouseCursorsAt(COREMOUSECURSOR::CM_ARROW) = (LoadCursor(nullptr, IDC_ARROW));
  MouseCursorsAt(COREMOUSECURSOR::CM_CROSS) = (LoadCursor(nullptr, IDC_CROSS));
  MouseCursorsAt(COREMOUSECURSOR::CM_SIZEWE) =
      (LoadCursor(nullptr, IDC_SIZEWE));
  MouseCursorsAt(COREMOUSECURSOR::CM_SIZENS) =
      (LoadCursor(nullptr, IDC_SIZENS));
  MouseCursorsAt(COREMOUSECURSOR::CM_SIZENESW) =
      (LoadCursor(nullptr, IDC_SIZENESW));
  MouseCursorsAt(COREMOUSECURSOR::CM_SIZENWSE) =
      (LoadCursor(nullptr, IDC_SIZENWSE));
  MouseCursorsAt(COREMOUSECURSOR::CM_TEXT) = (LoadCursor(nullptr, IDC_IBEAM));
  MouseCursorsAt(COREMOUSECURSOR::CM_WAIT) = (LoadCursor(nullptr, IDC_WAIT));

  Maximized = wp.Maximized;
  Minimized = false;
  Active = true;

  RECT r;
  GetClientRect(hWnd, &r);
  ClientRect = CRect(r.left, r.top, r.right, r.bottom);

  if (wp.Maximized) {
    RECT r2;
    GetClientRect(hWnd, &r2);

    XRes = r2.right - r2.left;
    YRes = r2.bottom - r2.top;
  }

  return true;
}

bool CCoreWindowHandlerWin::HandleMessages() { return HandleOSMessages(); }

bool CCoreWindowHandlerWin::HandleOSMessages() {
  MSG msg;
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return !Done;
}

bool CCoreWindowHandlerWin::DeviceOK() {
  if (!Active) {
    if (!InactiveFrameLimiter) return Device && Device->DeviceOk();

    const int32_t time = globalTimer.GetTime();
    if (time - LastRenderedFrame >= 1000 / LimitedFPS) {
      LastRenderedFrame = time;
      return true;
    }
    return false;
  }

  if (!Device) return false;
  return Device->DeviceOk();
}

void CCoreWindowHandlerWin::Destroy() {
  Done = true;
  if (hWnd) {
    DestroyWindow(hWnd);
    hWnd = nullptr;
  }
}

void CCoreWindowHandlerWin::ToggleFullScreen() {
  if (!Device) return;
  if (Device->IsWindowed()) {
    // go to fullscreen
    dwStyle = GetWindowLong(hWnd, GWL_STYLE);
    GetWindowPlacement(hWnd, &WindowPlacement);
    ShowWindow(hWnd, SW_HIDE);
    SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    Device->SetFullScreenMode(true, FullScreenX, FullScreenY);
    // Device->Resize(FullScreenX,FullScreenY,false);
    HandleResize();
    ShowWindow(hWnd, SW_SHOW);
  } else {
    // go to window mode
    SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, 0);
    SetWindowLongPtr(hWnd, GWL_STYLE, dwStyle);
    SetWindowPlacement(hWnd, &WindowPlacement);
    // Device->Resize(0,0,true);
    Device->SetFullScreenMode(false, 0, 0);
    ShowWindow(hWnd, SW_SHOW);
    HandleResize();
  }
}

void CCoreWindowHandlerWin::HandleAltEnter() {
  switch (Device->GetAPIType()) {
    case COREDEVICEAPI::DX9:
      ToggleFullScreen();
      break;
    case COREDEVICEAPI::DX11:
      // handled by dxgi <3
      break;
    case COREDEVICEAPI::OPENGL:
      break;
    default:
      break;
  }
}

LRESULT CALLBACK CCoreWindowHandlerWin::WndProcProxy(HWND hWnd, UINT uMsg,
                                                     WPARAM wParam,
                                                     LPARAM lParam) {
  CCoreWindowHandlerWin* wnd = nullptr;

  if (uMsg == WM_NCCREATE) {
    wnd = static_cast<CCoreWindowHandlerWin*>(
        ((LPCREATESTRUCT)lParam)->lpCreateParams);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)wnd);
    wnd->hWnd = hWnd;
  } else {
    wnd = (CCoreWindowHandlerWin*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  }

  if (wnd) {
    return wnd->WindowProc(uMsg, wParam, lParam);
  } else {
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }
}

LRESULT CCoreWindowHandlerWin::WindowProc(UINT uMsg, WPARAM wParam,
                                          LPARAM lParam) {
  if (!hWnd) return 0;

  switch (uMsg) {
    case WM_CLOSE:
      Done = true;
      return 0;
      break;
    case WM_ACTIVATE: {
      Active = wParam != WA_INACTIVE;
      break;
    }

    case WM_ENTERSIZEMOVE: {
      Active = false;
      break;
    }
    case WM_EXITSIZEMOVE: {
      Active = true;
      HandleResize();
    } break;
    case WM_SIZE: {
      if (Device && Device->IsWindowed() && hWnd) {
        dwStyle = GetWindowLong(hWnd, GWL_STYLE);
      }

      if (wParam == SIZE_MINIMIZED) {
        Active = false;
        Minimized = true;
        Maximized = false;
      }

      if (wParam == SIZE_MAXIMIZED) {
        Active = true;
        Minimized = false;
        Maximized = true;
        HandleResize();
      }

      if (wParam == SIZE_RESTORED) {
        if (Maximized) {
          Maximized = false;
          HandleResize();
        } else if (Minimized) {
          Active = true;
          Minimized = false;
          HandleResize();
        }

        // If we're neither maximized nor minimized, the window size
        // is changing by the user dragging the window edges.  In this
        // case, we don't reset the device yet -- we wait until the
        // user stops dragging, and a WM_EXITSIZEMOVE message comes.
      }

    } break;
    case WM_SYSKEYDOWN: {
      Log_Err("[wndproc] WM_SYSKEYDOWN {:d} {:d}", wParam, lParam);
      if (wParam == VK_F10) {
        return 0;  // if we dont do this, system menu opens up on F10 = bad for
      }
                   // fraps
      if (wParam == VK_MENU || wParam == VK_LMENU || wParam == VK_RMENU) {
        return 0;
      }
      if (wParam == VK_RETURN) {
        HandleAltEnter();
        break;
      }
      break;
    }
    case WM_SYSCOMMAND: {
      Log_Err("[wndproc] WM_SYSCOMMAND {:d} {:d}", wParam, lParam);
      switch (wParam) {
        case SC_SCREENSAVE:
        case SC_MONITORPOWER:
        case SC_KEYMENU:  // don't ding on alt+enter dammit
          return 0;
      }
      break;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
      SetCapture(hWnd);
      break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
      ReleaseCapture();
      break;
    default:
      break;
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void CCoreWindowHandlerWin::HandleResize() {
  const CRect old = ClientRect;

  RECT r;
  GetClientRect(hWnd, &r);
  ClientRect = CRect(r.left, r.top, r.right, r.bottom);
  XRes = ClientRect.Width();
  YRes = ClientRect.Height();

  if (Device && (old.Width() != XRes || old.Height() != YRes)) {
    Device->Resize(XRes, YRes);
  }
}

HWND CCoreWindowHandlerWin::GetHandle() { return hWnd; }

void CCoreWindowHandlerWin::FinalizeMouseCursor() {
  POINT ap;
  GetCursorPos(&ap);
  ScreenToClient(hWnd, &ap);
  CPoint mp = CPoint(ap.x, ap.y);
  RECT ClientRect;
  GetClientRect(hWnd, &ClientRect);

  if (CRect(0, 0, ClientRect.right, ClientRect.bottom).Contains(mp)) {
    SetCursor(MouseCursorsAt(CurrentMouseCursor));
  }
}

void CCoreWindowHandlerWin::SetWindowTitle(std::string_view Title) {
  SetWindowText(hWnd, Title.data());
}
