#include "src/core2/dx11_render_state.h"

#include <comdef.h>
#include <d3d11.h>

#include "src/base/logger.h"

namespace renderer {

CCoreDX11BlendState::CCoreDX11BlendState(CCoreDX11Device* device)
    : CCoreBlendState(device) {
  device_ = device;
  d3d_device_ = device_->GetDevice();
  context_ = device_->GetDeviceContext();
  state_ = nullptr;
}

CCoreDX11BlendState::~CCoreDX11BlendState() {
  if (state_) state_->Release();
}

bool CCoreDX11BlendState::Update() {
  if (!dirty_) return true;
  if (state_) state_->Release();
  state_ = nullptr;

  D3D11_BLEND_DESC desc;
  desc.AlphaToCoverageEnable = alpha_to_coverage_;
  desc.IndependentBlendEnable = independent_blend_;
  for (int32_t x = 0; x < 8; x++) {
    desc.RenderTarget[x].BlendEnable =
        render_target_blend_states_[x].blend_enable;
    desc.RenderTarget[x].SrcBlend =
        DX11BlendFactorsAt(render_target_blend_states_[x].src_blend);
    desc.RenderTarget[x].DestBlend =
        DX11BlendFactorsAt(render_target_blend_states_[x].dest_blend);
    desc.RenderTarget[x].BlendOp = DX11BlendOps[static_cast<uint8_t>(
        render_target_blend_states_[x].blend_op)];
    desc.RenderTarget[x].SrcBlendAlpha =
        DX11BlendFactorsAt(render_target_blend_states_[x].src_blend_alpha);
    desc.RenderTarget[x].DestBlendAlpha =
        DX11BlendFactorsAt(render_target_blend_states_[x].dest_blend_alpha);
    desc.RenderTarget[x].BlendOpAlpha = DX11BlendOps[static_cast<uint8_t>(
        render_target_blend_states_[x].blend_op_alpha)];
    desc.RenderTarget[x].RenderTargetWriteMask =
        render_target_blend_states_[x].render_target_write_mask;
  }

  dirty_ = false;

  const HRESULT result = d3d_device_->CreateBlendState(&desc, &state_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectX11 Blend state creation failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  return true;
}

bool CCoreDX11BlendState::Apply() {
  Update();
  if (device_->GetCurrentBlendState() != state_) {
    context_->OMSetBlendState(state_, nullptr, 0xffffffff);
    device_->SetCurrentBlendState(state_);
  }
  return true;
}

CCoreDX11DepthStencilState::CCoreDX11DepthStencilState(CCoreDX11Device* device)
    : CCoreDepthStencilState(device) {
  device_ = device;
  d3d_device_ = device_->GetDevice();
  context_ = device_->GetDeviceContext();
  state_ = nullptr;
}

CCoreDX11DepthStencilState::~CCoreDX11DepthStencilState() {
  if (state_) state_->Release();
}

bool CCoreDX11DepthStencilState::Update() {
  if (!dirty_) return true;
  if (state_) state_->Release();
  state_ = nullptr;

  D3D11_DEPTH_STENCIL_DESC desc;
  memset(&desc, 0, sizeof(D3D11_DEPTH_STENCIL_DESC));

  desc.DepthEnable = depth_enable_;
  desc.DepthWriteMask = z_write_enable_ ? D3D11_DEPTH_WRITE_MASK_ALL
                                        : D3D11_DEPTH_WRITE_MASK_ZERO;
  desc.DepthFunc = DX11ComparisonFunctionsAt(depth_func_);
  desc.StencilEnable = false;

  dirty_ = false;

  const HRESULT result = d3d_device_->CreateDepthStencilState(&desc, &state_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectX11 Depth Stencil state creation failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  return true;
}

bool CCoreDX11DepthStencilState::Apply() {
  Update();
  if (device_->GetCurrentDepthStencilState() != state_) {
    context_->OMSetDepthStencilState(state_, 0);
    device_->SetCurrentDepthStencilState(state_);
  }
  return true;
}

CCoreDX11RasterizerState::CCoreDX11RasterizerState(CCoreDX11Device* device)
    : CCoreRasterizerState(device) {
  device_ = device;
  d3d_device_ = device_->GetDevice();
  context_ = device_->GetDeviceContext();
  state_ = nullptr;
}

CCoreDX11RasterizerState::~CCoreDX11RasterizerState() {
  if (state_) state_->Release();
}

bool CCoreDX11RasterizerState::Update() {
  if (!dirty_) return true;
  if (state_) state_->Release();
  state_ = nullptr;

  D3D11_RASTERIZER_DESC desc;
  desc.AntialiasedLineEnable = antialiased_line_enable_;
  desc.CullMode = DX11CullModes[static_cast<uint8_t>(cull_mode_)];
  desc.DepthBias = depth_bias_;
  desc.DepthBiasClamp = depth_bias_clamp_;
  desc.FillMode = DX11FillModes[static_cast<uint8_t>(fill_mode_)];
  desc.FrontCounterClockwise = front_counter_clockwise_;
  desc.MultisampleEnable = multisample_enable_;
  desc.ScissorEnable = scissor_enable_;
  desc.SlopeScaledDepthBias = slope_scaled_depth_bias_;

  dirty_ = false;

  const HRESULT result = d3d_device_->CreateRasterizerState(&desc, &state_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectX11 Rasterizer state creation failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  return true;
}

bool CCoreDX11RasterizerState::Apply() {
  Update();
  if (device_->GetCurrentRasterizerState() != state_) {
    context_->RSSetState(state_);
    device_->SetCurrentRasterizerState(state_);
  }
  return true;
}

CCoreDX11SamplerState::CCoreDX11SamplerState(CCoreDX11Device* device)
    : CCoreSamplerState(device) {
  device_ = device;
  d3d_device_ = device_->GetDevice();
  context_ = device_->GetDeviceContext();
  state_ = nullptr;
}

CCoreDX11SamplerState::~CCoreDX11SamplerState() {
  if (state_) state_->Release();
}

bool CCoreDX11SamplerState::Update() {
  if (!dirty_) return true;
  if (state_) state_->Release();
  state_ = nullptr;

  D3D11_SAMPLER_DESC desc;

  desc.AddressU = DX11TextureAddressModes[static_cast<uint8_t>(address_u_)];
  desc.AddressV = DX11TextureAddressModes[static_cast<uint8_t>(address_v_)];
  desc.AddressW = DX11TextureAddressModes[static_cast<uint8_t>(address_w_)];
  desc.BorderColor[0] = border_color_[0];
  desc.BorderColor[1] = border_color_[1];
  desc.BorderColor[2] = border_color_[2];
  desc.BorderColor[3] = border_color_[3];
  desc.ComparisonFunc = DX11ComparisonFunctionsAt(comparison_func_);
  desc.Filter = DX11Filters[static_cast<uint16_t>(filter_)];
  desc.MaxAnisotropy = max_anisotropy_;
  desc.MaxLOD = max_lod_;
  desc.MinLOD = min_lod_;
  desc.MipLODBias = mip_lod_bias_;

  dirty_ = false;

  const HRESULT result = d3d_device_->CreateSamplerState(&desc, &state_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectX11 Sampler state creation failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  return true;
}

bool CCoreDX11SamplerState::Apply(CoreSampler sampler) {
  Update();

  if (sampler >= CoreSampler::kPs0 && sampler <= CoreSampler::kPs15) {
    const uint32_t slot =
        static_cast<uint32_t>(sampler) - static_cast<uint32_t>(CoreSampler::kPs0);
    context_->PSSetSamplers(slot, 1, &state_);
  }

  if (sampler >= CoreSampler::kVs0 && sampler <= CoreSampler::kVs3) {
    const uint32_t slot =
        static_cast<uint32_t>(sampler) - static_cast<uint32_t>(CoreSampler::kVs0);
    context_->VSSetSamplers(slot, 1, &state_);
  }

  if (sampler >= CoreSampler::kGs0 && sampler <= CoreSampler::kGs3) {
    const uint32_t slot =
        static_cast<uint32_t>(sampler) - static_cast<uint32_t>(CoreSampler::kGs0);
    context_->GSSetSamplers(slot, 1, &state_);
  }

  return true;
}

}  // namespace renderer
