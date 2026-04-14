module;
#include <memory>

#include "src/core2/render_state.h"

export module taco.overlay_application;

import whiteboard;

export class COverlayApp : public gui::CWBApplication {
 public:
  bool Initialize(const renderer::WindowParameters& WindowParams) override;

  COverlayApp();
  ~COverlayApp() override;

  void TakeScreenshot() override;

  std::unique_ptr<renderer::BlendState> holePunchBlendState;

 protected:
  bool DeviceOK() override;
};
