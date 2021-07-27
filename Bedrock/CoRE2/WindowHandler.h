#pragma once
#include <memory>
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
  HINSTANCE hInstance;
  bool FullScreen;
  int32_t XRes;
  int32_t YRes;
  TCHAR* WindowTitle;
  HICON Icon;
  bool Maximized;
  bool ResizeDisabled;

  DWORD OverrideWindowStyle = 0;
  DWORD OverrideWindowStyleEx = 0;

  CCoreWindowParameters();
  CCoreWindowParameters(HINSTANCE hinst, bool FullScreen, int32_t XRes,
                        int32_t YRes, TCHAR* WindowTitle, HICON Icon = nullptr,
                        bool Maximized = false, bool ResizeDisabled = false);

  std::unique_ptr<CCoreDevice> CreateDevice() const;
};

//////////////////////////////////////////////////////////////////////////
// interface

class CCoreWindowHandler {
 protected:
  bool Done = false;
  std::unique_ptr<CCoreDevice> Device;
  bool Active = false;
  bool Maximized = false;
  bool Minimized = false;
  CRect ClientRect;

  bool InactiveFrameLimiter = true;
  int32_t LimitedFPS = 20;
  int32_t LastRenderedFrame = 0;

  int32_t XRes = 0, YRes = 0;

  CCoreWindowParameters InitParameters;

  COREMOUSECURSOR CurrentMouseCursor = CM_ARROW;

  CPoint MousePos, LeftDownPos, RightDownPos, MidDownPos;

  virtual void HandleResize() = 0;
  virtual void HandleAltEnter() = 0;

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

  virtual uint32_t GetHandle() = 0;

  virtual int32_t GetXRes();
  virtual int32_t GetYRes();
  virtual CCoreWindowParameters& GetInitParameters();

  virtual void SelectMouseCursor(COREMOUSECURSOR Cursor);
  virtual void FinalizeMouseCursor() = 0;
  CPoint GetMousePos();
  CPoint GetLeftDownPos();
  CPoint GetRightDownPos();
  CPoint GetMidDownPos();

  INLINE CCoreDevice* GetDevice() { return Device.get(); }

  virtual void SetWindowTitle(std::string_view Title) = 0;
  virtual void SetInactiveFrameLimiter(bool set);
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

  bool Initialize(const CCoreWindowParameters& WindowParams) override;
  void Destroy() override;
  bool HandleMessages() override;
  bool HandleOSMessages() override;
  bool DeviceOK() override;
  void ToggleFullScreen() override;

  uint32_t GetHandle() override;

  void FinalizeMouseCursor() override;
  void SetWindowTitle(std::string_view Title) override;
};
