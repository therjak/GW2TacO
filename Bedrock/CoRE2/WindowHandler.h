#pragma once
#include <vector>

#include "../BaseLib/BaseLib.h"
#include "../BaseLib/Rectangle.h"

class CCoreDevice;

enum COREMOUSECURSOR {
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
  CCoreDevice *Device = nullptr;
  HINSTANCE hInstance;
  TBOOL FullScreen;
  int32_t XRes;
  int32_t YRes;
  TCHAR *WindowTitle;
  HICON Icon;
  TBOOL Maximized;
  TBOOL ResizeDisabled;

  DWORD OverrideWindowStyle = 0;
  DWORD OverrideWindowStyleEx = 0;

  CCoreWindowParameters();
  CCoreWindowParameters(HINSTANCE hinst, TBOOL FullScreen, int32_t XRes,
                        int32_t YRes, TCHAR *WindowTitle, HICON Icon = nullptr,
                        TBOOL Maximized = false, TBOOL ResizeDisabled = false);

  virtual void Initialize(CCoreDevice *device, HINSTANCE hinst,
                          TBOOL FullScreen, int32_t XRes, int32_t YRes,
                          TCHAR *WindowTitle, HICON Icon = nullptr,
                          TBOOL Maximized = false,
                          TBOOL ResizeDisabled = false);
};

//////////////////////////////////////////////////////////////////////////
// interface

class CCoreWindowHandler {
 protected:
  TBOOL Done;
  CCoreDevice *Device;
  TBOOL Active;
  TBOOL Maximized;
  TBOOL Minimized;
  CRect ClientRect;

  TBOOL InactiveFrameLimiter;
  int32_t LimitedFPS;
  int32_t LastRenderedFrame;

  int32_t XRes, YRes;

  CCoreWindowParameters InitParameters;

  COREMOUSECURSOR CurrentMouseCursor;

  CPoint MousePos, LeftDownPos, RightDownPos, MidDownPos;

  virtual void HandleResize() = 0;
  virtual void HandleAltEnter() = 0;

 public:
  CCoreWindowHandler();
  virtual ~CCoreWindowHandler();

  // this initializer will change to accommodate multiple platforms at once once
  // we get to that point:
  virtual TBOOL Initialize(const CCoreWindowParameters &WindowParams) = 0;

  virtual void Destroy();
  virtual TBOOL HandleMessages() = 0;
  virtual TBOOL HandleOSMessages() = 0;
  virtual TBOOL DeviceOK() = 0;
  virtual void ToggleFullScreen() = 0;

  virtual uint32_t GetHandle() = 0;

  virtual int32_t GetXRes();
  virtual int32_t GetYRes();
  virtual CCoreWindowParameters &GetInitParameters();

  virtual void SelectMouseCursor(COREMOUSECURSOR Cursor);
  virtual void FinalizeMouseCursor() = 0;
  CPoint GetMousePos();
  CPoint GetLeftDownPos();
  CPoint GetRightDownPos();
  CPoint GetMidDownPos();

  INLINE CCoreDevice *GetDevice() { return Device; }

  virtual void SetWindowTitle(std::string_view Title) = 0;
  virtual void SetInactiveFrameLimiter(TBOOL set);
};

//////////////////////////////////////////////////////////////////////////
// windows implementation

class CCoreWindowHandlerWin : public CCoreWindowHandler {
  std::vector<HCURSOR> MouseCursors;

 protected:
  HWND hWnd;
  WINDOWPLACEMENT WindowPlacement;
  int32_t dwStyle;
  int32_t FullScreenX, FullScreenY;

  static LRESULT CALLBACK WndProcProxy(HWND hWnd, UINT uMsg, WPARAM wParam,
                                       LPARAM lParam);
  virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

  void HandleResize() override;
  void HandleAltEnter() override;

 public:
  CCoreWindowHandlerWin();
  ~CCoreWindowHandlerWin() override;

  TBOOL Initialize(const CCoreWindowParameters &WindowParams) override;
  void Destroy() override;
  TBOOL HandleMessages() override;
  TBOOL HandleOSMessages() override;
  TBOOL DeviceOK() override;
  void ToggleFullScreen() override;

  uint32_t GetHandle() override;

  void FinalizeMouseCursor() override;
  void SetWindowTitle(std::string_view Title) override;
};
