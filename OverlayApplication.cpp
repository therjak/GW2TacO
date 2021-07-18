#include "OverlayApplication.h"

COverlayApp::COverlayApp() = default;

bool COverlayApp::Initialize(const CCoreWindowParameters& WindowParams) {
  FORCEDDEBUGLOG("COverlayApp::Initialize()");

  if (!CWBApplication::Initialize(WindowParams)) return false;

  FORCEDDEBUGLOG("CWBApplication::Initialize() returned with true.");

  auto GuiBlendState = Device->CreateBlendState();
  if (!GuiBlendState) {
    LOG(LOG_ERROR, _T( "[gui] Error creating UI Blend State" ));
    return false;
  }

  FORCEDDEBUGLOG("GUI Blendstate instance created");

  GuiBlendState->SetBlendEnable(0, true);
  GuiBlendState->SetSrcBlend(0, COREBLEND_SRCALPHA);
  GuiBlendState->SetDestBlend(0, COREBLEND_INVSRCALPHA);
  GuiBlendState->SetSrcBlendAlpha(0, COREBLEND_ONE);
  GuiBlendState->SetDestBlendAlpha(0, COREBLEND_INVSRCALPHA);

  DrawAPI->SetUIBlendState(std::move(GuiBlendState));

  holePunchBlendState.swap(DrawAPI->GetDevice()->CreateBlendState());
  holePunchBlendState->SetBlendEnable(0, true);
  holePunchBlendState->SetIndependentBlend(true);
  holePunchBlendState->SetSrcBlend(0, COREBLEND_ZERO);
  holePunchBlendState->SetDestBlend(0, COREBLEND_ZERO);
  holePunchBlendState->SetSrcBlendAlpha(0, COREBLEND_ZERO);
  holePunchBlendState->SetDestBlendAlpha(0, COREBLEND_ZERO);

  FORCEDDEBUGLOG("GUI Blendstate set, app init fully done");

  return true;
}

COverlayApp::~COverlayApp() = default;

void COverlayApp::TakeScreenshot() {}

bool COverlayApp::DeviceOK() {
  if (!Device) return false;
  return Device->DeviceOk();
}

void FetchMarkerPackOnline(std::string_view ourl);

LRESULT COverlayApp::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_COPYDATA:
      PCOPYDATASTRUCT pcpy = (PCOPYDATASTRUCT)lParam;
      if (pcpy) {
        std::string_view incoming(static_cast<char*>(pcpy->lpData),
                                  pcpy->cbData);
        FetchMarkerPackOnline(incoming);
      }
      break;
  }

  if (uMsg > 0x60ff) {
    int z = 0;
  }

  return CWBApplication::WindowProc(uMsg, wParam, lParam);
}
