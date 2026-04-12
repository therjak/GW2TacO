#include "src/core2/device.h"

#include <algorithm>

#include "src/core2/resource.h"

namespace renderer {

const CoreRenderStateId IdFromRenderState(const CoreRenderState state,
                                          const CoreSampler sampler) {
  return (static_cast<uint32_t>(state) << 16) + static_cast<uint32_t>(sampler);
}

void RenderStateFromId(const CoreRenderStateId id, CoreRenderState& state,
                       CoreSampler& sampler) {
  state = static_cast<CoreRenderState>(id >> 16);
  sampler = static_cast<CoreSampler>(id & 0xffff);
}

void CCoreDevice::AddResource(CCoreResource* resource) {
  resources_.push_back(resource);
}

void CCoreDevice::RemoveResource(CCoreResource* resource) {
  resources_.erase(std::remove(resources_.begin(), resources_.end(), resource),
                  resources_.end());
}

CCoreDevice::CCoreDevice() = default;

CCoreDevice::~CCoreDevice() {
  default_rasterizer_state_.reset();
  default_blend_state_.reset();
  default_depth_stencil_state_.reset();

  // remove remaining (leaked) resources:
  for (auto r : resources_) {
    delete r;
  }
  resources_.clear();
}

void CCoreDevice::ResetDevice() {
  for (auto& r : resources_) r->OnDeviceLost();

  ResetPrivateResources();

  for (auto& r : resources_) r->OnDeviceReset();

  // reload render state
  requested_render_state_.merge(current_render_state_);
  current_render_state_.clear();
}

bool CCoreDevice::ApplyRequestedRenderState() {
  for (const auto& x : requested_render_state_) {
    const CoreRenderStateId id = x.first;
    CoreRenderStateValue value = x.second;

    if (current_render_state_.find(id) == current_render_state_.end() ||
        current_render_state_[id] != value) {
      CoreRenderState rs;
      CoreSampler sampler;
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

bool CCoreDevice::ApplyTextureToSampler(const CoreSampler sampler,
                                        CCoreTexture* texture) {
  if (!texture) return false;
  return texture->SetToSampler(sampler);
}

bool CCoreDevice::ApplyVertexShader(CCoreVertexShader* shader) {
  if (!shader) return false;
  return shader->Apply();
}

bool CCoreDevice::ApplyGeometryShader(CCoreGeometryShader* shader) {
  if (!shader) return false;
  return shader->Apply();
}

bool CCoreDevice::ApplyHullShader(CCoreHullShader* shader) {
  if (!shader) return false;
  return shader->Apply();
}

bool CCoreDevice::ApplyDomainShader(CCoreDomainShader* shader) {
  if (!shader) return false;
  return shader->Apply();
}

bool CCoreDevice::ApplyComputeShader(CCoreComputeShader* shader) {
  if (!shader) return false;
  return shader->Apply();
}

bool CCoreDevice::ApplyPixelShader(CCorePixelShader* shader) {
  if (!shader) return false;
  return shader->Apply();
}

bool CCoreDevice::ApplyVertexFormat(CCoreVertexFormat* vertex_format) {
  if (!vertex_format) return false;
  return vertex_format->Apply();
}

bool CCoreDevice::ApplyIndexBuffer(CCoreIndexBuffer* idx_buffer) {
  if (!idx_buffer) return false;
  return idx_buffer->Apply();
}

bool CCoreDevice::ApplyVertexBuffer(CCoreVertexBuffer* vx_buffer,
                                    uint32_t offset) {
  if (!vx_buffer) return false;
  return vx_buffer->Apply(offset);
}

bool CCoreDevice::SetSamplerState(CoreSampler sampler,
                                  CCoreSamplerState* sampler_state) {
  requested_render_state_[IdFromRenderState(CoreRenderState::kSamplerState,
                                         sampler)]
      .sampler_state = sampler_state;
  return true;
}

bool CCoreDevice::SetRenderState(CCoreRasterizerState* rasterizer_state) {
  requested_render_state_[IdFromRenderState(CoreRenderState::kRasterizerState,
                                         static_cast<CoreSampler>(0))]
      .rasterizer_state = rasterizer_state;
  return true;
}

bool CCoreDevice::SetRenderState(CCoreBlendState* blend_state) {
  requested_render_state_[IdFromRenderState(CoreRenderState::kBlendState,
                                         static_cast<CoreSampler>(0))]
      .blend_state = blend_state;
  return true;
}

bool CCoreDevice::SetRenderState(CCoreDepthStencilState* depth_stencil_state) {
  requested_render_state_[IdFromRenderState(CoreRenderState::kDepthStencilState,
                                         static_cast<CoreSampler>(0))]
      .depth_stencil_state = depth_stencil_state;
  return true;
}

bool CCoreDevice::SetTexture(CoreSampler sampler, CCoreTexture* texture) {
  requested_render_state_[IdFromRenderState(CoreRenderState::kTexture, sampler)]
      .texture = texture;
  return true;
}

bool CCoreDevice::SetVertexShader(CCoreVertexShader* shader) {
  requested_render_state_[IdFromRenderState(CoreRenderState::kVertexShader,
                                         static_cast<CoreSampler>(0))]
      .vertex_shader = shader;
  return true;
}

bool CCoreDevice::SetPixelShader(CCorePixelShader* shader) {
  requested_render_state_[IdFromRenderState(CoreRenderState::kPixelShader,
                                         static_cast<CoreSampler>(0))]
      .pixel_shader = shader;
  return true;
}

bool CCoreDevice::SetGeometryShader(CCoreGeometryShader* shader) {
  requested_render_state_[IdFromRenderState(CoreRenderState::kGeometryShader,
                                         static_cast<CoreSampler>(0))]
      .geometry_shader = shader;
  return true;
}

bool CCoreDevice::SetHullShader(CCoreHullShader* shader) {
  requested_render_state_[IdFromRenderState(CoreRenderState::kHullShader,
                                         static_cast<CoreSampler>(0))]
      .hull_shader = shader;
  return true;
}

bool CCoreDevice::SetDomainShader(CCoreDomainShader* shader) {
  requested_render_state_[IdFromRenderState(CoreRenderState::kDomainShader,
                                         static_cast<CoreSampler>(0))]
      .domain_shader = shader;
  return true;
}

bool CCoreDevice::SetVertexBuffer(CCoreVertexBuffer* vertex_buffer,
                                  uint32_t offset) {
  requested_vertex_buffer_ = vertex_buffer;
  requested_vertex_buffer_offset_ = offset;
  return true;
}

bool CCoreDevice::SetIndexBuffer(CCoreIndexBuffer* index_buffer) {
  requested_render_state_[IdFromRenderState(CoreRenderState::kIndexBuffer,
                                         static_cast<CoreSampler>(0))]
      .index_buffer = index_buffer;
  return true;
}

bool CCoreDevice::SetVertexFormat(CCoreVertexFormat* vertex_format) {
  requested_render_state_[IdFromRenderState(CoreRenderState::kVertexFormat,
                                         static_cast<CoreSampler>(0))]
      .vertex_format = vertex_format;
  return true;
}

int32_t CCoreDevice::GetVertexFormatSize() { return current_vertex_format_size_; }

CCoreTexture* CCoreDevice::GetTexture(CoreSampler sampler) {
  if (requested_render_state_.find(IdFromRenderState(
          CoreRenderState::kTexture, sampler)) != requested_render_state_.end()) {
    return requested_render_state_[IdFromRenderState(CoreRenderState::kTexture,
                                                   sampler)]
        .texture;
  }
  if (current_render_state_.find(IdFromRenderState(
          CoreRenderState::kTexture, sampler)) != current_render_state_.end()) {
    return current_render_state_[IdFromRenderState(CoreRenderState::kTexture,
                                                 sampler)]
        .texture;
  }
  return nullptr;
}

bool CCoreDevice::CreateDefaultRenderStates() {
  bool success = true;

  default_blend_state_ = CreateBlendState();
  default_blend_state_->SetBlendEnable(0, true);
  default_blend_state_->SetSrcBlend(0, CoreBlendFactor::kSrcAlpha);
  default_blend_state_->SetDestBlend(0, CoreBlendFactor::kInvSrcAlpha);
  success |= default_blend_state_->Apply();

  default_depth_stencil_state_ = CreateDepthStencilState();
  default_depth_stencil_state_->SetDepthEnable(true);
  default_depth_stencil_state_->SetZWriteEnable(true);
  default_depth_stencil_state_->SetDepthFunc(CoreComparisonFunction::kLessEqual);
  success |= default_depth_stencil_state_->Apply();

  default_rasterizer_state_ = CreateRasterizerState();
  success |= default_rasterizer_state_->Apply();

  return success;
}

}  // namespace renderer
