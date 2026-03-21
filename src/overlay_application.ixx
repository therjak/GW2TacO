module;
#include <memory>
#include "src/white_board/application.h"

export module taco.overlay_application;

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
