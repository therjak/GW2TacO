#pragma once
#include "src/core2/render_state.h"
#include "src/core2/vk_device.h"

namespace renderer {

class VkBlendState : public BlendState {
 public:
  explicit VkBlendState(VulkanDevice* device);
  ~VkBlendState() override;

  bool Update() override;
  bool Apply() override;
  void* GetHandle() override { return &desc_vk_; }
  const VkPipelineColorBlendStateCreateInfo* GetVkDesc() const { return &desc_vk_; }
  const BlendDescriptor& GetDesc(int rt = 0) const { return render_target_blend_states_[rt]; }

 private:
  VulkanDevice* device_;
  VkPipelineColorBlendStateCreateInfo desc_vk_{};
  VkPipelineColorBlendAttachmentState attachment_vk_{};
};

class VkDepthStencilState : public DepthStencilState {
 public:
  explicit VkDepthStencilState(VulkanDevice* device);
  ~VkDepthStencilState() override;

  bool Update() override;
  bool Apply() override;
  void* GetHandle() override { return &desc_vk_; }
  const VkPipelineDepthStencilStateCreateInfo* GetVkDesc() const { return &desc_vk_; }
  bool GetDepthEnable() const { return depth_enable_; }
  bool GetDepthWriteEnable() const { return z_write_enable_; }
  ComparisonFunction GetDepthFunc() const { return depth_func_; }

 private:
  VulkanDevice* device_;
  VkPipelineDepthStencilStateCreateInfo desc_vk_{};
};

class VkRasterizerState : public RasterizerState {
 public:
  explicit VkRasterizerState(VulkanDevice* device);
  ~VkRasterizerState() override;

  bool Update() override;
  bool Apply() override;
  void* GetHandle() override { return &desc_vk_; }
  const VkPipelineRasterizationStateCreateInfo* GetVkDesc() const { return &desc_vk_; }
  CullMode GetCullMode() const { return cull_mode_; }
  FillMode GetFillMode() const { return fill_mode_; }

 private:
  VulkanDevice* device_;
  VkPipelineRasterizationStateCreateInfo desc_vk_{};
};

class VkSamplerState : public SamplerState {
 public:
  explicit VkSamplerState(VulkanDevice* device);
  ~VkSamplerState() override;

  bool Update() override;
  bool Apply(Sampler sampler) override;
  void* GetHandle() override { return sampler_; }

 private:
  VulkanDevice* device_;
  VkSampler sampler_ = VK_NULL_HANDLE;
};

}  // namespace renderer
