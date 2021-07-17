﻿#pragma once
#include "Bedrock/Whiteboard/WhiteBoard.h"

class COverlayApp : public CWBApplication {
 protected:
  LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
  TBOOL DeviceOK() override;

 public:
  TBOOL Initialize(const CCoreWindowParameters &WindowParams) override;

  COverlayApp();
  ~COverlayApp() override;

  void TakeScreenshot() override;

  std::unique_ptr<CCoreBlendState> holePunchBlendState;
};
