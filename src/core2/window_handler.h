#pragma once
#include <windows.h>

#include <array>
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include "src/base/rectangle.h"

class CCoreDevice;

enum class COREMOUSECURSOR : uint16_t {
  CM_ARROW,
  CM_CROSS,
  CM_SIZEWE,
  CM_SIZENS,
  CM_SIZENESW,
  CM_SIZENWSE,
  CM_TEXT,
  CM_WAIT,
};

//////////////////////////////////////////////////////////////////////////
// window init parameter structure

class CCoreWindowParameters {
 public:
  CCoreWindowParameters();
  CCoreWindowParameters(HINSTANCE hinst, bool FullScreen, int32_t XRes,
                        int32_t YRes, const TCHAR* WindowTitle,
                        HICON Icon = nullptr, bool Maximized = false,
                        bool ResizeDisabled = false);

  [[nodiscard]] std::unique_ptr<CCoreDevice> CreateDevice() const;

  HINSTANCE hInstance = nullptr;
  bool FullScreen = false;
  int32_t XRes = 800;
  int32_t YRes = 600;
  const TCHAR* WindowTitle = nullptr;
  HICON Icon = nullptr;
  bool Maximized = false;
  bool ResizeDisabled = false;

  DWORD OverrideWindowStyle = 0;
  DWORD OverrideWindowStyleEx = 0;
};

//////////////////////////////////////////////////////////////////////////
// interface

class CCoreWindowHandler {
 public:
  CCoreWindowHandler();
  virtual ~CCoreWindowHandler();

  // this initializer will change to accommodate multiple platforms at once once
  // we get to that point:
  virtual bool Initialize(const CCoreWindowParameters& WindowParams) = 0;

  virtual void Destroy();
  virtual bool HandleMessages() = 0;
  virtual bool HandleOSMessages() = 0;
  virtual bool DeviceOK() = 0;
  virtual void ToggleFullScreen() = 0;

  virtual HWND GetHandle() = 0;

  virtual int32_t GetXRes();
  virtual int32_t GetYRes();
  virtual CCoreWindowParameters& GetInitParameters();

  virtual void SelectMouseCursor(COREMOUSECURSOR Cursor);
  virtual void FinalizeMouseCursor() = 0;
  math::CPoint GetMousePos();
  math::CPoint GetLeftDownPos();
  math::CPoint GetRightDownPos();
  math::CPoint GetMidDownPos();

  CCoreDevice* GetDevice() { return Device.get(); }

  virtual void SetWindowTitle(std::string_view Title) = 0;
  virtual void SetInactiveFrameLimiter(bool set);

 protected:
  virtual void HandleResize() = 0;
  virtual void HandleAltEnter() = 0;

  bool Done = false;
  std::unique_ptr<CCoreDevice> Device;
  bool Active = false;
  bool Maximized = false;
  bool Minimized = false;
  math::CRect ClientRect;

  bool InactiveFrameLimiter = true;
  int32_t LimitedFPS = 20;
  int32_t LastRenderedFrame = 0;

  int32_t XRes = 0, YRes = 0;

  CCoreWindowParameters InitParameters;

  COREMOUSECURSOR CurrentMouseCursor = COREMOUSECURSOR::CM_ARROW;

  math::CPoint MousePos, LeftDownPos, RightDownPos, MidDownPos;
};

//////////////////////////////////////////////////////////////////////////
// windows implementation

class CCoreWindowHandlerWin : public CCoreWindowHandler {
 public:
  CCoreWindowHandlerWin();
  ~CCoreWindowHandlerWin() override;

  bool Initialize(const CCoreWindowParameters& WindowParams) override;
  void Destroy() override;
  bool HandleMessages() override;
  bool HandleOSMessages() override;
  bool DeviceOK() override;
  void ToggleFullScreen() override;

  HWND GetHandle() override;

  void FinalizeMouseCursor() override;
  void SetWindowTitle(std::string_view Title) override;

 protected:
  static LRESULT CALLBACK WndProcProxy(HWND hWnd, UINT uMsg, WPARAM wParam,
                                       LPARAM lParam);
  virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

  void HandleResize() override;
  void HandleAltEnter() override;

  HWND hWnd = nullptr;
  WINDOWPLACEMENT WindowPlacement{};
  int32_t dwStyle = 0;
  int32_t FullScreenX = 0, FullScreenY = 0;

 private:
  HCURSOR& MouseCursorsAt(COREMOUSECURSOR c) {
    return MouseCursors[static_cast<uint16_t>(c)];
  }

  std::array<HCURSOR, 8> MouseCursors = {0};
};
