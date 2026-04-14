#pragma once

#include <array>
#include <cstdint>

#include "src/core2/index_buffer.h"
#include "src/core2/shader.h"
#include "src/core2/texture.h"
#include "src/core2/vertex_format.h"

import xml;

namespace renderer {

class SamplerState;
class DepthStencilState;
class BlendState;
class RasterizerState;

union RenderStateValue {
  SamplerState* sampler_state;
  DepthStencilState* depth_stencil_state;
  BlendState* blend_state;
  RasterizerState* rasterizer_state;
  Texture* texture;
  IndexBuffer* index_buffer;
  VertexFormat* vertex_format;
  VertexShader* vertex_shader;
  PixelShader* pixel_shader;
  GeometryShader* geometry_shader;
  DomainShader* domain_shader;
  ComputeShader* compute_shader;
  HullShader* hull_shader;

  constexpr friend bool operator==(const RenderStateValue& lhs,
                                   const RenderStateValue& rhs) {
    return lhs.sampler_state == rhs.sampler_state;
  }
};

using RenderStateId = uint32_t;

struct BlendDescriptor {
  bool blend_enable = false;
  BlendFactor src_blend = BlendFactor::kZero;
  BlendFactor dest_blend = BlendFactor::kZero;
  BlendOp blend_op = BlendOp::kAdd;
  BlendFactor src_blend_alpha = BlendFactor::kZero;
  BlendFactor dest_blend_alpha = BlendFactor::kZero;
  BlendOp blend_op_alpha = BlendOp::kAdd;
  uint8_t render_target_write_mask = 0;
};

class RenderStateBatch : public Resource {
 public:
  explicit RenderStateBatch(Device* device);
  ~RenderStateBatch() override;
  virtual bool Import(CXMLNode* n) = 0;
  virtual void Export(CXMLNode* n) = 0;

 protected:
  bool dirty_ = false;
};

class BlendState : public RenderStateBatch {
 public:
  explicit BlendState(Device* device);
  ~BlendState() override;

  virtual bool Update() = 0;
  virtual bool Apply() = 0;

  void SetAlphaToCoverage(bool enable);
  void SetIndependentBlend(bool enable);
  void SetBlendEnable(int32_t render_target, bool enable);
  void SetSrcBlend(int32_t render_target, BlendFactor factor);
  void SetDestBlend(int32_t render_target, BlendFactor factor);
  void SetBlendOp(int32_t render_target, BlendOp op);
  void SetSrcBlendAlpha(int32_t render_target, BlendFactor factor);
  void SetDestBlendAlpha(int32_t render_target, BlendFactor factor);
  void SetBlendOpAlpha(int32_t render_target, BlendOp op);
  void SetRenderTargetWriteMask(int32_t render_target, uint8_t mask);

  bool Import(CXMLNode* n) override;
  void Export(CXMLNode* n) override;
  virtual void* GetHandle() = 0;

 protected:
  bool alpha_to_coverage_;
  bool independent_blend_;
  std::array<BlendDescriptor, 8> render_target_blend_states_;
};

class DepthStencilState : public RenderStateBatch {
 public:
  explicit DepthStencilState(Device* device);
  ~DepthStencilState() override;

  virtual bool Update() = 0;
  virtual bool Apply() = 0;

  void SetDepthEnable(bool enable);
  void SetZWriteEnable(bool enable);
  void SetDepthFunc(ComparisonFunction func);

  bool Import(CXMLNode* n) override;
  void Export(CXMLNode* n) override;

  virtual void* GetHandle() = 0;

 protected:
  bool depth_enable_;
  bool z_write_enable_;
  ComparisonFunction depth_func_;
};

class RasterizerState : public RenderStateBatch {
 public:
  explicit RasterizerState(Device* device);
  ~RasterizerState() override;

  virtual bool Update() = 0;
  virtual bool Apply() = 0;

  void SetFillMode(FillMode mode);
  void SetCullMode(CullMode mode);
  void SetFrontCounterClockwise(bool enable);
  void SetDepthBias(int32_t bias);
  void SetDepthBiasClamp(float clamp);
  void SetSlopeScaledDepthBias(float bias);
  void SetDepthClipEnable(bool enable);
  void SetScissorEnable(bool enable);
  void SetMultisampleEnable(bool enable);
  void SetAntialiasedLineEnable(bool enable);

  bool Import(CXMLNode* n) override;
  void Export(CXMLNode* n) override;
  virtual void* GetHandle() = 0;

 protected:
  FillMode fill_mode_;
  CullMode cull_mode_;
  bool front_counter_clockwise_;
  int32_t depth_bias_;
  float depth_bias_clamp_;
  float slope_scaled_depth_bias_;
  bool depth_clip_enable_;
  bool scissor_enable_;
  bool multisample_enable_;
  bool antialiased_line_enable_;
};

class SamplerState : public RenderStateBatch {
 public:
  explicit SamplerState(Device* device);
  ~SamplerState() override;

  virtual bool Update() = 0;
  virtual bool Apply(Sampler sampler) = 0;

  void SetFilter(Filter filter);

  void SetAddressU(TextureAddressMode mode);
  void SetAddressV(TextureAddressMode mode);
  void SetAddressW(TextureAddressMode mode);

  void SetMipLODBias(float bias);
  void SetMaxAnisotropy(int32_t anisotropy);
  void SetComparisonFunc(ComparisonFunction func);
  void SetMinLOD(float lod);
  void SetMaxLOD(float lod);

  void SetBorderColor(float r, float g, float b, float a);

  bool Import(CXMLNode* n) override;
  void Export(CXMLNode* n) override;
  virtual void* GetHandle() = 0;

 protected:
  Filter filter_;
  TextureAddressMode address_u_;
  TextureAddressMode address_v_;
  TextureAddressMode address_w_;
  float mip_lod_bias_;
  int32_t max_anisotropy_;
  ComparisonFunction comparison_func_;
  std::array<float, 4> border_color_ = {0};
  float min_lod_;
  float max_lod_;
};
}  // namespace renderer
