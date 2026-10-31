#pragma once
#include <cstdint>
#define VK_USE_PLATFORM_WIN32_KHR 1
#include "volk.h"
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"
#include "src/core2/constant_buffer.h"
#include "src/core2/vk_device.h"

namespace renderer {

class VkConstantBuffer : public ConstantBuffer {
 public:
  explicit VkConstantBuffer(VulkanDevice* device);
  ~VkConstantBuffer() override;

  void Upload() override;
  [[nodiscard]] void* GetBufferPointer() const override;
  
  // Phase 5 will need this to bind to descriptors
  VkBuffer GetVkBuffer() const { return buffer_; }

 private:
  bool Setup();
  void Release();

  VulkanDevice* vk_owner_ = nullptr;
  VkBuffer buffer_ = VK_NULL_HANDLE;
  VmaAllocation allocation_ = VK_NULL_HANDLE;
  VmaAllocationInfo allocation_info_ = {};
  void* mapped_data_ = nullptr;
};

}  // namespace renderer
