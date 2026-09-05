#include "src/core2/vk_device.h"

#include <memory>
#include <string>
#include <string_view>

#include "src/base/logger.h"
#include "src/core2/vk_constant_buffer.h"
#include "src/core2/vk_index_buffer.h"
#include "src/core2/vk_render_state.h"
#include "src/core2/vk_shader.h"
#include "src/core2/vk_texture.h"
#include "src/core2/vk_vertex_buffer.h"
#include "src/core2/vk_vertex_format.h"

import math;

namespace renderer {

using math::Rect;

VulkanDevice::VulkanDevice() = default;
VulkanDevice::~VulkanDevice() = default;

void VulkanDevice::ResetPrivateResources() {}

bool VulkanDevice::InitAPI(const HWND /*window_handle*/,
                           const bool /*full_screen*/, const int32_t /*x_res*/,
                           const int32_t /*y_res*/,
                           const int32_t /*sample_count*/,
                           const int32_t /*refresh_rate*/) {
  Log_Err("[vk] VulkanDevice::InitAPI not yet implemented (Phase 1 stub).");
  return false;
}

bool VulkanDevice::Initialize(WindowHandler* window,
                              const int32_t sample_count) {
  window_ = window;
  return InitAPI(window_->GetHandle(),
                 window_->GetInitParameters()->full_screen_,
                 window_->GetXRes(), window_->GetYRes(), sample_count, 60);
}

bool VulkanDevice::DeviceOk() { return false; }
bool VulkanDevice::IsWindowed() { return true; }

void VulkanDevice::Resize(const int32_t /*x_res*/, const int32_t /*y_res*/) {}

void VulkanDevice::SetFullScreenMode(const bool /*full_screen*/,
                                     const int32_t /*x_res*/,
                                     const int32_t /*y_res*/) {}

//////////////////////////////////////////////////////////////////////////
// Texture

std::unique_ptr<Texture2D> VulkanDevice::CreateTexture2D(
    const int32_t x_res, const int32_t y_res, const uint8_t* data,
    const char bytes_per_pixel, const Format format,
    const bool render_target) {
  auto result = std::make_unique<VkTexture2D>(this);
  if (!result->Create(x_res, y_res, data, bytes_per_pixel, format,
                      render_target)) {
    result.reset();
  }
  return result;
}

std::unique_ptr<Texture2D> VulkanDevice::CreateTexture2D(const uint8_t* data,
                                                         const int32_t size) {
  auto result = std::make_unique<VkTexture2D>(this);
  if (!result->Create(data, size)) {
    result.reset();
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////
// Vertex / Index buffers

std::unique_ptr<VertexBuffer> VulkanDevice::CreateVertexBuffer(
    const uint8_t* data, const int32_t size) {
  auto result = std::make_unique<VkVertexBuffer>(this);
  if (!result->Create(data, size)) {
    result.reset();
  }
  return result;
}

std::unique_ptr<VertexBuffer> VulkanDevice::CreateVertexBufferDynamic(
    const int32_t size) {
  auto result = std::make_unique<VkVertexBuffer>(this);
  if (!result->CreateDynamic(size)) {
    result.reset();
  }
  return result;
}

std::unique_ptr<IndexBuffer> VulkanDevice::CreateIndexBuffer(
    const int32_t index_count, const int32_t index_size) {
  auto result = std::make_unique<VkIndexBuffer>(this);
  if (!result->Create(index_count, index_size)) {
    result.reset();
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////
// Vertex format

std::unique_ptr<VertexFormat> VulkanDevice::CreateVertexFormat(
    const std::vector<VertexAttribute>& attributes, VertexShader* vs) {
  auto result = std::make_unique<VkVertexFormat>(this);
  if (!result->Create(attributes, vs)) {
    result.reset();
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////
// Shaders — source compilation

std::unique_ptr<VertexShader> VulkanDevice::CreateVertexShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) *error = "";
  if (!code || !code_size || !entry_function || !shader_version) return {};
  auto shader = std::make_unique<VkVertexShader>(this);
  shader->SetCode(code, entry_function, shader_version);
  if (!shader->CompileAndCreate(error)) shader.reset();
  return shader;
}

std::unique_ptr<PixelShader> VulkanDevice::CreatePixelShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) *error = "";
  if (!code || !code_size || !entry_function || !shader_version) return {};
  auto shader = std::make_unique<VkPixelShader>(this);
  shader->SetCode(code, entry_function, shader_version);
  if (!shader->CompileAndCreate(error)) shader.reset();
  return shader;
}

std::unique_ptr<GeometryShader> VulkanDevice::CreateGeometryShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) *error = "";
  if (!code || !code_size || !entry_function || !shader_version) return {};
  auto shader = std::make_unique<VkGeometryShader>(this);
  shader->SetCode(code, entry_function, shader_version);
  if (!shader->CompileAndCreate(error)) shader.reset();
  return shader;
}

std::unique_ptr<DomainShader> VulkanDevice::CreateDomainShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) *error = "";
  if (!code || !code_size || !entry_function || !shader_version) return {};
  auto shader = std::make_unique<VkDomainShader>(this);
  shader->SetCode(code, entry_function, shader_version);
  if (!shader->CompileAndCreate(error)) shader.reset();
  return shader;
}

std::unique_ptr<HullShader> VulkanDevice::CreateHullShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) *error = "";
  if (!code || !code_size || !entry_function || !shader_version) return {};
  auto shader = std::make_unique<VkHullShader>(this);
  shader->SetCode(code, entry_function, shader_version);
  if (!shader->CompileAndCreate(error)) shader.reset();
  return shader;
}

std::unique_ptr<ComputeShader> VulkanDevice::CreateComputeShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) *error = "";
  if (!code || !code_size || !entry_function || !shader_version) return {};
  auto shader = std::make_unique<VkComputeShader>(this);
  shader->SetCode(code, entry_function, shader_version);
  if (!shader->CompileAndCreate(error)) shader.reset();
  return shader;
}

//////////////////////////////////////////////////////////////////////////
// Shaders — pre-compiled blob

std::unique_ptr<VertexShader> VulkanDevice::CreateVertexShaderFromBlob(
    uint8_t* code, int32_t code_size) {
  auto shader = std::make_unique<VkVertexShader>(this);
  if (!shader->CreateFromBlob(code, code_size)) shader.reset();
  return shader;
}

std::unique_ptr<PixelShader> VulkanDevice::CreatePixelShaderFromBlob(
    uint8_t* code, int32_t code_size) {
  auto shader = std::make_unique<VkPixelShader>(this);
  if (!shader->CreateFromBlob(code, code_size)) shader.reset();
  return shader;
}

//////////////////////////////////////////////////////////////////////////
// Shaders — empty shell factory

std::unique_ptr<VertexShader> VulkanDevice::CreateVertexShader() {
  return std::make_unique<VkVertexShader>(this);
}
std::unique_ptr<PixelShader> VulkanDevice::CreatePixelShader() {
  return std::make_unique<VkPixelShader>(this);
}
std::unique_ptr<GeometryShader> VulkanDevice::CreateGeometryShader() {
  return std::make_unique<VkGeometryShader>(this);
}
std::unique_ptr<DomainShader> VulkanDevice::CreateDomainShader() {
  return std::make_unique<VkDomainShader>(this);
}
std::unique_ptr<HullShader> VulkanDevice::CreateHullShader() {
  return std::make_unique<VkHullShader>(this);
}
std::unique_ptr<ComputeShader> VulkanDevice::CreateComputeShader() {
  return std::make_unique<VkComputeShader>(this);
}

//////////////////////////////////////////////////////////////////////////
// Constant buffer / shader constants

void VulkanDevice::SetShaderConstants(const ConstantBuffer* /*buffers*/) {}

std::unique_ptr<ConstantBuffer> VulkanDevice::CreateConstantBuffer() {
  return std::make_unique<VkConstantBuffer>(this);
}

//////////////////////////////////////////////////////////////////////////
// Render state objects

std::unique_ptr<BlendState> VulkanDevice::CreateBlendState() {
  return std::make_unique<VkBlendState>(this);
}
std::unique_ptr<DepthStencilState> VulkanDevice::CreateDepthStencilState() {
  return std::make_unique<VkDepthStencilState>(this);
}
std::unique_ptr<RasterizerState> VulkanDevice::CreateRasterizerState() {
  return std::make_unique<VkRasterizerState>(this);
}
std::unique_ptr<SamplerState> VulkanDevice::CreateSamplerState() {
  return std::make_unique<VkSamplerState>(this);
}

bool VulkanDevice::SetRenderTarget(Texture2D* /*rt*/) { return false; }

bool VulkanDevice::SetViewport(const math::Rect& /*viewport*/) {
  return false;
}

//////////////////////////////////////////////////////////////////////////
// Frame lifecycle

bool VulkanDevice::BeginScene() { return false; }
bool VulkanDevice::EndScene() { return false; }

bool VulkanDevice::Clear(const bool /*clear_pixels*/,
                         const bool /*clear_depth*/, const Color& /*color*/,
                         const float /*depth*/, const int32_t /*stencil*/) {
  return false;
}

bool VulkanDevice::Flip(bool /*vsync*/) { return false; }

//////////////////////////////////////////////////////////////////////////
// Draw calls

bool VulkanDevice::DrawIndexedTriangles(int32_t /*count*/,
                                        int32_t /*num_vertices*/) {
  return false;
}
bool VulkanDevice::DrawIndexedLines(int32_t /*count*/,
                                    int32_t /*num_vertices*/) {
  return false;
}
bool VulkanDevice::DrawTriangles(int32_t /*count*/) { return false; }
bool VulkanDevice::DrawLines(int32_t /*count*/) { return false; }

//////////////////////////////////////////////////////////////////////////
// Misc

void VulkanDevice::ForceStateReset() {}
void VulkanDevice::TakeScreenShot(std::string_view /*filename*/) {}
void VulkanDevice::InitializeDebugAPI() {}
void VulkanDevice::CaptureCurrentFrame() {}

void VulkanDevice::BeginOcclusionQuery() {}
bool VulkanDevice::EndOcclusionQuery() { return false; }
void VulkanDevice::WaitRetrace() {}

//////////////////////////////////////////////////////////////////////////
// Internal render-state plumbing

bool VulkanDevice::ApplyRenderState(const Sampler /*sampler*/,
                                    const RenderState /*render_state*/,
                                    const RenderStateValue /*value*/) {
  return false;
}
bool VulkanDevice::SetNoVertexBuffer() { return false; }
bool VulkanDevice::CommitRenderStates() { return false; }

}  // namespace renderer
