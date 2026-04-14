#include "src/core2/device.h"

#include <algorithm>

#include "src/core2/resource.h"

namespace renderer {

const RenderStateId IdFromRenderState(const RenderState state,
                                      const Sampler sampler) {
  return (static_cast<uint32_t>(state) << 16) + static_cast<uint32_t>(sampler);
}

void RenderStateFromId(const RenderStateId id, RenderState& state,
                       Sampler& sampler) {
  state = static_cast<RenderState>(id >> 16);
  sampler = static_cast<Sampler>(id & 0xffff);
}

void Device::AddResource(Resource* resource) { resources_.push_back(resource); }

void Device::RemoveResource(Resource* resource) {
  resources_.erase(std::remove(resources_.begin(), resources_.end(), resource),
                   resources_.end());
}

Device::Device() = default;

Device::~Device() {
  default_rasterizer_state_.reset();
  default_blend_state_.reset();
  default_depth_stencil_state_.reset();

  // remove remaining (leaked) resources:
  for (auto r : resources_) {
    delete r;
  }
  resources_.clear();
}

void Device::ResetDevice() {
  for (auto& r : resources_) r->OnDeviceLost();

  ResetPrivateResources();

  for (auto& r : resources_) r->OnDeviceReset();

  // reload render state
  requested_render_state_.merge(current_render_state_);
  current_render_state_.clear();
}

bool Device::ApplyRequestedRenderState() {
  for (const auto& x : requested_render_state_) {
    const RenderStateId id = x.first;
    RenderStateValue value = x.second;

    if (current_render_state_.find(id) == current_render_state_.end() ||
        current_render_state_[id] != value) {
      RenderState rs;
      Sampler sampler;
      RenderStateFromId(id, rs, sampler);
      if (!ApplyRenderState(sampler, rs, value)) return false;
      current_render_state_[id] = value;
    }
  }

  if (current_vertex_buffer_ != requested_vertex_buffer_ ||
      current_vertex_buffer_offset_ != requested_vertex_buffer_offset_) {
    if (!requested_vertex_buffer_) {
      if (!SetNoVertexBuffer()) return false;
      requested_vertex_buffer_ = nullptr;
      requested_vertex_buffer_offset_ = 0;
    } else {
      if (!ApplyVertexBuffer(requested_vertex_buffer_,
                             requested_vertex_buffer_offset_)) {
        return false;
      }
    }
  }

  requested_render_state_.clear();

  return CommitRenderStates();
}

bool Device::ApplyTextureToSampler(const Sampler sampler, Texture* texture) {
  if (!texture) return false;
  return texture->SetToSampler(sampler);
}

bool Device::ApplyVertexShader(VertexShader* shader) {
  if (!shader) return false;
  return shader->Apply();
}

bool Device::ApplyGeometryShader(GeometryShader* shader) {
  if (!shader) return false;
  return shader->Apply();
}

bool Device::ApplyHullShader(HullShader* shader) {
  if (!shader) return false;
  return shader->Apply();
}

bool Device::ApplyDomainShader(DomainShader* shader) {
  if (!shader) return false;
  return shader->Apply();
}

bool Device::ApplyComputeShader(ComputeShader* shader) {
  if (!shader) return false;
  return shader->Apply();
}

bool Device::ApplyPixelShader(PixelShader* shader) {
  if (!shader) return false;
  return shader->Apply();
}

bool Device::ApplyVertexFormat(VertexFormat* vertex_format) {
  if (!vertex_format) return false;
  return vertex_format->Apply();
}

bool Device::ApplyIndexBuffer(IndexBuffer* idx_buffer) {
  if (!idx_buffer) return false;
  return idx_buffer->Apply();
}

bool Device::ApplyVertexBuffer(VertexBuffer* vx_buffer, uint32_t offset) {
  if (!vx_buffer) return false;
  return vx_buffer->Apply(offset);
}

bool Device::SetSamplerState(Sampler sampler, SamplerState* sampler_state) {
  requested_render_state_[IdFromRenderState(RenderState::kSamplerState,
                                            sampler)]
      .sampler_state = sampler_state;
  return true;
}

bool Device::SetRenderState(RasterizerState* rasterizer_state) {
  requested_render_state_[IdFromRenderState(RenderState::kRasterizerState,
                                            static_cast<Sampler>(0))]
      .rasterizer_state = rasterizer_state;
  return true;
}

bool Device::SetRenderState(BlendState* blend_state) {
  requested_render_state_[IdFromRenderState(RenderState::kBlendState,
                                            static_cast<Sampler>(0))]
      .blend_state = blend_state;
  return true;
}

bool Device::SetRenderState(DepthStencilState* depth_stencil_state) {
  requested_render_state_[IdFromRenderState(RenderState::kDepthStencilState,
                                            static_cast<Sampler>(0))]
      .depth_stencil_state = depth_stencil_state;
  return true;
}

bool Device::SetTexture(Sampler sampler, Texture* texture) {
  requested_render_state_[IdFromRenderState(RenderState::kTexture, sampler)]
      .texture = texture;
  return true;
}

bool Device::SetVertexShader(VertexShader* shader) {
  requested_render_state_[IdFromRenderState(RenderState::kVertexShader,
                                            static_cast<Sampler>(0))]
      .vertex_shader = shader;
  return true;
}

bool Device::SetPixelShader(PixelShader* shader) {
  requested_render_state_[IdFromRenderState(RenderState::kPixelShader,
                                            static_cast<Sampler>(0))]
      .pixel_shader = shader;
  return true;
}

bool Device::SetGeometryShader(GeometryShader* shader) {
  requested_render_state_[IdFromRenderState(RenderState::kGeometryShader,
                                            static_cast<Sampler>(0))]
      .geometry_shader = shader;
  return true;
}

bool Device::SetHullShader(HullShader* shader) {
  requested_render_state_[IdFromRenderState(RenderState::kHullShader,
                                            static_cast<Sampler>(0))]
      .hull_shader = shader;
  return true;
}

bool Device::SetDomainShader(DomainShader* shader) {
  requested_render_state_[IdFromRenderState(RenderState::kDomainShader,
                                            static_cast<Sampler>(0))]
      .domain_shader = shader;
  return true;
}

bool Device::SetVertexBuffer(VertexBuffer* vertex_buffer, uint32_t offset) {
  requested_vertex_buffer_ = vertex_buffer;
  requested_vertex_buffer_offset_ = offset;
  return true;
}

bool Device::SetIndexBuffer(IndexBuffer* index_buffer) {
  requested_render_state_[IdFromRenderState(RenderState::kIndexBuffer,
                                            static_cast<Sampler>(0))]
      .index_buffer = index_buffer;
  return true;
}

bool Device::SetVertexFormat(VertexFormat* vertex_format) {
  requested_render_state_[IdFromRenderState(RenderState::kVertexFormat,
                                            static_cast<Sampler>(0))]
      .vertex_format = vertex_format;
  return true;
}

int32_t Device::GetVertexFormatSize() { return current_vertex_format_size_; }

Texture* Device::GetTexture(Sampler sampler) {
  if (requested_render_state_.find(IdFromRenderState(
          RenderState::kTexture, sampler)) != requested_render_state_.end()) {
    return requested_render_state_[IdFromRenderState(RenderState::kTexture,
                                                     sampler)]
        .texture;
  }
  if (current_render_state_.find(IdFromRenderState(
          RenderState::kTexture, sampler)) != current_render_state_.end()) {
    return current_render_state_[IdFromRenderState(RenderState::kTexture,
                                                   sampler)]
        .texture;
  }
  return nullptr;
}

bool Device::CreateDefaultRenderStates() {
  bool success = true;

  default_blend_state_ = CreateBlendState();
  default_blend_state_->SetBlendEnable(0, true);
  default_blend_state_->SetSrcBlend(0, BlendFactor::kSrcAlpha);
  default_blend_state_->SetDestBlend(0, BlendFactor::kInvSrcAlpha);
  success |= default_blend_state_->Apply();

  default_depth_stencil_state_ = CreateDepthStencilState();
  default_depth_stencil_state_->SetDepthEnable(true);
  default_depth_stencil_state_->SetZWriteEnable(true);
  default_depth_stencil_state_->SetDepthFunc(ComparisonFunction::kLessEqual);
  success |= default_depth_stencil_state_->Apply();

  default_rasterizer_state_ = CreateRasterizerState();
  success |= default_rasterizer_state_->Apply();

  return success;
}

}  // namespace renderer
