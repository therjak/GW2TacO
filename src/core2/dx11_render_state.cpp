#include "src/core2/dx11_render_state.h"

#include <comdef.h>

#include "src/base/logger.h"

CCoreDX11BlendState::CCoreDX11BlendState(CCoreDX11Device* d)
    : CCoreBlendState(d) {
  Device = d;
  Dev = Device->GetDevice();
  Context = Device->GetDeviceContext();
  State = nullptr;
}

CCoreDX11BlendState::~CCoreDX11BlendState() {
  if (State) State->Release();
}

bool CCoreDX11BlendState::Update() {
  if (!Dirty) return true;
  if (State) State->Release();
  State = nullptr;

  D3D11_BLEND_DESC desc;
  desc.AlphaToCoverageEnable = AlphaToCoverage;
  desc.IndependentBlendEnable = IndependentBlend;
  for (int32_t x = 0; x < 8; x++) {
    desc.RenderTarget[x].BlendEnable = RenderTargetBlendStates[x].BlendEnable;
    desc.RenderTarget[x].SrcBlend =
        DX11BlendFactorsAt(RenderTargetBlendStates[x].SrcBlend);
    desc.RenderTarget[x].DestBlend =
        DX11BlendFactorsAt(RenderTargetBlendStates[x].DestBlend);
    desc.RenderTarget[x].BlendOp =
        DX11BlendOps[static_cast<uint8_t>(RenderTargetBlendStates[x].BlendOp)];
    desc.RenderTarget[x].SrcBlendAlpha =
        DX11BlendFactorsAt(RenderTargetBlendStates[x].SrcBlendAlpha);
    desc.RenderTarget[x].DestBlendAlpha =
        DX11BlendFactorsAt(RenderTargetBlendStates[x].DestBlendAlpha);
    desc.RenderTarget[x].BlendOpAlpha = DX11BlendOps[static_cast<uint8_t>(
        RenderTargetBlendStates[x].BlendOpAlpha)];
    desc.RenderTarget[x].RenderTargetWriteMask =
        RenderTargetBlendStates[x].RenderTargetWriteMask;
  }

  Dirty = false;

  const HRESULT res = Dev->CreateBlendState(&desc, &State);
  if (res != S_OK) {
    _com_error err(res);
    Log_Err("[core] DirectX11 Blend state creation failed ({:s})",
            err.ErrorMessage());
    return false;
  }

  return true;
}

bool CCoreDX11BlendState::Apply() {
  Update();
  if (Device->GetCurrentBlendState() != State) {
    Context->OMSetBlendState(State, nullptr, 0xffffffff);
    Device->SetCurrentBlendState(State);
  }
  return true;
}

CCoreDX11DepthStencilState::CCoreDX11DepthStencilState(CCoreDX11Device* d)
    : CCoreDepthStencilState(d) {
  Device = d;
  Dev = Device->GetDevice();
  Context = Device->GetDeviceContext();
  State = nullptr;
}

CCoreDX11DepthStencilState::~CCoreDX11DepthStencilState() {
  if (State) State->Release();
}

bool CCoreDX11DepthStencilState::Update() {
  if (!Dirty) return true;
  if (State) State->Release();
  State = nullptr;

  D3D11_DEPTH_STENCIL_DESC desc;
  memset(&desc, 0, sizeof(D3D11_DEPTH_STENCIL_DESC));

  desc.DepthEnable = DepthEnable;
  desc.DepthWriteMask =
      ZWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
  desc.DepthFunc = DX11ComparisonFunctionsAt(DepthFunc);
  desc.StencilEnable = false;

  Dirty = false;

  const HRESULT res = Dev->CreateDepthStencilState(&desc, &State);
  if (res != S_OK) {
    _com_error err(res);
    Log_Err("[core] DirectX11 Depth Stencil state creation failed ({:s})",
            err.ErrorMessage());
    return false;
  }

  return true;
}

bool CCoreDX11DepthStencilState::Apply() {
  Update();
  if (Device->GetCurrentDepthStencilState() != State) {
    Context->OMSetDepthStencilState(State, 0);
    Device->SetCurrentDepthStencilState(State);
  }
  return true;
}

CCoreDX11RasterizerState::CCoreDX11RasterizerState(CCoreDX11Device* d)
    : CCoreRasterizerState(d) {
  Device = d;
  Dev = Device->GetDevice();
  Context = Device->GetDeviceContext();
  State = nullptr;
}

CCoreDX11RasterizerState::~CCoreDX11RasterizerState() {
  if (State) State->Release();
}

bool CCoreDX11RasterizerState::Update() {
  if (!Dirty) return true;
  if (State) State->Release();
  State = nullptr;

  D3D11_RASTERIZER_DESC desc;
  desc.AntialiasedLineEnable = AntialiasedLineEnable;
  desc.CullMode = DX11CullModes[static_cast<uint8_t>(CullMode)];
  desc.DepthBias = DepthBias;
  desc.DepthBiasClamp = DepthBiasClamp;
  desc.FillMode = DX11FillModes[static_cast<uint8_t>(FillMode)];
  desc.FrontCounterClockwise = FrontCounterClockwise;
  desc.MultisampleEnable = MultisampleEnable;
  desc.ScissorEnable = ScissorEnable;
  desc.SlopeScaledDepthBias = SlopeScaledDepthBias;

  Dirty = false;

  const HRESULT res = Dev->CreateRasterizerState(&desc, &State);
  if (res != S_OK) {
    _com_error err(res);
    Log_Err("[core] DirectX11 Rasterizer state creation failed ({:s})",
            err.ErrorMessage());
    return false;
  }

  return true;
}

bool CCoreDX11RasterizerState::Apply() {
  Update();
  if (Device->GetCurrentRasterizerState() != State) {
    Context->RSSetState(State);
    Device->SetCurrentRasterizerState(State);
  }
  return true;
}

CCoreDX11SamplerState::CCoreDX11SamplerState(CCoreDX11Device* d)
    : CCoreSamplerState(d) {
  Device = d;
  Dev = Device->GetDevice();
  Context = Device->GetDeviceContext();
  State = nullptr;
}

CCoreDX11SamplerState::~CCoreDX11SamplerState() {
  if (State) State->Release();
}

bool CCoreDX11SamplerState::Update() {
  if (!Dirty) return true;
  if (State) State->Release();
  State = nullptr;

  D3D11_SAMPLER_DESC desc;

  desc.AddressU = DX11TextureAddressModes[static_cast<uint8_t>(AddressU)];
  desc.AddressV = DX11TextureAddressModes[static_cast<uint8_t>(AddressV)];
  desc.AddressW = DX11TextureAddressModes[static_cast<uint8_t>(AddressW)];
  desc.BorderColor[0] = BorderColor[0];
  desc.BorderColor[1] = BorderColor[1];
  desc.BorderColor[2] = BorderColor[2];
  desc.BorderColor[3] = BorderColor[3];
  desc.ComparisonFunc = DX11ComparisonFunctionsAt(ComparisonFunc);
  desc.Filter = DX11Filters[static_cast<uint16_t>(Filter)];
  desc.MaxAnisotropy = MaxAnisotropy;
  desc.MaxLOD = MaxLOD;
  desc.MinLOD = MinLOD;
  desc.MipLODBias = MipLODBias;

  Dirty = false;

  const HRESULT res = Dev->CreateSamplerState(&desc, &State);
  if (res != S_OK) {
    _com_error err(res);
    Log_Err("[core] DirectX11 Sampler state creation failed ({:s})",
            err.ErrorMessage());
    return false;
  }

  return true;
}

bool CCoreDX11SamplerState::Apply(CORESAMPLER Smp) {
  Update();

  if (Smp >= CORESAMPLER::PS0 && Smp <= CORESAMPLER::PS15)
    Context->PSSetSamplers(Smp - CORESAMPLER::PS0, 1, &State);

  if (Smp >= CORESAMPLER::VS0 && Smp <= CORESAMPLER::VS3)
    Context->VSSetSamplers(Smp - CORESAMPLER::VS0, 1, &State);

  if (Smp >= CORESAMPLER::GS0 && Smp <= CORESAMPLER::GS3)
    Context->GSSetSamplers(Smp - CORESAMPLER::GS0, 1, &State);

  return true;
}
