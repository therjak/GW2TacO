module;
#include "src/base/logger.h"
#include "src/core2/core2.h"

module taco.overlay_application;

import whiteboard;

COverlayApp::COverlayApp() = default;

bool COverlayApp::Initialize(const renderer::CCoreWindowParameters& WindowParams) {
  if (!CWBApplication::Initialize(WindowParams)) return false;

  auto GuiBlendState = device_->CreateBlendState();
  if (!GuiBlendState) {
    Log_Err("[gui] Error creating UI Blend State");
    return false;
  }

  GuiBlendState->SetBlendEnable(0, true);
  GuiBlendState->SetSrcBlend(0, renderer::CoreBlendFactor::kSrcAlpha);
  GuiBlendState->SetDestBlend(0, renderer::CoreBlendFactor::kInvSrcAlpha);
  GuiBlendState->SetSrcBlendAlpha(0, renderer::CoreBlendFactor::kOne);
  GuiBlendState->SetDestBlendAlpha(0, renderer::CoreBlendFactor::kInvSrcAlpha);

  DrawAPI->SetUIBlendState(std::move(GuiBlendState));

  holePunchBlendState = DrawAPI->GetDevice()->CreateBlendState();
  holePunchBlendState->SetBlendEnable(0, true);
  holePunchBlendState->SetIndependentBlend(true);
  holePunchBlendState->SetSrcBlend(0, renderer::CoreBlendFactor::kZero);
  holePunchBlendState->SetDestBlend(0, renderer::CoreBlendFactor::kZero);
  holePunchBlendState->SetSrcBlendAlpha(0, renderer::CoreBlendFactor::kZero);
  holePunchBlendState->SetDestBlendAlpha(0, renderer::CoreBlendFactor::kZero);

  return true;
}

COverlayApp::~COverlayApp() = default;

void COverlayApp::TakeScreenshot() {}

bool COverlayApp::DeviceOK() {
  if (!device_) return false;
  return device_->DeviceOk();
}
