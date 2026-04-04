module;
#include <memory>

#include "src/core2/render_state.h"

export module taco.overlay_application;

import whiteboard;

export class COverlayApp : public CWBApplication {
 public:
  bool Initialize(const CCoreWindowParameters& WindowParams) override;

  COverlayApp();
  ~COverlayApp() override;

  void TakeScreenshot() override;

  std::unique_ptr<CCoreBlendState> holePunchBlendState;

 protected:
  bool DeviceOK() override;
};
