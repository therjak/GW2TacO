#pragma once
#include "src/core2/dx11_device.h"
#include "src/core2/render_state.h"

class CCoreDX11BlendState : public CCoreBlendState {
  CCoreDX11Device* Device;
  ID3D11Device* Dev;
  ID3D11DeviceContext* Context;
  ID3D11BlendState* State;

 public:
  explicit CCoreDX11BlendState(CCoreDX11Device* Device);
  ~CCoreDX11BlendState() override;

  bool Update() override;
  bool Apply() override;
  void* GetHandle() override { return State; }
};

class CCoreDX11DepthStencilState : public CCoreDepthStencilState {
  CCoreDX11Device* Device;
  ID3D11Device* Dev;
  ID3D11DeviceContext* Context;
  ID3D11DepthStencilState* State;

 public:
  explicit CCoreDX11DepthStencilState(CCoreDX11Device* Device);
  ~CCoreDX11DepthStencilState() override;

  bool Update() override;
  bool Apply() override;
  void* GetHandle() override { return State; }
};

class CCoreDX11RasterizerState : public CCoreRasterizerState {
  CCoreDX11Device* Device;
  ID3D11Device* Dev;
  ID3D11DeviceContext* Context;
  ID3D11RasterizerState* State;

 public:
  explicit CCoreDX11RasterizerState(CCoreDX11Device* Device);
  ~CCoreDX11RasterizerState() override;

  bool Update() override;
  bool Apply() override;
  void* GetHandle() override { return State; }
};

class CCoreDX11SamplerState : public CCoreSamplerState {
  CCoreDX11Device* Device;
  ID3D11Device* Dev;
  ID3D11DeviceContext* Context;
  ID3D11SamplerState* State;

 public:
  explicit CCoreDX11SamplerState(CCoreDX11Device* Device);
  ~CCoreDX11SamplerState() override;

  bool Update() override;
  bool Apply(CORESAMPLER Smp) override;
  void* GetHandle() override { return State; }
};
