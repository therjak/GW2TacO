#include "src/core2/window_handler.h"

#include <windows.h>

#include <array>
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include "src/base/logger.h"
#include "src/base/timer.h"
#include "src/core2/dx11_device.h"

import math;

using math::Point;
using math::Rect;

namespace renderer {

//////////////////////////////////////////////////////////////////////////
// window init parameter structure

WindowParameters::WindowParameters() = default;

WindowParameters::WindowParameters(HINSTANCE h_instance, bool full_screen,
                                   int32_t x_res, int32_t y_res,
                                   const TCHAR* title, HICON icon,
                                   bool maximize, bool noresize)
    : h_instance_(h_instance),
      full_screen_(full_screen),
      x_res_(x_res),
      y_res_(y_res),
      window_title_(title),
      icon_(icon),
      maximized_(maximize),
      resize_disabled_(noresize) {}

std::unique_ptr<Device> WindowParameters::CreateDevice() const {
  return std::make_unique<DX11Device>();
}

//////////////////////////////////////////////////////////////////////////
// windowhandler baseclass

WindowHandler::WindowHandler() { last_rendered_frame_ = globalTimer.GetTime(); }

WindowHandler::~WindowHandler() { Destroy(); }

void WindowHandler::Destroy() { done_ = true; }

int32_t WindowHandler::GetXRes() { return x_res_; }

int32_t WindowHandler::GetYRes() { return y_res_; }

WindowParameters& WindowHandler::GetInitParameters() {
  return init_parameters_;
}

void WindowHandler::SelectMouseCursor(MouseCursor m) {
  current_mouse_cursor_ = m;
}

Point WindowHandler::GetMousePos() { return mouse_pos_; }

Point WindowHandler::GetLeftDownPos() { return left_down_pos_; }

Point WindowHandler::GetRightDownPos() { return right_down_pos_; }

Point WindowHandler::GetMidDownPos() { return mid_down_pos_; }

void WindowHandler::SetInactiveFrameLimiter(bool set) {
  inactive_frame_limiter_ = set;
}

//////////////////////////////////////////////////////////////////////////
// windows windowhandler

WindowHandlerWin::WindowHandlerWin() : WindowHandler() {
  window_placement_.length = sizeof(WINDOWPLACEMENT);
}

WindowHandlerWin::~WindowHandlerWin() {
  for (auto m : mouse_cursors_) {
    DeleteObject(m);
  }
}

bool WindowHandlerWin::Initialize(const WindowParameters& window_params) {
  x_res_ = window_params.x_res_;
  y_res_ = window_params.y_res_;
  init_parameters_ = window_params;

  WNDCLASS wc;
  memset(&wc, 0, sizeof(wc));
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
  wc.lpfnWndProc = WndProcProxy;
  wc.hInstance = window_params.h_instance_;
  wc.hIcon = window_params.icon_;
  wc.lpszClassName = "CoRE2";
  RegisterClass(&wc);

  RECT window_rect;
  window_rect.left = 0;
  window_rect.right = x_res_;
  window_rect.top = 0;
  window_rect.bottom = y_res_;

  if (!window_params.full_screen_) {
    dw_style_ =
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_OVERLAPPED |
        WS_MINIMIZEBOX |
        ((WS_MAXIMIZEBOX | WS_SIZEBOX) * (!window_params.resize_disabled_)) |
        (WS_MAXIMIZE * window_params.maximized_);
    full_screen_x_ = GetSystemMetrics(SM_CXSCREEN);
    full_screen_y_ = GetSystemMetrics(SM_CYSCREEN);
  } else {
    dw_style_ = WS_POPUP | WS_OVERLAPPED;
    full_screen_x_ = x_res_;
    full_screen_y_ = y_res_;
  }

  if (!window_params.override_window_style_ex_) {
    dw_style_ = dw_style_ | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    if (window_params.override_window_style_)
      dw_style_ = window_params.override_window_style_;
    AdjustWindowRect(&window_rect, dw_style_, FALSE);
    window_handle_ = CreateWindow("CoRE2", window_params.window_title_,
                                  dw_style_, CW_USEDEFAULT, CW_USEDEFAULT,
                                  window_rect.right - window_rect.left,
                                  window_rect.bottom - window_rect.top, nullptr,
                                  nullptr, window_params.h_instance_, this);
  } else {
    dw_style_ = window_params.override_window_style_;
    AdjustWindowRect(&window_rect, dw_style_, FALSE);
    window_handle_ =
        CreateWindowEx(window_params.override_window_style_ex_, "CoRE2",
                       window_params.window_title_, dw_style_, CW_USEDEFAULT,
                       CW_USEDEFAULT, window_rect.right - window_rect.left,
                       window_rect.bottom - window_rect.top, nullptr, nullptr,
                       window_params.h_instance_, this);
  }

  device_ = window_params.CreateDevice();

  if (!device_) {
    Log_Err("[init] Device object is NULL during init.");
    return false;
  }

  if (!device_->Initialize(this)) {
    device_.reset();
    return false;
  }

  ShowWindow(window_handle_, maximized_ ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
  SetForegroundWindow(window_handle_);
  SetFocus(window_handle_);

  MouseCursorsAt(MouseCursor::kArrow) = (LoadCursor(nullptr, IDC_ARROW));
  MouseCursorsAt(MouseCursor::kCross) = (LoadCursor(nullptr, IDC_CROSS));
  MouseCursorsAt(MouseCursor::kSizeWe) = (LoadCursor(nullptr, IDC_SIZEWE));
  MouseCursorsAt(MouseCursor::kSizeNs) = (LoadCursor(nullptr, IDC_SIZENS));
  MouseCursorsAt(MouseCursor::kSizeNeSw) = (LoadCursor(nullptr, IDC_SIZENESW));
  MouseCursorsAt(MouseCursor::kSizeNwSe) = (LoadCursor(nullptr, IDC_SIZENWSE));
  MouseCursorsAt(MouseCursor::kText) = (LoadCursor(nullptr, IDC_IBEAM));
  MouseCursorsAt(MouseCursor::kWait) = (LoadCursor(nullptr, IDC_WAIT));

  maximized_ = window_params.maximized_;
  minimized_ = false;
  active_ = true;

  RECT r;
  GetClientRect(window_handle_, &r);
  client_rect_ = Rect(r.left, r.top, r.right, r.bottom);

  if (window_params.maximized_) {
    RECT r2;
    GetClientRect(window_handle_, &r2);

    x_res_ = r2.right - r2.left;
    y_res_ = r2.bottom - r2.top;
  }

  return true;
}

bool WindowHandlerWin::HandleMessages() { return HandleOSMessages(); }

bool WindowHandlerWin::HandleOSMessages() {
  MSG msg;
  while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return !done_;
}

bool WindowHandlerWin::DeviceOK() {
  if (!active_) {
    if (!inactive_frame_limiter_) return device_ && device_->DeviceOk();

    const int32_t time = globalTimer.GetTime();
    if (time - last_rendered_frame_ >= 1000 / limited_fps_) {
      last_rendered_frame_ = time;
      return true;
    }
    return false;
  }

  if (!device_) return false;
  return device_->DeviceOk();
}

void WindowHandlerWin::Destroy() {
  done_ = true;
  if (window_handle_) {
    DestroyWindow(window_handle_);
    window_handle_ = nullptr;
  }
}

void WindowHandlerWin::ToggleFullScreen() {
  if (!device_) return;
  if (device_->IsWindowed()) {
    // go to fullscreen
    dw_style_ = GetWindowLong(window_handle_, GWL_STYLE);
    GetWindowPlacement(window_handle_, &window_placement_);
    ShowWindow(window_handle_, SW_HIDE);
    SetWindowLongPtr(window_handle_, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    device_->SetFullScreenMode(true, full_screen_x_, full_screen_y_);
    // Device->Resize(FullScreenX,FullScreenY,false);
    HandleResize();
    ShowWindow(window_handle_, SW_SHOW);
  } else {
    // go to window mode
    SetWindowPos(window_handle_, HWND_NOTOPMOST, 0, 0, 0, 0, 0);
    SetWindowLongPtr(window_handle_, GWL_STYLE, dw_style_);
    SetWindowPlacement(window_handle_, &window_placement_);
    // Device->Resize(0,0,true);
    device_->SetFullScreenMode(false, 0, 0);
    ShowWindow(window_handle_, SW_SHOW);
    HandleResize();
  }
}

void WindowHandlerWin::HandleAltEnter() {
  switch (device_->GetAPIType()) {
    case DeviceApi::kDx9:
      ToggleFullScreen();
      break;
    case DeviceApi::kDx11:
      // handled by dxgi <3
      break;
    case DeviceApi::kOpenGl:
      break;
    default:
      break;
  }
}

LRESULT CALLBACK WindowHandlerWin::WndProcProxy(HWND hWnd, UINT uMsg,
                                                WPARAM wParam, LPARAM lParam) {
  WindowHandlerWin* wnd = nullptr;

  if (uMsg == WM_NCCREATE) {
    wnd = static_cast<WindowHandlerWin*>(
        ((LPCREATESTRUCT)lParam)->lpCreateParams);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)wnd);
    wnd->window_handle_ = hWnd;
  } else {
    wnd = (WindowHandlerWin*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  }

  if (wnd) {
    return wnd->WindowProc(uMsg, wParam, lParam);
  } else {
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }
}

LRESULT WindowHandlerWin::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (!window_handle_) return 0;

  switch (uMsg) {
    case WM_CLOSE:
      done_ = true;
      return 0;
      break;
    case WM_ACTIVATE: {
      active_ = wParam != WA_INACTIVE;
      break;
    }

    case WM_ENTERSIZEMOVE: {
      active_ = false;
      break;
    }
    case WM_EXITSIZEMOVE: {
      active_ = true;
      HandleResize();
    } break;
    case WM_SIZE: {
      if (device_ && device_->IsWindowed() && window_handle_) {
        dw_style_ = GetWindowLong(window_handle_, GWL_STYLE);
      }

      if (wParam == SIZE_MINIMIZED) {
        active_ = false;
        minimized_ = true;
        maximized_ = false;
      }

      if (wParam == SIZE_MAXIMIZED) {
        active_ = true;
        minimized_ = false;
        maximized_ = true;
        HandleResize();
      }

      if (wParam == SIZE_RESTORED) {
        if (maximized_) {
          maximized_ = false;
          HandleResize();
        } else if (minimized_) {
          active_ = true;
          minimized_ = false;
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
        // if we dont do this, system menu opens up on F10 = bad for fraps
        return 0;
      }

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
        case SC_KEYMENU:  // don't ding on alt+enter
          return 0;
      }
      break;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
      SetCapture(window_handle_);
      break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
      ReleaseCapture();
      break;
    default:
      break;
  }

  return DefWindowProc(window_handle_, uMsg, wParam, lParam);
}

void WindowHandlerWin::HandleResize() {
  const Rect old = client_rect_;

  RECT r;
  GetClientRect(window_handle_, &r);
  client_rect_ = Rect(r.left, r.top, r.right, r.bottom);
  x_res_ = client_rect_.Width();
  y_res_ = client_rect_.Height();

  if (device_ && (old.Width() != x_res_ || old.Height() != y_res_)) {
    device_->Resize(x_res_, y_res_);
  }
}

HWND WindowHandlerWin::GetHandle() { return window_handle_; }

void WindowHandlerWin::FinalizeMouseCursor() {
  POINT point;
  GetCursorPos(&point);
  ScreenToClient(window_handle_, &point);
  Point mouse_pos = Point(point.x, point.y);
  RECT client_rect;
  GetClientRect(window_handle_, &client_rect);

  if (Rect(0, 0, client_rect.right, client_rect.bottom).Contains(mouse_pos)) {
    SetCursor(MouseCursorsAt(current_mouse_cursor_));
  }
}

void WindowHandlerWin::SetWindowTitle(std::string_view title) {
  SetWindowText(window_handle_, title.data());
}

}  // namespace renderer
