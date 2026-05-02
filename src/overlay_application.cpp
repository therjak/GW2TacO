module;

#include <utility>

#include "src/base/logger.h"
#include "src/core2/core2.h"

module taco.overlay_application;

import whiteboard;

OverlayApplication::OverlayApplication() = default;

bool OverlayApplication::Initialize(
    const renderer::WindowParameters& window_params) {
  if (!CWBApplication::Initialize(window_params)) return false;

  auto gui_blend_state = device_->CreateBlendState();
  if (!gui_blend_state) {
    Log_Err("[gui] Error creating UI Blend State");
    return false;
  }

  gui_blend_state->SetBlendEnable(0, true);
  gui_blend_state->SetSrcBlend(0, renderer::BlendFactor::kSrcAlpha);
  gui_blend_state->SetDestBlend(0, renderer::BlendFactor::kInvSrcAlpha);
  gui_blend_state->SetSrcBlendAlpha(0, renderer::BlendFactor::kOne);
  gui_blend_state->SetDestBlendAlpha(0, renderer::BlendFactor::kInvSrcAlpha);

  DrawAPI->SetUIBlendState(std::move(gui_blend_state));

  hole_punch_blend_state = DrawAPI->GetDevice()->CreateBlendState();
  hole_punch_blend_state->SetBlendEnable(0, true);
  hole_punch_blend_state->SetIndependentBlend(true);
  hole_punch_blend_state->SetSrcBlend(0, renderer::BlendFactor::kZero);
  hole_punch_blend_state->SetDestBlend(0, renderer::BlendFactor::kZero);
  hole_punch_blend_state->SetSrcBlendAlpha(0, renderer::BlendFactor::kZero);
  hole_punch_blend_state->SetDestBlendAlpha(0, renderer::BlendFactor::kZero);

  return true;
}

OverlayApplication::~OverlayApplication() = default;

void OverlayApplication::TakeScreenshot() {}

bool OverlayApplication::DeviceOk() {
  if (!device_) return false;
  return device_->DeviceOk();
}
