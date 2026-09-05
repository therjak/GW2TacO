#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "src/base/color.h"
#include "src/core2/device.h"

import math;

namespace renderer {

// Vulkan backend for the renderer::Device abstraction.
//
// Phase 1: skeleton only — every virtual method is declared and stubbed in the
// corresponding .cpp so that the translation unit compiles cleanly.  No Vulkan
// headers are included yet; they will be added in Phase 2 when InitAPI() is
// implemented.
class VulkanDevice : public Device {
 public:
  VulkanDevice();
  ~VulkanDevice() override;

  DeviceApi GetAPIType() override { return DeviceApi::kVulkan; }

  bool Initialize(WindowHandler* window,
                  const int32_t sample_count = 0) override;

  bool DeviceOk() override;
  bool IsWindowed() override;
  void Resize(const int32_t x_res, const int32_t y_res) override;
  void SetFullScreenMode(const bool full_screen, const int32_t x_res,
                         const int32_t y_res) override;

  // Texture creation
  std::unique_ptr<Texture2D> CreateTexture2D(
      const int32_t x_res, const int32_t y_res, const uint8_t* data,
      const char bytes_per_pixel = 4, const Format format = Format::kA8R8G8B8,
      const bool render_target = false) override;
  std::unique_ptr<Texture2D> CreateTexture2D(const uint8_t* data,
                                             const int32_t size) override;

  // Buffer creation
  std::unique_ptr<VertexBuffer> CreateVertexBuffer(
      const uint8_t* data, const int32_t size) override;
  std::unique_ptr<VertexBuffer> CreateVertexBufferDynamic(
      const int32_t size) override;
  std::unique_ptr<IndexBuffer> CreateIndexBuffer(
      const int32_t index_count, const int32_t index_size = 2) override;
  std::unique_ptr<VertexFormat> CreateVertexFormat(
      const std::vector<VertexAttribute>& attributes,
      VertexShader* vs = nullptr) override;

  // Shader creation — source compilation
  std::unique_ptr<VertexShader> CreateVertexShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
  std::unique_ptr<PixelShader> CreatePixelShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) override;
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

  // Shader creation — pre-compiled blob
  std::unique_ptr<VertexShader> CreateVertexShaderFromBlob(
      uint8_t* code, int32_t code_size) override;
  std::unique_ptr<PixelShader> CreatePixelShaderFromBlob(
      uint8_t* code, int32_t code_size) override;

  // Empty-shell shader factory (no source / blob)
  std::unique_ptr<VertexShader> CreateVertexShader() override;
  std::unique_ptr<PixelShader> CreatePixelShader() override;
  std::unique_ptr<GeometryShader> CreateGeometryShader() override;
  std::unique_ptr<DomainShader> CreateDomainShader() override;
  std::unique_ptr<HullShader> CreateHullShader() override;
  std::unique_ptr<ComputeShader> CreateComputeShader() override;

  void SetShaderConstants(const ConstantBuffer* buffers) override;
  std::unique_ptr<ConstantBuffer> CreateConstantBuffer() override;

  // Render state objects
  std::unique_ptr<BlendState> CreateBlendState() override;
  std::unique_ptr<DepthStencilState> CreateDepthStencilState() override;
  std::unique_ptr<RasterizerState> CreateRasterizerState() override;
  std::unique_ptr<SamplerState> CreateSamplerState() override;

  bool SetRenderTarget(Texture2D* rt) override;
  bool SetViewport(const math::Rect& viewport) override;

  // Frame lifecycle
  bool BeginScene() override;
  bool EndScene() override;
  bool Clear(const bool clear_pixels = true, const bool clear_depth = true,
             const Color& color = Color(), const float depth = 1,
             const int32_t stencil = 0) override;
  bool Flip(bool vsync = true) override;

  // Draw calls
  bool DrawIndexedTriangles(int32_t count, int32_t num_vertices) override;
  bool DrawIndexedLines(int32_t count, int32_t num_vertices) override;
  bool DrawTriangles(int32_t count) override;
  bool DrawLines(int32_t count) override;

  void ForceStateReset() override;
  void TakeScreenShot(std::string_view filename) override;
  void InitializeDebugAPI() override;
  void CaptureCurrentFrame() override;

  void BeginOcclusionQuery() override;
  bool EndOcclusionQuery() override;
  void WaitRetrace() override;

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
};

}  // namespace renderer
