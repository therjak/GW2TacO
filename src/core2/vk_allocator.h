#pragma once
#include <cstdint>
// Include volk first so vma uses the right function pointers
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR 1
#else
#define VK_USE_PLATFORM_WAYLAND_KHR 1
#endif
#include "volk.h"
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

namespace renderer {

// Owns a VmaAllocator instance. One per VulkanDevice.
class VkMemAllocator {
 public:
  VkMemAllocator() = default;
  ~VkMemAllocator();

  bool Initialize(VkInstance instance, VkPhysicalDevice physical_device,
                  VkDevice device);
  void Destroy();

  VmaAllocator Get() const { return allocator_; }
  bool IsValid() const { return allocator_ != VK_NULL_HANDLE; }

 private:
  VmaAllocator allocator_ = VK_NULL_HANDLE;
};

}  // namespace renderer
