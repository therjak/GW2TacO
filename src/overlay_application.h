#pragma once
#include "src/white_board/application.h"

class COverlayApp : public CWBApplication {
 public:
  bool Initialize(const CCoreWindowParameters& WindowParams) override;

  COverlayApp();
  ~COverlayApp() override;

  void TakeScreenshot() override;

  std::unique_ptr<CCoreBlendState> holePunchBlendState;

 protected:
  LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
  bool DeviceOK() override;
};
