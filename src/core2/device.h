#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "src/base/color.h"
#include "src/core2/constant_buffer.h"
#include "src/core2/enums.h"
#include "src/core2/index_buffer.h"
#include "src/core2/render_state.h"
#include "src/core2/vertex_buffer.h"
#include "src/core2/vertex_format.h"
#include "src/core2/window_handler.h"

import math;
import xml;

namespace renderer {

class Device {
  friend class Resource;

 public:
  Device();
  virtual ~Device();
  virtual DeviceApi GetAPIType() = 0;

  bool ApplyRequestedRenderState();

  virtual bool Initialize(WindowHandler* window,
                          const int32_t sample_count = 0) = 0;

  virtual bool DeviceOk() = 0;
  virtual bool IsWindowed() = 0;
  virtual void Resize(const int32_t x_res, const int32_t y_res) = 0;
  virtual void SetFullScreenMode(const bool full_screen, const int32_t x_res,
                                 const int32_t y_res) = 0;

  virtual std::unique_ptr<Texture2D> CreateTexture2D(
      const int32_t x_res, const int32_t y_res, const uint8_t* data,
      const char bytes_per_pixel = 4, const Format format = Format::kA8R8G8B8,
      const bool render_target = false) = 0;
  virtual std::unique_ptr<Texture2D> CreateTexture2D(const uint8_t* data,
                                                     const int32_t size) = 0;

  virtual std::unique_ptr<VertexBuffer> CreateVertexBuffer(
      const uint8_t* data, const int32_t size) = 0;
  virtual std::unique_ptr<VertexBuffer> CreateVertexBufferDynamic(
      const int32_t size) = 0;

  virtual std::unique_ptr<IndexBuffer> CreateIndexBuffer(
      const int32_t index_count, const int32_t index_size = 2) = 0;

  virtual std::unique_ptr<VertexFormat> CreateVertexFormat(
      const std::vector<VertexAttribute>& attributes,
      VertexShader* vs = nullptr) = 0;

  bool SetRenderState(RasterizerState* rasterizer_state);
  bool SetRenderState(BlendState* blend_state);
  bool SetRenderState(DepthStencilState* depth_stencil_state);
  bool SetVertexShader(VertexShader* shader);
  bool SetPixelShader(PixelShader* shader);
  bool SetGeometryShader(GeometryShader* shader);
  bool SetHullShader(HullShader* shader);
  bool SetDomainShader(DomainShader* shader);
  bool SetSamplerState(Sampler sampler, SamplerState* sampler_state);
  bool SetTexture(Sampler sampler, Texture* texture);
  bool SetIndexBuffer(IndexBuffer* index_buffer);
  bool SetVertexBuffer(VertexBuffer* vertex_buffer, uint32_t offset);
  bool SetVertexFormat(VertexFormat* vertex_format);
  int32_t GetVertexFormatSize();
  Texture* GetTexture(Sampler sampler);

  virtual bool SetRenderTarget(Texture2D* rt) = 0;
  virtual bool SetViewport(math::Rect viewport) = 0;

  virtual std::unique_ptr<VertexShader> CreateVertexShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) = 0;
  virtual std::unique_ptr<PixelShader> CreatePixelShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) = 0;
  virtual std::unique_ptr<VertexShader> CreateVertexShaderFromBlob(
      uint8_t* code, int32_t code_size) = 0;
  virtual std::unique_ptr<PixelShader> CreatePixelShaderFromBlob(
      uint8_t* code, int32_t code_size) = 0;
  virtual std::unique_ptr<GeometryShader> CreateGeometryShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) = 0;
  virtual std::unique_ptr<DomainShader> CreateDomainShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) = 0;
  virtual std::unique_ptr<HullShader> CreateHullShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) = 0;
  virtual std::unique_ptr<ComputeShader> CreateComputeShader(
      LPCSTR code, int32_t code_size, LPCSTR entry_function,
      LPCSTR shader_version, std::string* error = nullptr) = 0;
  virtual std::unique_ptr<VertexShader> CreateVertexShader() = 0;
  virtual std::unique_ptr<PixelShader> CreatePixelShader() = 0;
  virtual std::unique_ptr<GeometryShader> CreateGeometryShader() = 0;
  virtual std::unique_ptr<DomainShader> CreateDomainShader() = 0;
  virtual std::unique_ptr<HullShader> CreateHullShader() = 0;
  virtual std::unique_ptr<ComputeShader> CreateComputeShader() = 0;
  virtual void SetShaderConstants(const ConstantBuffer* buffers) = 0;
  virtual std::unique_ptr<ConstantBuffer> CreateConstantBuffer() = 0;

  virtual std::unique_ptr<BlendState> CreateBlendState() = 0;
  virtual std::unique_ptr<DepthStencilState> CreateDepthStencilState() = 0;
  virtual std::unique_ptr<RasterizerState> CreateRasterizerState() = 0;
  virtual std::unique_ptr<SamplerState> CreateSamplerState() = 0;

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

  bool ApplyTextureToSampler(const Sampler sampler, Texture* texture);
  bool ApplyVertexShader(VertexShader* shader);
  bool ApplyGeometryShader(GeometryShader* shader);
  bool ApplyHullShader(HullShader* shader);
  bool ApplyDomainShader(DomainShader* shader);
  bool ApplyComputeShader(ComputeShader* shader);
  bool ApplyPixelShader(PixelShader* shader);
  bool ApplyVertexFormat(VertexFormat* vertex_format);
  bool ApplyIndexBuffer(IndexBuffer* idx_buffer);
  bool ApplyVertexBuffer(VertexBuffer* vx_buffer, uint32_t offset);
  virtual bool ApplyRenderState(const Sampler sampler,
                                const RenderState render_state,
                                const RenderStateValue value) = 0;
  virtual bool SetNoVertexBuffer() = 0;

  virtual bool CommitRenderStates() = 0;
  bool CreateDefaultRenderStates();

  WindowHandler* window_ = nullptr;
  std::unordered_map<RenderStateId, RenderStateValue> current_render_state_;
  std::unordered_map<RenderStateId, RenderStateValue> requested_render_state_;

  VertexBuffer *current_vertex_buffer_ = nullptr,
               *requested_vertex_buffer_ = nullptr;
  uint32_t current_vertex_buffer_offset_ = 0,
           requested_vertex_buffer_offset_ = 0;
  int32_t current_vertex_format_size_ = 0;

  std::unique_ptr<BlendState> default_blend_state_;
  std::unique_ptr<DepthStencilState> default_depth_stencil_state_;
  std::unique_ptr<RasterizerState> default_rasterizer_state_;

 private:
  void RemoveResource(Resource* resource);
  void AddResource(Resource* resource);

  std::vector<Resource*> resources_;
};

}  // namespace renderer
