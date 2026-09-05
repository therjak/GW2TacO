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
  void* GetHandle() override { return nullptr; }

 private:
  VulkanDevice* device_;
};

class VkDepthStencilState : public DepthStencilState {
 public:
  explicit VkDepthStencilState(VulkanDevice* device);
  ~VkDepthStencilState() override;

  bool Update() override;
  bool Apply() override;
  void* GetHandle() override { return nullptr; }

 private:
  VulkanDevice* device_;
};

class VkRasterizerState : public RasterizerState {
 public:
  explicit VkRasterizerState(VulkanDevice* device);
  ~VkRasterizerState() override;

  bool Update() override;
  bool Apply() override;
  void* GetHandle() override { return nullptr; }

 private:
  VulkanDevice* device_;
};

class VkSamplerState : public SamplerState {
 public:
  explicit VkSamplerState(VulkanDevice* device);
  ~VkSamplerState() override;

  bool Update() override;
  bool Apply(Sampler sampler) override;
  void* GetHandle() override { return nullptr; }

 private:
  VulkanDevice* device_;
};

}  // namespace renderer
