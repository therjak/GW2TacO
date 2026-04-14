#pragma once
#include <d3d11.h>

#include "src/core2/dx11_device.h"
#include "src/core2/render_state.h"

namespace renderer {

class DX11BlendState : public BlendState {
 public:
  explicit DX11BlendState(DX11Device* device);
  ~DX11BlendState() override;

  bool Update() override;
  bool Apply() override;
  void* GetHandle() override { return state_; }

 private:
  DX11Device* device_;
  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* context_;
  ID3D11BlendState* state_;
};

class DX11DepthStencilState : public DepthStencilState {
 public:
  explicit DX11DepthStencilState(DX11Device* device);
  ~DX11DepthStencilState() override;

  bool Update() override;
  bool Apply() override;
  void* GetHandle() override { return state_; }

 private:
  DX11Device* device_;
  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* context_;
  ID3D11DepthStencilState* state_;
};

class DX11RasterizerState : public RasterizerState {
 public:
  explicit DX11RasterizerState(DX11Device* device);
  ~DX11RasterizerState() override;

  bool Update() override;
  bool Apply() override;
  void* GetHandle() override { return state_; }

 private:
  DX11Device* device_;
  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* context_;
  ID3D11RasterizerState* state_;
};

class DX11SamplerState : public SamplerState {
 public:
  explicit DX11SamplerState(DX11Device* device);
  ~DX11SamplerState() override;

  bool Update() override;
  bool Apply(Sampler sampler) override;
  void* GetHandle() override { return state_; }

 private:
  DX11Device* device_;
  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* context_;
  ID3D11SamplerState* state_;
};

}  // namespace renderer
