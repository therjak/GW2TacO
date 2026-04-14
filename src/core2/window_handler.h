#pragma once
#include <windows.h>

#include <array>
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

import math;

namespace renderer {

class CCoreDevice;

enum class CoreMouseCursor : uint16_t {
  kArrow,
  kCross,
  kSizeWe,
  kSizeNs,
  kSizeNeSw,
  kSizeNwSe,
  kText,
  kWait,
};

//////////////////////////////////////////////////////////////////////////
// window init parameter structure

class CCoreWindowParameters {
 public:
  CCoreWindowParameters();
  CCoreWindowParameters(HINSTANCE h_instance, bool full_screen, int32_t x_res,
                        int32_t y_res, const TCHAR* window_title,
                        HICON icon = nullptr, bool maximized = false,
                        bool resize_disabled = false);

  [[nodiscard]] std::unique_ptr<CCoreDevice> CreateDevice() const;

  HINSTANCE h_instance_ = nullptr;
  bool full_screen_ = false;
  int32_t x_res_ = 800;
  int32_t y_res_ = 600;
  const TCHAR* window_title_ = nullptr;
  HICON icon_ = nullptr;
  bool maximized_ = false;
  bool resize_disabled_ = false;

  DWORD override_window_style_ = 0;
  DWORD override_window_style_ex_ = 0;
};

//////////////////////////////////////////////////////////////////////////
// interface

class CCoreWindowHandler {
 public:
  CCoreWindowHandler();
  virtual ~CCoreWindowHandler();

  // this initializer will change to accommodate multiple platforms at once once
  // we get to that point:
  virtual bool Initialize(const CCoreWindowParameters& window_params) = 0;

  virtual void Destroy();
  virtual bool HandleMessages() = 0;
  virtual bool HandleOSMessages() = 0;
  virtual bool DeviceOK() = 0;
  virtual void ToggleFullScreen() = 0;

  virtual HWND GetHandle() = 0;

  virtual int32_t GetXRes();
  virtual int32_t GetYRes();
  virtual CCoreWindowParameters& GetInitParameters();

  virtual void SelectMouseCursor(CoreMouseCursor cursor);
  virtual void FinalizeMouseCursor() = 0;
  math::CPoint GetMousePos();
  math::CPoint GetLeftDownPos();
  math::CPoint GetRightDownPos();
  math::CPoint GetMidDownPos();

  CCoreDevice* GetDevice() { return device_.get(); }

  virtual void SetWindowTitle(std::string_view title) = 0;
  virtual void SetInactiveFrameLimiter(bool set);

 protected:
  virtual void HandleResize() = 0;
  virtual void HandleAltEnter() = 0;

  bool done_ = false;
  std::unique_ptr<CCoreDevice> device_;
  bool active_ = false;
  bool maximized_ = false;
  bool minimized_ = false;
  math::CRect client_rect_;

  bool inactive_frame_limiter_ = true;
  int32_t limited_fps_ = 20;
  int32_t last_rendered_frame_ = 0;

  int32_t x_res_ = 0, y_res_ = 0;

  CCoreWindowParameters init_parameters_;

  CoreMouseCursor current_mouse_cursor_ = CoreMouseCursor::kArrow;

  math::CPoint mouse_pos_, left_down_pos_, right_down_pos_, mid_down_pos_;
};

//////////////////////////////////////////////////////////////////////////
// windows implementation

class CCoreWindowHandlerWin : public CCoreWindowHandler {
 public:
  CCoreWindowHandlerWin();
  ~CCoreWindowHandlerWin() override;

  bool Initialize(const CCoreWindowParameters& window_params) override;
  void Destroy() override;
  bool HandleMessages() override;
  bool HandleOSMessages() override;
  bool DeviceOK() override;
  void ToggleFullScreen() override;

  HWND GetHandle() override;

  void FinalizeMouseCursor() override;
  void SetWindowTitle(std::string_view title) override;

 protected:
  static LRESULT CALLBACK WndProcProxy(HWND h_wnd, UINT u_msg, WPARAM w_param,
                                       LPARAM l_param);
  virtual LRESULT WindowProc(UINT u_msg, WPARAM w_param, LPARAM l_param);

  void HandleResize() override;
  void HandleAltEnter() override;

  HWND window_handle_ = nullptr;
  WINDOWPLACEMENT window_placement_{};
  int32_t dw_style_ = 0;
  int32_t full_screen_x_ = 0, full_screen_y_ = 0;

 private:
  HCURSOR& MouseCursorsAt(CoreMouseCursor c) {
    return mouse_cursors_[static_cast<uint16_t>(c)];
  }

  std::array<HCURSOR, 8> mouse_cursors_ = {0};
};

}  // namespace renderer
