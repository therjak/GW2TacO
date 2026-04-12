#pragma once
#include <d3d11.h>

#include "src/core2/dx11_device.h"
#include "src/core2/render_state.h"

namespace renderer {

class CCoreDX11BlendState : public CCoreBlendState {
 public:
  explicit CCoreDX11BlendState(CCoreDX11Device* device);
  ~CCoreDX11BlendState() override;

  bool Update() override;
  bool Apply() override;
  void* GetHandle() override { return state_; }

 private:
  CCoreDX11Device* device_;
  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* context_;
  ID3D11BlendState* state_;
};

class CCoreDX11DepthStencilState : public CCoreDepthStencilState {
 public:
  explicit CCoreDX11DepthStencilState(CCoreDX11Device* device);
  ~CCoreDX11DepthStencilState() override;

  bool Update() override;
  bool Apply() override;
  void* GetHandle() override { return state_; }

 private:
  CCoreDX11Device* device_;
  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* context_;
  ID3D11DepthStencilState* state_;
};

class CCoreDX11RasterizerState : public CCoreRasterizerState {
 public:
  explicit CCoreDX11RasterizerState(CCoreDX11Device* device);
  ~CCoreDX11RasterizerState() override;

  bool Update() override;
  bool Apply() override;
  void* GetHandle() override { return state_; }

 private:
  CCoreDX11Device* device_;
  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* context_;
  ID3D11RasterizerState* state_;
};

class CCoreDX11SamplerState : public CCoreSamplerState {
 public:
  explicit CCoreDX11SamplerState(CCoreDX11Device* device);
  ~CCoreDX11SamplerState() override;

  bool Update() override;
  bool Apply(CoreSampler sampler) override;
  void* GetHandle() override { return state_; }

 private:
  CCoreDX11Device* device_;
  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* context_;
  ID3D11SamplerState* state_;
};

}  // namespace renderer
