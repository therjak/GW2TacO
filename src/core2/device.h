#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "src/base/color.h"
#include "src/base/rectangle.h"
#include "src/core2/constant_buffer.h"
#include "src/core2/enums.h"
#include "src/core2/index_buffer.h"
#include "src/core2/render_state.h"
#include "src/core2/vertex_buffer.h"
#include "src/core2/vertex_format.h"
#include "src/core2/window_handler.h"

import xml;

namespace renderer {

class CCoreDevice {
  friend class CCoreResource;

 public:
  CCoreDevice();
  virtual ~CCoreDevice();
  virtual CoreDeviceApi GetAPIType() = 0;

  bool ApplyRequestedRenderState();

  virtual bool Initialize(CCoreWindowHandler* window,
                          const int32_t sample_count = 0) = 0;

  virtual bool DeviceOk() = 0;
  virtual bool IsWindowed() = 0;
  virtual void Resize(const int32_t x_res, const int32_t y_res) = 0;
  virtual void SetFullScreenMode(const bool full_screen, const int32_t x_res,
                                 const int32_t y_res) = 0;

  virtual std::unique_ptr<CCoreTexture2D> CreateTexture2D(
      const int32_t x_res, const int32_t y_res, const uint8_t* data,
      const char bytes_per_pixel = 4,
      const CoreFormat format = CoreFormat::kA8R8G8B8,
      const bool render_target = false) = 0;
  virtual std::unique_ptr<CCoreTexture2D> CreateTexture2D(
      const uint8_t* data, const int32_t size) = 0;

  virtual std::unique_ptr<CCoreVertexBuffer> CreateVertexBuffer(
      const uint8_t* data, const int32_t size) = 0;
  virtual std::unique_ptr<CCoreVertexBuffer> CreateVertexBufferDynamic(
      const int32_t size) = 0;

  virtual std::unique_ptr<CCoreIndexBuffer> CreateIndexBuffer(
      const int32_t index_count, const int32_t index_size = 2) = 0;

  virtual std::unique_ptr<CCoreVertexFormat> CreateVertexFormat(
      const std::vector<CoreVertexAttribute>& attributes,
      CCoreVertexShader* vs = nullptr) = 0;

  bool SetRenderState(CCoreRasterizerState* rasterizer_state);
  bool SetRenderState(CCoreBlendState* blend_state);
  bool SetRenderState(CCoreDepthStencilState* depth_stencil_state);
  bool SetVertexShader(CCoreVertexShader* shader);
  bool SetPixelShader(CCorePixelShader* shader);
  bool SetGeometryShader(CCoreGeometryShader* shader);
  bool SetHullShader(CCoreHullShader* shader);
  bool SetDomainShader(CCoreDomainShader* shader);
  bool SetSamplerState(CoreSampler sampler, CCoreSamplerState* sampler_state);
  bool SetTexture(CoreSampler sampler, CCoreTexture* texture);
  bool SetIndexBuffer(CCoreIndexBuffer* index_buffer);
  bool SetVertexBuffer(CCoreVertexBuffer* vertex_buffer, uint32_t offset);
  bool SetVertexFormat(CCoreVertexFormat* vertex_format);
  int32_t GetVertexFormatSize();
  CCoreTexture* GetTexture(CoreSampler sampler);

  virtual bool SetRenderTarget(CCoreTexture2D* rt) = 0;
  virtual bool SetViewport(math::CRect viewport) = 0;

  virtual std::unique_ptr<CCoreVertexShader> CreateVertexShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) = 0;
  virtual std::unique_ptr<CCorePixelShader> CreatePixelShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) = 0;
  virtual std::unique_ptr<CCoreVertexShader> CreateVertexShaderFromBlob(
      uint8_t* code, int32_t code_size) = 0;
  virtual std::unique_ptr<CCorePixelShader> CreatePixelShaderFromBlob(
      uint8_t* code, int32_t code_size) = 0;
  virtual std::unique_ptr<CCoreGeometryShader> CreateGeometryShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) = 0;
  virtual std::unique_ptr<CCoreDomainShader> CreateDomainShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) = 0;
  virtual std::unique_ptr<CCoreHullShader> CreateHullShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) = 0;
  virtual std::unique_ptr<CCoreComputeShader> CreateComputeShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) = 0;
  virtual std::unique_ptr<CCoreVertexShader> CreateVertexShader() = 0;
  virtual std::unique_ptr<CCorePixelShader> CreatePixelShader() = 0;
  virtual std::unique_ptr<CCoreGeometryShader> CreateGeometryShader() = 0;
  virtual std::unique_ptr<CCoreDomainShader> CreateDomainShader() = 0;
  virtual std::unique_ptr<CCoreHullShader> CreateHullShader() = 0;
  virtual std::unique_ptr<CCoreComputeShader> CreateComputeShader() = 0;
  virtual void SetShaderConstants(const CCoreConstantBuffer* buffers) = 0;
  virtual std::unique_ptr<CCoreConstantBuffer> CreateConstantBuffer() = 0;

  virtual std::unique_ptr<CCoreBlendState> CreateBlendState() = 0;
  virtual std::unique_ptr<CCoreDepthStencilState> CreateDepthStencilState() = 0;
  virtual std::unique_ptr<CCoreRasterizerState> CreateRasterizerState() = 0;
  virtual std::unique_ptr<CCoreSamplerState> CreateSamplerState() = 0;

  virtual bool BeginScene() = 0;
  virtual bool EndScene() = 0;
  virtual bool Clear(const bool clear_pixels = true,
                     const bool clear_depth = true,
                     const CColor& color = CColor(), const float depth = 1,
                     const int32_t stencil = 0) = 0;
  virtual bool Flip(bool vsync = true) = 0;
  virtual bool DrawIndexedTriangles(int32_t count, int32_t num_vertices) = 0;
  virtual bool DrawIndexedLines(int32_t count, int32_t num_vertices) = 0;
  virtual bool DrawTriangles(int32_t count) = 0;
  virtual bool DrawLines(int32_t count) = 0;

  virtual void ForceStateReset() = 0;

  virtual void TakeScreenShot(std::string_view filename) = 0;

  virtual void InitializeDebugAPI() = 0;
  virtual void CaptureCurrentFrame() = 0;

  virtual float GetUVOffset() { return 0; }

  virtual void BeginOcclusionQuery() = 0;
  virtual bool EndOcclusionQuery() = 0;
  virtual void WaitRetrace() = 0;

 protected:
  void ResetDevice();
  virtual void ResetPrivateResources() = 0;
  virtual bool InitAPI(const HWND window_handle, const bool full_screen,
                       const int32_t x_res, const int32_t y_res,
                       const int32_t sample_count = 0,
                       const int32_t refresh_rate = 60) = 0;

  bool ApplyTextureToSampler(const CoreSampler sampler, CCoreTexture* texture);
  bool ApplyVertexShader(CCoreVertexShader* shader);
  bool ApplyGeometryShader(CCoreGeometryShader* shader);
  bool ApplyHullShader(CCoreHullShader* shader);
  bool ApplyDomainShader(CCoreDomainShader* shader);
  bool ApplyComputeShader(CCoreComputeShader* shader);
  bool ApplyPixelShader(CCorePixelShader* shader);
  bool ApplyVertexFormat(CCoreVertexFormat* vertex_format);
  bool ApplyIndexBuffer(CCoreIndexBuffer* idx_buffer);
  bool ApplyVertexBuffer(CCoreVertexBuffer* vx_buffer, uint32_t offset);
  virtual bool ApplyRenderState(const CoreSampler sampler,
                                const CoreRenderState render_state,
                                const CoreRenderStateValue value) = 0;
  virtual bool SetNoVertexBuffer() = 0;

  virtual bool CommitRenderStates() = 0;
  bool CreateDefaultRenderStates();

  CCoreWindowHandler* window_ = nullptr;
  std::unordered_map<CoreRenderStateId, CoreRenderStateValue>
      current_render_state_;
  std::unordered_map<CoreRenderStateId, CoreRenderStateValue>
      requested_render_state_;

  CCoreVertexBuffer *current_vertex_buffer_ = nullptr,
                    *requested_vertex_buffer_ = nullptr;
  uint32_t current_vertex_buffer_offset_ = 0,
           requested_vertex_buffer_offset_ = 0;
  int32_t current_vertex_format_size_ = 0;

  std::unique_ptr<CCoreBlendState> default_blend_state_;
  std::unique_ptr<CCoreDepthStencilState> default_depth_stencil_state_;
  std::unique_ptr<CCoreRasterizerState> default_rasterizer_state_;

 private:
  void RemoveResource(CCoreResource* resource);
  void AddResource(CCoreResource* resource);

  std::vector<CCoreResource*> resources_;
};

}  // namespace renderer
