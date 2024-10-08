﻿#include "src/overlay_application.h"

#include "src/base/logger.h"
#include "src/white_board/application.h"

COverlayApp::COverlayApp() = default;

bool COverlayApp::Initialize(const CCoreWindowParameters& WindowParams) {
  if (!CWBApplication::Initialize(WindowParams)) return false;

  auto GuiBlendState = Device->CreateBlendState();
  if (!GuiBlendState) {
    Log_Err("[gui] Error creating UI Blend State");
    return false;
  }

  GuiBlendState->SetBlendEnable(0, true);
  GuiBlendState->SetSrcBlend(0, COREBLENDFACTOR::SRCALPHA);
  GuiBlendState->SetDestBlend(0, COREBLENDFACTOR::INVSRCALPHA);
  GuiBlendState->SetSrcBlendAlpha(0, COREBLENDFACTOR::ONE);
  GuiBlendState->SetDestBlendAlpha(0, COREBLENDFACTOR::INVSRCALPHA);

  DrawAPI->SetUIBlendState(std::move(GuiBlendState));

  holePunchBlendState = DrawAPI->GetDevice()->CreateBlendState();
  holePunchBlendState->SetBlendEnable(0, true);
  holePunchBlendState->SetIndependentBlend(true);
  holePunchBlendState->SetSrcBlend(0, COREBLENDFACTOR::ZERO);
  holePunchBlendState->SetDestBlend(0, COREBLENDFACTOR::ZERO);
  holePunchBlendState->SetSrcBlendAlpha(0, COREBLENDFACTOR::ZERO);
  holePunchBlendState->SetDestBlendAlpha(0, COREBLENDFACTOR::ZERO);

  return true;
}

COverlayApp::~COverlayApp() = default;

void COverlayApp::TakeScreenshot() {}

bool COverlayApp::DeviceOK() {
  if (!Device) return false;
  return Device->DeviceOk();
}
