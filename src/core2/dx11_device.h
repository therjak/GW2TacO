#pragma once
#include <d3d11.h>
#include <dxgi1_3.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "src/base/color.h"
#include "src/core2/core2_config.h"
#include "src/core2/device.h"
#include "src/core2/dx11_enums.h"

import math;

namespace renderer {

class DX11Device : public Device {
 public:
  DX11Device();
  ~DX11Device() override;
  ID3D11Device* GetDevice() { return d3d_device_; }
  ID3D11DeviceContext* GetDeviceContext() { return d3d_device_context_; }
  DeviceApi GetAPIType() override { return DeviceApi::kDx11; }

  // this initializer will change to accommodate multiple platforms at once once
  // we get to that point:
  bool Initialize(WindowHandler* window,
                  const int32_t sample_count = 0) override;

  bool DeviceOk() override;
  bool IsWindowed() override;
  void Resize(const int32_t x_res, const int32_t y_res) override;
  void SetFullScreenMode(const bool full_screen, const int32_t x_res,
                         const int32_t y_res) override;

  ID3D11Texture2D* GetBackBuffer();

  //////////////////////////////////////////////////////////////////////////
  // texture functions

  std::unique_ptr<Texture2D> CreateTexture2D(
      const int32_t x_res, const int32_t y_res, const uint8_t* data,
      const char bytes_per_pixel = 4, const Format format = Format::kA8R8G8B8,
      const bool render_target = false) override;
  std::unique_ptr<Texture2D> CreateTexture2D(const uint8_t* data,
                                             const int32_t size) override;

  //////////////////////////////////////////////////////////////////////////
  // vertexbuffer functions

  std::unique_ptr<VertexBuffer> CreateVertexBuffer(const uint8_t* data,
                                                   const int32_t size) override;
  std::unique_ptr<VertexBuffer> CreateVertexBufferDynamic(
      const int32_t size) override;

  //////////////////////////////////////////////////////////////////////////
  // indexbuffer functions

  std::unique_ptr<IndexBuffer> CreateIndexBuffer(
      const int32_t index_count, const int32_t index_size = 2) override;

  //////////////////////////////////////////////////////////////////////////
  // vertexformat functions

  std::unique_ptr<VertexFormat> CreateVertexFormat(
      const std::vector<VertexAttribute>& attributes,
      VertexShader* vs = nullptr) override;

  //////////////////////////////////////////////////////////////////////////
  // shader functions

  std::unique_ptr<VertexShader> CreateVertexShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<PixelShader> CreatePixelShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<VertexShader> CreateVertexShaderFromBlob(
      uint8_t* code, int32_t code_size) override;
  std::unique_ptr<PixelShader> CreatePixelShaderFromBlob(
      uint8_t* code, int32_t code_size) override;
  std::unique_ptr<GeometryShader> CreateGeometryShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<DomainShader> CreateDomainShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<HullShader> CreateHullShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<ComputeShader> CreateComputeShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<VertexShader> CreateVertexShader() override;
  std::unique_ptr<PixelShader> CreatePixelShader() override;
  std::unique_ptr<GeometryShader> CreateGeometryShader() override;
  std::unique_ptr<DomainShader> CreateDomainShader() override;
  std::unique_ptr<HullShader> CreateHullShader() override;
  std::unique_ptr<ComputeShader> CreateComputeShader() override;
  void SetShaderConstants(const ConstantBuffer* buffers) override;
  std::unique_ptr<ConstantBuffer> CreateConstantBuffer() override;

  std::unique_ptr<BlendState> CreateBlendState() override;
  std::unique_ptr<DepthStencilState> CreateDepthStencilState() override;
  std::unique_ptr<RasterizerState> CreateRasterizerState() override;
  std::unique_ptr<SamplerState> CreateSamplerState() override;

  bool SetRenderTarget(Texture2D* rt) override;

  //////////////////////////////////////////////////////////////////////////
  // display functions

  bool BeginScene() override;
  bool EndScene() override;
  bool Clear(const bool clear_pixels = true, const bool clear_depth = true,
             const CColor& color = CColor(), const float depth = 1,
             const int32_t stencil = 0) override;
  bool Flip(bool vsync = true) override;
  bool DrawIndexedTriangles(int32_t count, int32_t vertex_count) override;
  bool DrawTriangles(int32_t count) override;
  bool DrawIndexedLines(int32_t count, int32_t vertex_count) override;
  bool DrawLines(int32_t count) override;

  //////////////////////////////////////////////////////////////////////////
  // renderstate functions

  bool SetViewport(const math::Rect& viewport) override;
  ID3D11BlendState* GetCurrentBlendState();
  void SetCurrentBlendState(ID3D11BlendState* blend_state);
  ID3D11RasterizerState* GetCurrentRasterizerState();
  void SetCurrentRasterizerState(ID3D11RasterizerState* rasterizer_state);
  ID3D11DepthStencilState* GetCurrentDepthStencilState();
  void SetCurrentDepthStencilState(
      ID3D11DepthStencilState* depth_stencil_state);

  void ForceStateReset() override;

  void TakeScreenShot(std::string_view filename) override;

  void InitializeDebugAPI() override;
  void CaptureCurrentFrame() override;

  //////////////////////////////////////////////////////////////////////////
  // queries

  void BeginOcclusionQuery() override;
  bool EndOcclusionQuery() override;
  void WaitRetrace() override;

  //////////////////////////////////////////////////////////////////////////
  // dx11 specific functions

  ID3D11DepthStencilView* GetDepthBufferView() { return depth_buffer_view_; }

 private:
  void ResetPrivateResources() override;
  bool InitAPI(const HWND window_handle, const bool full_screen,
               const int32_t x_res, const int32_t y_res,
               const int32_t sample_count = 0,
               const int32_t refresh_rate = 60) override;
  bool ApplyRenderState(const Sampler sampler, const RenderState render_state,
                        const RenderStateValue value) override;
  bool SetNoVertexBuffer() override;
  bool CommitRenderStates() override;

  virtual bool CreateBackBuffer(int32_t x_res, int32_t y_res);
  virtual bool CreateDepthBuffer(int32_t x_res, int32_t y_res);

  bool CreateClassicSwapChain(const HWND window_handle, const bool full_screen,
                              const int32_t x_res, const int32_t y_res,
                              const int32_t sample_count,
                              const int32_t refresh_rate);
  bool CreateDirectCompositionSwapchain(const HWND window_handle,
                                        const bool full_screen,
                                        const int32_t x_res,
                                        const int32_t y_res,
                                        const int32_t sample_count,
                                        const int32_t refresh_rate);

  IDXGISwapChain1* dxgi_swap_chain_ = nullptr;
  ID3D11Device* d3d_device_ = nullptr;
  ID3D11DeviceContext* d3d_device_context_ = nullptr;

  ID3D11RenderTargetView* back_buffer_view_ = nullptr;
  ID3D11Texture2D* depth_buffer_ = nullptr;
  ID3D11DepthStencilView* depth_buffer_view_ = nullptr;

  ID3D11BlendState* current_blend_state_ = nullptr;
  ID3D11DepthStencilState* current_depth_stencil_state_ = nullptr;
  ID3D11RasterizerState* current_rasterizer_state_ = nullptr;

  ID3D11Query* occlusion_query_ = nullptr;
  HANDLE swap_chain_retrace_object_ = nullptr;
};

}  // namespace renderer
