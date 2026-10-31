#include "src/core2/vk_render_state.h"

namespace renderer {

//////////////////////////////////////////////////////////////////////////
// VkBlendState

VkBlendState::VkBlendState(VulkanDevice* device)
    : BlendState(device), device_(device) {}
VkBlendState::~VkBlendState() = default;

bool VkBlendState::Update() {
  auto map_blend = [](BlendFactor f) -> VkBlendFactor {
    switch (f) {
      case BlendFactor::kZero: return VK_BLEND_FACTOR_ZERO;
      case BlendFactor::kOne: return VK_BLEND_FACTOR_ONE;
      case BlendFactor::kSrcCol: return VK_BLEND_FACTOR_SRC_COLOR;
      case BlendFactor::kInvSrcCol: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
      case BlendFactor::kSrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
      case BlendFactor::kInvSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      case BlendFactor::kDstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
      case BlendFactor::kInvDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
      case BlendFactor::kDstColor: return VK_BLEND_FACTOR_DST_COLOR;
      case BlendFactor::kInvDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
      case BlendFactor::kSrcAlphaSaturate: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
      case BlendFactor::kBlendFactor: return VK_BLEND_FACTOR_CONSTANT_COLOR;
      case BlendFactor::kInvBlendFactor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
      default: return VK_BLEND_FACTOR_ZERO;
    }
  };

  auto map_op = [](BlendOp o) -> VkBlendOp {
    switch (o) {
      case BlendOp::kAdd: return VK_BLEND_OP_ADD;
      case BlendOp::kSub: return VK_BLEND_OP_SUBTRACT;
      case BlendOp::kRevSub: return VK_BLEND_OP_REVERSE_SUBTRACT;
      case BlendOp::kMin: return VK_BLEND_OP_MIN;
      case BlendOp::kMax: return VK_BLEND_OP_MAX;
      default: return VK_BLEND_OP_ADD;
    }
  };

  const auto& rt0 = render_target_blend_states_[0];
  attachment_vk_ = {};
  attachment_vk_.blendEnable = rt0.blend_enable ? VK_TRUE : VK_FALSE;
  attachment_vk_.srcColorBlendFactor = map_blend(rt0.src_blend);
  attachment_vk_.dstColorBlendFactor = map_blend(rt0.dest_blend);
  attachment_vk_.colorBlendOp = map_op(rt0.blend_op);
  attachment_vk_.srcAlphaBlendFactor = map_blend(rt0.src_blend_alpha);
  attachment_vk_.dstAlphaBlendFactor = map_blend(rt0.dest_blend_alpha);
  attachment_vk_.alphaBlendOp = map_op(rt0.blend_op_alpha);
  attachment_vk_.colorWriteMask = rt0.render_target_write_mask ? rt0.render_target_write_mask
      : (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);

  desc_vk_ = {};
  desc_vk_.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  desc_vk_.logicOpEnable = VK_FALSE;
  desc_vk_.attachmentCount = 1;
  desc_vk_.pAttachments = &attachment_vk_;
  dirty_ = false;
  return true;
}

bool VkBlendState::Apply() {
  Update();
  return true;
}

//////////////////////////////////////////////////////////////////////////
// VkDepthStencilState

VkDepthStencilState::VkDepthStencilState(VulkanDevice* device)
    : DepthStencilState(device), device_(device) {}
VkDepthStencilState::~VkDepthStencilState() = default;

bool VkDepthStencilState::Update() {
  desc_vk_ = {};
  desc_vk_.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  desc_vk_.depthTestEnable = depth_enable_ ? VK_TRUE : VK_FALSE;
  desc_vk_.depthWriteEnable = z_write_enable_ ? VK_TRUE : VK_FALSE;
  
  switch (depth_func_) {
    case ComparisonFunction::kNever: desc_vk_.depthCompareOp = VK_COMPARE_OP_NEVER; break;
    case ComparisonFunction::kLess: desc_vk_.depthCompareOp = VK_COMPARE_OP_LESS; break;
    case ComparisonFunction::kEqual: desc_vk_.depthCompareOp = VK_COMPARE_OP_EQUAL; break;
    case ComparisonFunction::kLessEqual: desc_vk_.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; break;
    case ComparisonFunction::kGreater: desc_vk_.depthCompareOp = VK_COMPARE_OP_GREATER; break;
    case ComparisonFunction::kNotEqual: desc_vk_.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL; break;
    case ComparisonFunction::kGreaterEqual: desc_vk_.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL; break;
    case ComparisonFunction::kAlways: desc_vk_.depthCompareOp = VK_COMPARE_OP_ALWAYS; break;
    default: desc_vk_.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; break;
  }
  
  desc_vk_.depthBoundsTestEnable = VK_FALSE;
  desc_vk_.stencilTestEnable = VK_FALSE;
  dirty_ = false;
  return true;
}

bool VkDepthStencilState::Apply() {
  Update();
  return true;
}

//////////////////////////////////////////////////////////////////////////
// VkRasterizerState

VkRasterizerState::VkRasterizerState(VulkanDevice* device)
    : RasterizerState(device), device_(device) {}
VkRasterizerState::~VkRasterizerState() = default;

bool VkRasterizerState::Update() {
  desc_vk_ = {};
  desc_vk_.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  desc_vk_.depthClampEnable = VK_FALSE;
  desc_vk_.rasterizerDiscardEnable = VK_FALSE;
  
  switch (fill_mode_) {
    case FillMode::kEdges: desc_vk_.polygonMode = VK_POLYGON_MODE_LINE; break;
    case FillMode::kPoints: desc_vk_.polygonMode = VK_POLYGON_MODE_POINT; break;
    case FillMode::kSolid:
    default: desc_vk_.polygonMode = VK_POLYGON_MODE_FILL; break;
  }
  
  switch (cull_mode_) {
    case CullMode::kNone: desc_vk_.cullMode = VK_CULL_MODE_NONE; break;
    case CullMode::kCw: desc_vk_.cullMode = VK_CULL_MODE_FRONT_BIT; break;
    case CullMode::kCcw: desc_vk_.cullMode = VK_CULL_MODE_BACK_BIT; break;
    default: desc_vk_.cullMode = VK_CULL_MODE_NONE; break;
  }
  
  desc_vk_.frontFace = front_counter_clockwise_ ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
  desc_vk_.depthBiasEnable = (depth_bias_ != 0) ? VK_TRUE : VK_FALSE;
  desc_vk_.depthBiasConstantFactor = static_cast<float>(depth_bias_);
  desc_vk_.depthBiasClamp = depth_bias_clamp_;
  desc_vk_.depthBiasSlopeFactor = slope_scaled_depth_bias_;
  desc_vk_.lineWidth = 1.0f;
  dirty_ = false;
  return true;
}

bool VkRasterizerState::Apply() {
  Update();
  return true;
}

//////////////////////////////////////////////////////////////////////////
// VkSamplerState

VkSamplerState::VkSamplerState(VulkanDevice* device)
    : SamplerState(device), device_(device) {}

VkSamplerState::~VkSamplerState() {
  if (sampler_ != VK_NULL_HANDLE) {
    vkDestroySampler(device_->GetVkDevice(), sampler_, nullptr);
  }
}

bool VkSamplerState::Update() {
  VkSamplerCreateInfo sampler_info{};
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  
  auto map_filter = [](Filter f) {
    if (f == Filter::kMinMagMipPoint) return VK_FILTER_NEAREST;
    return VK_FILTER_LINEAR;
  };
  
  auto map_address = [](TextureAddressMode t) {
    if (t == TextureAddressMode::kWrap) return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    if (t == TextureAddressMode::kClamp) return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    if (t == TextureAddressMode::kMirror) return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  };
  
  sampler_info.magFilter = map_filter(filter_);
  sampler_info.minFilter = map_filter(filter_);
  sampler_info.addressModeU = map_address(address_u_);
  sampler_info.addressModeV = map_address(address_v_);
  sampler_info.addressModeW = map_address(address_w_);
  sampler_info.anisotropyEnable = (max_anisotropy_ > 1) ? VK_TRUE : VK_FALSE;
  sampler_info.maxAnisotropy = static_cast<float>(max_anisotropy_);
  sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
  sampler_info.mipmapMode = (filter_ == Filter::kMinMagMipPoint) ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.mipLodBias = mip_lod_bias_;
  sampler_info.minLod = min_lod_;
  sampler_info.maxLod = max_lod_;
  
  if (sampler_ != VK_NULL_HANDLE) {
    vkDestroySampler(device_->GetVkDevice(), sampler_, nullptr);
    sampler_ = VK_NULL_HANDLE;
  }
  if (vkCreateSampler(device_->GetVkDevice(), &sampler_info, nullptr, &sampler_) != VK_SUCCESS) {
    return false;
  }
  dirty_ = false;
  return true;
}

bool VkSamplerState::Apply(Sampler sampler) {
  Update();
  uint32_t slot = 0;
  if (sampler >= Sampler::kPs0 && sampler <= Sampler::kPs15) {
      slot = static_cast<uint32_t>(sampler) - static_cast<uint32_t>(Sampler::kPs0);
  } else if (sampler >= Sampler::kVs0 && sampler <= Sampler::kVs3) {
      slot = 16 + static_cast<uint32_t>(sampler) - static_cast<uint32_t>(Sampler::kVs0);
  } else if (sampler >= Sampler::kGs0 && sampler <= Sampler::kGs3) {
      slot = 20 + static_cast<uint32_t>(sampler) - static_cast<uint32_t>(Sampler::kGs0);
  } else {
      return false;
  }
  
  device_->BindTexture(slot, VK_NULL_HANDLE, sampler_);
  return true;
}

}  // namespace renderer
