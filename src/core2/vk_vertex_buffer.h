#pragma once
#include <cstdint>
#define VK_USE_PLATFORM_WIN32_KHR 1
#include "volk.h"
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"
#include "src/core2/vertex_buffer.h"
#include "src/core2/vk_device.h"

namespace renderer {

class VkVertexBuffer : public VertexBuffer {
 public:
  explicit VkVertexBuffer(VulkanDevice* device);
  ~VkVertexBuffer() override;

  bool Create(const uint8_t* data, const uint32_t size) override;
  bool CreateDynamic(const uint32_t size) override;
  bool Update(const int32_t offset, const uint8_t* data, const uint32_t size) override;
  bool Lock(void** result) override;
  bool Lock(void** result, const uint32_t offset, const int32_t size, const int32_t flags = 0) override;
  bool UnLock() override;
  void* GetHandle() override { return buffer_; }

 private:
  bool Apply(const uint32_t offset) override;
  void Release();

  VulkanDevice* vk_owner_ = nullptr;
  VkBuffer buffer_ = VK_NULL_HANDLE;
  VmaAllocation allocation_ = VK_NULL_HANDLE;
  VmaAllocationInfo allocation_info_ = {};
  uint32_t size_ = 0;
  bool dynamic_ = false;
  void* mapped_data_ = nullptr;
};

}  // namespace renderer
