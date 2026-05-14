#include "src/core2/render_state.h"

#include <limits>
#include <string>

import xml;

namespace renderer {

RenderStateBatch::RenderStateBatch(Device* device)
    : Resource(device) {
  dirty_ = true;
}

RenderStateBatch::~RenderStateBatch() = default;

BlendState::BlendState(Device* device)
    : RenderStateBatch(device) {
  alpha_to_coverage_ = false;
  independent_blend_ = false;

  for (auto& render_target_blend_state : render_target_blend_states_) {
    render_target_blend_state.blend_enable = false;
    render_target_blend_state.src_blend = BlendFactor::kOne;
    render_target_blend_state.dest_blend = BlendFactor::kZero;
    render_target_blend_state.blend_op = BlendOp::kAdd;
    render_target_blend_state.src_blend_alpha = BlendFactor::kOne;
    render_target_blend_state.dest_blend_alpha = BlendFactor::kZero;
    render_target_blend_state.blend_op_alpha = BlendOp::kAdd;
    render_target_blend_state.render_target_write_mask = 0x0f;
  }
}

BlendState::~BlendState() = default;

void BlendState::SetRenderTargetWriteMask(int32_t render_target,
                                               uint8_t mask) {
  if (render_target_blend_states_[render_target].render_target_write_mask !=
      mask)
    dirty_ = true;
  render_target_blend_states_[render_target].render_target_write_mask = mask;
}

void BlendState::SetBlendOpAlpha(int32_t render_target,
                                      BlendOp blend_op) {
  if (render_target_blend_states_[render_target].blend_op_alpha != blend_op)
    dirty_ = true;
  render_target_blend_states_[render_target].blend_op_alpha = blend_op;
}

void BlendState::SetDestBlendAlpha(int32_t render_target,
                                        BlendFactor blend_factor) {
  if (render_target_blend_states_[render_target].dest_blend_alpha !=
      blend_factor)
    dirty_ = true;
  render_target_blend_states_[render_target].dest_blend_alpha = blend_factor;
}

void BlendState::SetSrcBlendAlpha(int32_t render_target,
                                       BlendFactor blend_factor) {
  if (render_target_blend_states_[render_target].src_blend_alpha !=
      blend_factor)
    dirty_ = true;
  render_target_blend_states_[render_target].src_blend_alpha = blend_factor;
}

void BlendState::SetBlendOp(int32_t render_target, BlendOp blend_op) {
  if (render_target_blend_states_[render_target].blend_op != blend_op)
    dirty_ = true;
  render_target_blend_states_[render_target].blend_op = blend_op;
}

void BlendState::SetDestBlend(int32_t render_target,
                                   BlendFactor blend_factor) {
  if (render_target_blend_states_[render_target].dest_blend != blend_factor)
    dirty_ = true;
  render_target_blend_states_[render_target].dest_blend = blend_factor;
}

void BlendState::SetSrcBlend(int32_t render_target,
                                  BlendFactor blend_factor) {
  if (render_target_blend_states_[render_target].src_blend != blend_factor)
    dirty_ = true;
  render_target_blend_states_[render_target].src_blend = blend_factor;
}

void BlendState::SetBlendEnable(int32_t render_target, bool enabled) {
  if (render_target_blend_states_[render_target].blend_enable != enabled)
    dirty_ = true;
  render_target_blend_states_[render_target].blend_enable = enabled;
}

void BlendState::SetIndependentBlend(bool enabled) {
  if (independent_blend_ != enabled) dirty_ = true;
  independent_blend_ = enabled;
}

void BlendState::SetAlphaToCoverage(bool enabled) {
  if (alpha_to_coverage_ != enabled) dirty_ = true;
  alpha_to_coverage_ = enabled;
}

bool BlendState::Import(CXMLNode* node) {
  std::string value_str;

  if (node->HasAttribute("AlphaToCoverage")) {
    node->GetChild("AlphaToCoverage").GetValue(&alpha_to_coverage_);
  }
  if (node->HasAttribute("IndependentBlend")) {
    node->GetChild("IndependentBlend").GetValue(&independent_blend_);
  }

  for (int32_t x = 0; x < node->GetChildCount("RenderTarget"); x++) {
    CXMLNode child = node->GetChild("RenderTarget", x);
    int32_t id = 0;
    child.GetAttributeAsInteger("Target", &id);

    if (child.GetChildCount("BlendEnable")) {
      child.GetChild("BlendEnable")
          .GetValue(&render_target_blend_states_[id].blend_enable);
    }
    if (child.GetChildCount("SrcBlend")) {
      value_str = child.GetChild("SrcBlend").GetText();
      FindEnumByName(BlendFactorNames, value_str,
                     render_target_blend_states_[id].src_blend);
    }
    if (child.GetChildCount("DestBlend")) {
      value_str = child.GetChild("DestBlend").GetText();
      FindEnumByName(BlendFactorNames, value_str,
                     render_target_blend_states_[id].dest_blend);
    }
    if (child.GetChildCount("BlendOp")) {
      value_str = child.GetChild("BlendOp").GetText();
      FindEnumByName(BlendOpNames, value_str,
                     render_target_blend_states_[id].blend_op);
    }

    if (child.GetChildCount("SrcBlendAlpha")) {
      value_str = child.GetChild("SrcBlendAlpha").GetText();
      FindEnumByName(BlendFactorNames, value_str,
                     render_target_blend_states_[id].src_blend_alpha);
    }
    if (child.GetChildCount("DestBlendAlpha")) {
      value_str = child.GetChild("DestBlendAlpha").GetText();
      FindEnumByName(BlendFactorNames, value_str,
                     render_target_blend_states_[id].dest_blend_alpha);
    }
    if (child.GetChildCount("BlendOpAlpha")) {
      value_str = child.GetChild("BlendOpAlpha").GetText();
      FindEnumByName(BlendOpNames, value_str,
                     render_target_blend_states_[id].blend_op_alpha);
    }

    if (child.GetChildCount("RenderTargetWriteMask")) {
      child.GetChild("RenderTargetWriteMask")
          .GetValue(&render_target_blend_states_[id].render_target_write_mask);
    }
  }

  dirty_ = true;
  return true;
}

void BlendState::Export(CXMLNode* node) {
  node->AddChild("AlphaToCoverage").SetInt(alpha_to_coverage_);
  node->AddChild("IndependentBlend").SetInt(independent_blend_);

  for (int32_t x = 0; x < 8; x++) {
    CXMLNode blend_node = node->AddChild("RenderTarget");
    blend_node.SetAttributeFromInteger("Target", x);

    blend_node.AddChild("BlendEnable")
        .SetInt(render_target_blend_states_[x].blend_enable);
    blend_node.AddChild("SrcBlend")
        .SetText(FindNameByEnum(BlendFactorNames,
                                render_target_blend_states_[x].src_blend)
                     .data());
    blend_node.AddChild("DestBlend")
        .SetText(FindNameByEnum(BlendFactorNames,
                                render_target_blend_states_[x].dest_blend)
                     .data());
    blend_node.AddChild("BlendOp").SetText(
        FindNameByEnum(BlendOpNames, render_target_blend_states_[x].blend_op)
            .data());

    blend_node.AddChild("SrcBlendAlpha")
        .SetText(FindNameByEnum(BlendFactorNames,
                                render_target_blend_states_[x].src_blend_alpha)
                     .data());
    blend_node.AddChild("DestBlendAlpha")
        .SetText(FindNameByEnum(BlendFactorNames,
                                render_target_blend_states_[x].dest_blend_alpha)
                     .data());
    blend_node.AddChild("BlendOpAlpha")
        .SetText(FindNameByEnum(BlendOpNames,
                                render_target_blend_states_[x].blend_op_alpha)
                     .data());

    blend_node.AddChild("RenderTargetWriteMask")
        .SetInt(render_target_blend_states_[x].render_target_write_mask);
  }
}

DepthStencilState::DepthStencilState(Device* device)
    : RenderStateBatch(device) {
  depth_enable_ = true;
  z_write_enable_ = true;
  depth_func_ = ComparisonFunction::kLess;
}

DepthStencilState::~DepthStencilState() = default;

void DepthStencilState::SetDepthFunc(ComparisonFunction func) {
  if (func != depth_func_) dirty_ = true;
  depth_func_ = func;
}

void DepthStencilState::SetZWriteEnable(bool enabled) {
  if (enabled != z_write_enable_) dirty_ = true;
  z_write_enable_ = enabled;
}

void DepthStencilState::SetDepthEnable(bool enabled) {
  if (enabled != depth_enable_) dirty_ = true;
  depth_enable_ = enabled;
}

bool DepthStencilState::Import(CXMLNode* node) {
  if (node->GetChildCount("DepthEnable")) {
    node->GetChild("DepthEnable").GetValue(&depth_enable_);
  }
  if (node->GetChildCount("ZWriteEnable")) {
    node->GetChild("ZWriteEnable").GetValue(&z_write_enable_);
  }
  if (node->GetChildCount("DepthFunc")) {
    auto value_str = node->GetChild("DepthFunc").GetText();
    FindEnumByName(ComparisonFunctionNames, value_str, depth_func_);
  }

  dirty_ = true;
  return true;
}

void DepthStencilState::Export(CXMLNode* node) {
  node->AddChild("DepthEnable").SetInt(depth_enable_);
  node->AddChild("ZWriteEnable").SetInt(z_write_enable_);
  node->AddChild("DepthFunc")
      .SetText(FindNameByEnum(ComparisonFunctionNames, depth_func_).data());
}

RasterizerState::RasterizerState(Device* device)
    : RenderStateBatch(device) {
  fill_mode_ = FillMode::kSolid;
  cull_mode_ = CullMode::kCcw;
  front_counter_clockwise_ = false;
  depth_bias_ = 0;
  depth_bias_clamp_ = 0;
  slope_scaled_depth_bias_ = 0;
  depth_clip_enable_ = true;
  scissor_enable_ = false;
  multisample_enable_ = false;
  antialiased_line_enable_ = false;
}

RasterizerState::~RasterizerState() = default;

void RasterizerState::SetAntialiasedLineEnable(bool enabled) {
  if (antialiased_line_enable_ != enabled) dirty_ = true;
  antialiased_line_enable_ = enabled;
}

void RasterizerState::SetMultisampleEnable(bool enabled) {
  if (multisample_enable_ != enabled) dirty_ = true;
  multisample_enable_ = enabled;
}

void RasterizerState::SetScissorEnable(bool enabled) {
  if (scissor_enable_ != enabled) dirty_ = true;
  scissor_enable_ = enabled;
}

void RasterizerState::SetDepthClipEnable(bool enabled) {
  if (depth_clip_enable_ != enabled) dirty_ = true;
  depth_clip_enable_ = enabled;
}

void RasterizerState::SetSlopeScaledDepthBias(float value) {
  if (slope_scaled_depth_bias_ != value) dirty_ = true;
  slope_scaled_depth_bias_ = value;
}

void RasterizerState::SetDepthBiasClamp(float value) {
  if (depth_bias_clamp_ != value) dirty_ = true;
  depth_bias_clamp_ = value;
}

void RasterizerState::SetDepthBias(int32_t value) {
  if (depth_bias_ != value) dirty_ = true;
  depth_bias_ = value;
}

void RasterizerState::SetFrontCounterClockwise(bool enabled) {
  if (front_counter_clockwise_ != enabled) dirty_ = true;
  front_counter_clockwise_ = enabled;
}

void RasterizerState::SetCullMode(CullMode mode) {
  if (cull_mode_ != mode) dirty_ = true;
  cull_mode_ = mode;
}

void RasterizerState::SetFillMode(FillMode mode) {
  if (fill_mode_ != mode) dirty_ = true;
  fill_mode_ = mode;
}

bool RasterizerState::Import(CXMLNode* node) {
  if (node->GetChildCount("FillMode")) {
    auto value_str = node->GetChild("FillMode").GetText();
    FindEnumByName(FillModeNames, value_str, fill_mode_);
  }
  if (node->GetChildCount("CullMode")) {
    auto value_str = node->GetChild("CullMode").GetText();
    FindEnumByName(CullModeNames, value_str, cull_mode_);
  }

  if (node->GetChildCount("DepthBias")) {
    node->GetChild("DepthBias").GetValue(&depth_bias_);
  }
  if (node->GetChildCount("DepthBiasClamp")) {
    node->GetChild("DepthBiasClamp").GetValue(&depth_bias_clamp_);
  }
  if (node->GetChildCount("SlopeScaledDepthBias")) {
    node->GetChild("SlopeScaledDepthBias").GetValue(&slope_scaled_depth_bias_);
  }

  if (node->GetChildCount("FrontCounterClockwise")) {
    node->GetChild("FrontCounterClockwise").GetValue(&front_counter_clockwise_);
  }
  if (node->GetChildCount("DepthClipEnable")) {
    node->GetChild("DepthClipEnable").GetValue(&depth_clip_enable_);
  }
  if (node->GetChildCount("ScissorEnable")) {
    node->GetChild("ScissorEnable").GetValue(&scissor_enable_);
  }
  if (node->GetChildCount("MultisampleEnable")) {
    node->GetChild("MultisampleEnable").GetValue(&multisample_enable_);
  }
  if (node->GetChildCount("AntialiasedLineEnable")) {
    node->GetChild("AntialiasedLineEnable").GetValue(&antialiased_line_enable_);
  }

  dirty_ = true;
  return true;
}

void RasterizerState::Export(CXMLNode* node) {
  node->AddChild("FillMode")
      .SetText(FindNameByEnum(FillModeNames, fill_mode_).data());
  node->AddChild("CullMode")
      .SetText(FindNameByEnum(CullModeNames, cull_mode_).data());

  node->AddChild("DepthBias").SetInt(depth_bias_);
  node->AddChild("DepthBiasClamp").SetFloat(depth_bias_clamp_);
  node->AddChild("SlopeScaledDepthBias").SetFloat(slope_scaled_depth_bias_);
  node->AddChild("FrontCounterClockwise").SetInt(front_counter_clockwise_);
  node->AddChild("DepthClipEnable").SetInt(depth_clip_enable_);
  node->AddChild("ScissorEnable").SetInt(scissor_enable_);
  node->AddChild("MultisampleEnable").SetInt(multisample_enable_);
  node->AddChild("AntialiasedLineEnable").SetInt(antialiased_line_enable_);
}

SamplerState::SamplerState(Device* device)
    : RenderStateBatch(device) {
  filter_ = Filter::kMinMagMipLinear;
  address_u_ = TextureAddressMode::kClamp;
  address_v_ = TextureAddressMode::kClamp;
  address_w_ = TextureAddressMode::kClamp;
  min_lod_ = std::numeric_limits<float>::lowest();
  max_lod_ = std::numeric_limits<float>::max();
  mip_lod_bias_ = 0;
  max_anisotropy_ = 1;
  comparison_func_ = ComparisonFunction::kNever;
  border_color_[0] = border_color_[1] = border_color_[2] = border_color_[3] = 1;
}

SamplerState::~SamplerState() = default;

void SamplerState::SetBorderColor(float r, float g, float b, float a) {
  if (border_color_[0] != r || border_color_[1] != g || border_color_[2] != b ||
      border_color_[3] != a) {
    dirty_ = true;
  }
  border_color_[0] = r;
  border_color_[1] = g;
  border_color_[2] = b;
  border_color_[3] = a;
}

void SamplerState::SetMaxLOD(float value) {
  if (max_lod_ != value) dirty_ = true;
  max_lod_ = value;
}

void SamplerState::SetMinLOD(float value) {
  if (min_lod_ != value) dirty_ = true;
  min_lod_ = value;
}

void SamplerState::SetComparisonFunc(ComparisonFunction func) {
  if (comparison_func_ != func) dirty_ = true;
  comparison_func_ = func;
}

void SamplerState::SetMaxAnisotropy(int32_t value) {
  if (max_anisotropy_ != value) dirty_ = true;
  max_anisotropy_ = value;
}

void SamplerState::SetMipLODBias(float value) {
  if (mip_lod_bias_ != value) dirty_ = true;
  mip_lod_bias_ = value;
}

void SamplerState::SetAddressW(TextureAddressMode mode) {
  if (address_w_ != mode) dirty_ = true;
  address_w_ = mode;
}

void SamplerState::SetAddressV(TextureAddressMode mode) {
  if (address_v_ != mode) dirty_ = true;
  address_v_ = mode;
}

void SamplerState::SetAddressU(TextureAddressMode mode) {
  if (address_u_ != mode) dirty_ = true;
  address_u_ = mode;
}

void SamplerState::SetFilter(Filter filter) {
  if (filter_ != filter) dirty_ = true;
  filter_ = filter;
}

bool SamplerState::Import(CXMLNode* node) {
  if (node->GetChildCount("Filter")) {
    auto value_str = node->GetChild("Filter").GetText();
    FindEnumByName(FilterNames, value_str, filter_);
  }
  if (node->GetChildCount("AddressU")) {
    auto value_str = node->GetChild("AddressU").GetText();
    FindEnumByName(AddressModeNames, value_str, address_u_);
  }
  if (node->GetChildCount("AddressV")) {
    auto value_str = node->GetChild("AddressV").GetText();
    FindEnumByName(AddressModeNames, value_str, address_v_);
  }
  if (node->GetChildCount("AddressW")) {
    auto value_str = node->GetChild("AddressW").GetText();
    FindEnumByName(AddressModeNames, value_str, address_w_);
  }

  if (node->GetChildCount("ComparisonFunc")) {
    auto value_str = node->GetChild("ComparisonFunc").GetText();
    FindEnumByName(ComparisonFunctionNames, value_str, comparison_func_);
  }

  if (node->GetChildCount("MipLODBias")) {
    node->GetChild("MipLODBias").GetValue(&mip_lod_bias_);
  }
  if (node->GetChildCount("MinLOD")) {
    node->GetChild("MinLOD").GetValue(&min_lod_);
  }
  if (node->GetChildCount("MaxLOD")) {
    node->GetChild("MaxLOD").GetValue(&max_lod_);
  }

  if (node->GetChildCount("MaxAnisotropy")) {
    node->GetChild("MaxAnisotropy").GetValue(&max_anisotropy_);
  }

  dirty_ = true;
  return true;
}

void SamplerState::Export(CXMLNode* node) {
  node->AddChild("Filter").SetText(FindNameByEnum(FilterNames, filter_));
  node->AddChild("AddressU")
      .SetText(FindNameByEnum(AddressModeNames, address_u_));
  node->AddChild("AddressV")
      .SetText(FindNameByEnum(AddressModeNames, address_v_));
  node->AddChild("AddressW")
      .SetText(FindNameByEnum(AddressModeNames, address_w_));
  node->AddChild("ComparisonFunc")
      .SetText(FindNameByEnum(ComparisonFunctionNames, comparison_func_));
  node->AddChild("MipLODBias").SetFloat(mip_lod_bias_);
  node->AddChild("MinLOD").SetFloat(min_lod_);
  node->AddChild("MaxLOD").SetFloat(max_lod_);
  node->AddChild("MaxAnisotropy").SetInt(max_anisotropy_);
}

}  // namespace renderer
