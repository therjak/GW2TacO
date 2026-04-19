module;
#include <memory>

#include "src/core2/render_state.h"

export module taco.overlay_application;

import whiteboard;

export class OverlayApplication : public gui::CWBApplication {
 public:
  OverlayApplication();
  ~OverlayApplication() override;

  bool Initialize(const renderer::WindowParameters& window_params) override;

  void TakeScreenshot() override;

  std::unique_ptr<renderer::BlendState> hole_punch_blend_state;

 protected:
  bool DeviceOk() override;
};
