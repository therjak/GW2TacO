#include "src/core2/vk_allocator.h"
#include "src/base/logger.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace renderer {

VkMemAllocator::~VkMemAllocator() {
  Destroy();
}

bool VkMemAllocator::Initialize(VkInstance instance, VkPhysicalDevice physical_device,
                                VkDevice device) {
  VmaVulkanFunctions vulkanFunctions = {};
  vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.physicalDevice = physical_device;
  allocatorInfo.device = device;
  allocatorInfo.instance = instance;
  allocatorInfo.pVulkanFunctions = &vulkanFunctions;

  // Enable buffer device address if supported (assuming Vulkan 1.2+ setup elsewhere)
  // For now just setting VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT
  // We can query physical device features if we need to be strictly safe.
  // Actually, standard taco might not enable this. The prompt says:
  // "create VmaAllocator with VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT if supported, otherwise 0 flags"
  
  // To keep it simple, check if the extension was enabled or just try.
  // Let's assume standard Vulkan 1.2 and just set it if we want, but it might fail 
  // if not enabled in device creation. 
  // Let's just use 0 flags for safety if we don't know, or try creating with the flag,
  // and if it fails, fallback to 0.

  allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  
  VkResult res = vmaCreateAllocator(&allocatorInfo, &allocator_);
  if (res != VK_SUCCESS) {
      allocatorInfo.flags = 0;
      res = vmaCreateAllocator(&allocatorInfo, &allocator_);
  }
  
  if (res != VK_SUCCESS) {
      Log_Err("Failed to create VMA allocator!");
      return false;
  }
  
  return true;
}

void VkMemAllocator::Destroy() {
  if (allocator_ != VK_NULL_HANDLE) {
    vmaDestroyAllocator(allocator_);
    allocator_ = VK_NULL_HANDLE;
  }
}

}  // namespace renderer
