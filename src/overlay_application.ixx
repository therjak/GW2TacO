module;
#include <memory>

#include "src/core2/render_state.h"

export module taco.overlay_application;

import whiteboard;

export class COverlayApp : public CWBApplication {
 public:
  bool Initialize(const renderer::CCoreWindowParameters& WindowParams) override;

  COverlayApp();
  ~COverlayApp() override;

  void TakeScreenshot() override;

  std::unique_ptr<renderer::CCoreBlendState> holePunchBlendState;

 protected:
  bool DeviceOK() override;
};
