#pragma once
#include <cstdint>
#define VK_USE_PLATFORM_WIN32_KHR 1
#include "volk.h"
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"
#include "src/core2/index_buffer.h"
#include "src/core2/vk_device.h"

namespace renderer {

class VkIndexBuffer : public IndexBuffer {
 public:
  explicit VkIndexBuffer(VulkanDevice* device);
  ~VkIndexBuffer() override;

  bool Create(const uint32_t index_count, const uint32_t index_size = 2) override;
  bool Lock(void** result, const uint32_t index_offset, const int32_t index_count) override;
  bool Lock(void** result) override;
  bool UnLock() override;
  void* GetHandle() override { return buffer_; }

 private:
  bool Apply() override;
  void Release();

  VulkanDevice* vk_owner_ = nullptr;
  VkBuffer buffer_ = VK_NULL_HANDLE;
  VmaAllocation allocation_ = VK_NULL_HANDLE;
  VmaAllocationInfo allocation_info_ = {};
  uint32_t size_ = 0;
  void* mapped_data_ = nullptr;
  VkIndexType index_type_ = VK_INDEX_TYPE_UINT16;
};

}  // namespace renderer
