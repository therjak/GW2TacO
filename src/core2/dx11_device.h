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

class CCoreDX11Device : public CCoreDevice {
 public:
  CCoreDX11Device();
  ~CCoreDX11Device() override;
  ID3D11Device* GetDevice() { return d3d_device_; }
  ID3D11DeviceContext* GetDeviceContext() { return d3d_device_context_; }
  CoreDeviceApi GetAPIType() override { return CoreDeviceApi::kDx11; }

  // this initializer will change to accommodate multiple platforms at once once
  // we get to that point:
  bool Initialize(CCoreWindowHandler* window,
                  const int32_t sample_count = 0) override;

  bool DeviceOk() override;
  bool IsWindowed() override;
  void Resize(const int32_t x_res, const int32_t y_res) override;
  void SetFullScreenMode(const bool full_screen, const int32_t x_res,
                         const int32_t y_res) override;

  ID3D11Texture2D* GetBackBuffer();

  //////////////////////////////////////////////////////////////////////////
  // texture functions

  std::unique_ptr<CCoreTexture2D> CreateTexture2D(
      const int32_t x_res, const int32_t y_res, const uint8_t* data,
      const char bytes_per_pixel = 4,
      const CoreFormat format = CoreFormat::kA8R8G8B8,
      const bool render_target = false) override;
  std::unique_ptr<CCoreTexture2D> CreateTexture2D(const uint8_t* data,
                                                  const int32_t size) override;

  //////////////////////////////////////////////////////////////////////////
  // vertexbuffer functions

  std::unique_ptr<CCoreVertexBuffer> CreateVertexBuffer(
      const uint8_t* data, const int32_t size) override;
  std::unique_ptr<CCoreVertexBuffer> CreateVertexBufferDynamic(
      const int32_t size) override;

  //////////////////////////////////////////////////////////////////////////
  // indexbuffer functions

  std::unique_ptr<CCoreIndexBuffer> CreateIndexBuffer(
      const int32_t index_count, const int32_t index_size = 2) override;

  //////////////////////////////////////////////////////////////////////////
  // vertexformat functions

  std::unique_ptr<CCoreVertexFormat> CreateVertexFormat(
      const std::vector<CoreVertexAttribute>& attributes,
      CCoreVertexShader* vs = nullptr) override;

  //////////////////////////////////////////////////////////////////////////
  // shader functions

  std::unique_ptr<CCoreVertexShader> CreateVertexShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<CCorePixelShader> CreatePixelShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<CCoreVertexShader> CreateVertexShaderFromBlob(
      uint8_t* code, int32_t code_size) override;
  std::unique_ptr<CCorePixelShader> CreatePixelShaderFromBlob(
      uint8_t* code, int32_t code_size) override;
  std::unique_ptr<CCoreGeometryShader> CreateGeometryShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<CCoreDomainShader> CreateDomainShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<CCoreHullShader> CreateHullShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<CCoreComputeShader> CreateComputeShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<CCoreVertexShader> CreateVertexShader() override;
  std::unique_ptr<CCorePixelShader> CreatePixelShader() override;
  std::unique_ptr<CCoreGeometryShader> CreateGeometryShader() override;
  std::unique_ptr<CCoreDomainShader> CreateDomainShader() override;
  std::unique_ptr<CCoreHullShader> CreateHullShader() override;
  std::unique_ptr<CCoreComputeShader> CreateComputeShader() override;
  void SetShaderConstants(const CCoreConstantBuffer* buffers) override;
  std::unique_ptr<CCoreConstantBuffer> CreateConstantBuffer() override;

  std::unique_ptr<CCoreBlendState> CreateBlendState() override;
  std::unique_ptr<CCoreDepthStencilState> CreateDepthStencilState() override;
  std::unique_ptr<CCoreRasterizerState> CreateRasterizerState() override;
  std::unique_ptr<CCoreSamplerState> CreateSamplerState() override;

  bool SetRenderTarget(CCoreTexture2D* rt) override;

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

  bool SetViewport(math::CRect viewport) override;
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
  bool ApplyRenderState(const CoreSampler sampler,
                        const CoreRenderState render_state,
                        const CoreRenderStateValue value) override;
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
