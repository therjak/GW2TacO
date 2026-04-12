#pragma once

#include <array>
#include <cstdint>

#include "src/core2/index_buffer.h"
#include "src/core2/shader.h"
#include "src/core2/texture.h"
#include "src/core2/vertex_format.h"

import xml;

namespace renderer {

class CCoreSamplerState;
class CCoreDepthStencilState;
class CCoreBlendState;
class CCoreRasterizerState;

union CoreRenderStateValue {
  CCoreSamplerState* sampler_state;
  CCoreDepthStencilState* depth_stencil_state;
  CCoreBlendState* blend_state;
  CCoreRasterizerState* rasterizer_state;
  CCoreTexture* texture;
  CCoreIndexBuffer* index_buffer;
  CCoreVertexFormat* vertex_format;
  CCoreVertexShader* vertex_shader;
  CCorePixelShader* pixel_shader;
  CCoreGeometryShader* geometry_shader;
  CCoreDomainShader* domain_shader;
  CCoreComputeShader* compute_shader;
  CCoreHullShader* hull_shader;

  constexpr friend bool operator==(const CoreRenderStateValue& lhs,
                                   const CoreRenderStateValue& rhs) {
    return lhs.sampler_state == rhs.sampler_state;
  }
};

using CoreRenderStateId = uint32_t;

struct CoreBlendDescriptor {
  bool blend_enable = false;
  CoreBlendFactor src_blend = CoreBlendFactor::kZero;
  CoreBlendFactor dest_blend = CoreBlendFactor::kZero;
  CoreBlendOp blend_op = CoreBlendOp::kAdd;
  CoreBlendFactor src_blend_alpha = CoreBlendFactor::kZero;
  CoreBlendFactor dest_blend_alpha = CoreBlendFactor::kZero;
  CoreBlendOp blend_op_alpha = CoreBlendOp::kAdd;
  uint8_t render_target_write_mask = 0;
};

class CCoreRenderStateBatch : public CCoreResource {
 public:
  explicit CCoreRenderStateBatch(CCoreDevice* device);
  ~CCoreRenderStateBatch() override;
  virtual bool Import(CXMLNode* n) = 0;
  virtual void Export(CXMLNode* n) = 0;

 protected:
  bool dirty_ = false;
};

class CCoreBlendState : public CCoreRenderStateBatch {
 public:
  explicit CCoreBlendState(CCoreDevice* device);
  ~CCoreBlendState() override;

  virtual bool Update() = 0;
  virtual bool Apply() = 0;

  void SetAlphaToCoverage(bool enable);
  void SetIndependentBlend(bool enable);
  void SetBlendEnable(int32_t render_target, bool enable);
  void SetSrcBlend(int32_t render_target, CoreBlendFactor factor);
  void SetDestBlend(int32_t render_target, CoreBlendFactor factor);
  void SetBlendOp(int32_t render_target, CoreBlendOp op);
  void SetSrcBlendAlpha(int32_t render_target, CoreBlendFactor factor);
  void SetDestBlendAlpha(int32_t render_target, CoreBlendFactor factor);
  void SetBlendOpAlpha(int32_t render_target, CoreBlendOp op);
  void SetRenderTargetWriteMask(int32_t render_target, uint8_t mask);

  bool Import(CXMLNode* n) override;
  void Export(CXMLNode* n) override;
  virtual void* GetHandle() = 0;

 protected:
  bool alpha_to_coverage_;
  bool independent_blend_;
  std::array<CoreBlendDescriptor, 8> render_target_blend_states_;
};

class CCoreDepthStencilState : public CCoreRenderStateBatch {
 public:
  explicit CCoreDepthStencilState(CCoreDevice* device);
  ~CCoreDepthStencilState() override;

  virtual bool Update() = 0;
  virtual bool Apply() = 0;

  void SetDepthEnable(bool enable);
  void SetZWriteEnable(bool enable);
  void SetDepthFunc(CoreComparisonFunction func);

  bool Import(CXMLNode* n) override;
  void Export(CXMLNode* n) override;

  virtual void* GetHandle() = 0;

 protected:
  bool depth_enable_;
  bool z_write_enable_;
  CoreComparisonFunction depth_func_;
};

class CCoreRasterizerState : public CCoreRenderStateBatch {
 public:
  explicit CCoreRasterizerState(CCoreDevice* device);
  ~CCoreRasterizerState() override;

  virtual bool Update() = 0;
  virtual bool Apply() = 0;

  void SetFillMode(CoreFillMode mode);
  void SetCullMode(CoreCullMode mode);
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
  CoreFillMode fill_mode_;
  CoreCullMode cull_mode_;
  bool front_counter_clockwise_;
  int32_t depth_bias_;
  float depth_bias_clamp_;
  float slope_scaled_depth_bias_;
  bool depth_clip_enable_;
  bool scissor_enable_;
  bool multisample_enable_;
  bool antialiased_line_enable_;
};

class CCoreSamplerState : public CCoreRenderStateBatch {
 public:
  explicit CCoreSamplerState(CCoreDevice* device);
  ~CCoreSamplerState() override;

  virtual bool Update() = 0;
  virtual bool Apply(CoreSampler sampler) = 0;

  void SetFilter(CoreFilter filter);

  void SetAddressU(CoreTextureAddressMode mode);
  void SetAddressV(CoreTextureAddressMode mode);
  void SetAddressW(CoreTextureAddressMode mode);

  void SetMipLODBias(float bias);
  void SetMaxAnisotropy(int32_t anisotropy);
  void SetComparisonFunc(CoreComparisonFunction func);
  void SetMinLOD(float lod);
  void SetMaxLOD(float lod);

  void SetBorderColor(float r, float g, float b, float a);

  bool Import(CXMLNode* n) override;
  void Export(CXMLNode* n) override;
  virtual void* GetHandle() = 0;

 protected:
  CoreFilter filter_;
  CoreTextureAddressMode address_u_;
  CoreTextureAddressMode address_v_;
  CoreTextureAddressMode address_w_;
  float mip_lod_bias_;
  int32_t max_anisotropy_;
  CoreComparisonFunction comparison_func_;
  std::array<float, 4> border_color_ = {0};
  float min_lod_;
  float max_lod_;
};
}  // namespace renderer
