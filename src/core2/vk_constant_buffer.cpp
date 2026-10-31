#include "src/core2/vk_constant_buffer.h"
#include <cstring>
#include "src/base/logger.h"

namespace renderer {

VkConstantBuffer::VkConstantBuffer(VulkanDevice* device) : ConstantBuffer(device), vk_owner_(device) {}

VkConstantBuffer::~VkConstantBuffer() {
  Release();
}

void VkConstantBuffer::Release() {
  if (buffer_ != VK_NULL_HANDLE) {
    vmaDestroyBuffer(vk_owner_->GetAllocator(), buffer_, allocation_);
    buffer_ = VK_NULL_HANDLE;
    allocation_ = VK_NULL_HANDLE;
    mapped_data_ = nullptr;
  }
}

bool VkConstantBuffer::Setup() {
  Release();
  
  if (buffer_length_ == 0) return false;

  VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  bufferInfo.size = buffer_length_;
  bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

  if (vmaCreateBuffer(vk_owner_->GetAllocator(), &bufferInfo, &allocInfo, &buffer_, &allocation_, &allocation_info_) != VK_SUCCESS) {
    Log_Err("Failed to create constant buffer.");
    return false;
  }

  mapped_data_ = allocation_info_.pMappedData;
  return true;
}

void VkConstantBuffer::Upload() {
  if (!mapped_data_ || !data_) return;
  std::memcpy(mapped_data_, data_.get(), buffer_length_);
  vmaFlushAllocation(vk_owner_->GetAllocator(), allocation_, 0, VK_WHOLE_SIZE);
}

void* VkConstantBuffer::GetBufferPointer() const {
  return mapped_data_;
}

}  // namespace renderer
