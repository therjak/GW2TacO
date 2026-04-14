module;
#include "src/base/logger.h"
#include "src/core2/core2.h"

module taco.overlay_application;

import whiteboard;

COverlayApp::COverlayApp() = default;

bool COverlayApp::Initialize(const renderer::WindowParameters& WindowParams) {
  if (!CWBApplication::Initialize(WindowParams)) return false;

  auto GuiBlendState = device_->CreateBlendState();
  if (!GuiBlendState) {
    Log_Err("[gui] Error creating UI Blend State");
    return false;
  }

  GuiBlendState->SetBlendEnable(0, true);
  GuiBlendState->SetSrcBlend(0, renderer::BlendFactor::kSrcAlpha);
  GuiBlendState->SetDestBlend(0, renderer::BlendFactor::kInvSrcAlpha);
  GuiBlendState->SetSrcBlendAlpha(0, renderer::BlendFactor::kOne);
  GuiBlendState->SetDestBlendAlpha(0, renderer::BlendFactor::kInvSrcAlpha);

  DrawAPI->SetUIBlendState(std::move(GuiBlendState));

  holePunchBlendState = DrawAPI->GetDevice()->CreateBlendState();
  holePunchBlendState->SetBlendEnable(0, true);
  holePunchBlendState->SetIndependentBlend(true);
  holePunchBlendState->SetSrcBlend(0, renderer::BlendFactor::kZero);
  holePunchBlendState->SetDestBlend(0, renderer::BlendFactor::kZero);
  holePunchBlendState->SetSrcBlendAlpha(0, renderer::BlendFactor::kZero);
  holePunchBlendState->SetDestBlendAlpha(0, renderer::BlendFactor::kZero);

  return true;
}

COverlayApp::~COverlayApp() = default;

void COverlayApp::TakeScreenshot() {}

bool COverlayApp::DeviceOK() {
  if (!device_) return false;
  return device_->DeviceOk();
}
