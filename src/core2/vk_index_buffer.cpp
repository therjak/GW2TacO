#include "src/core2/vk_index_buffer.h"
#include <cstring>
#include "src/base/logger.h"

namespace renderer {

VkIndexBuffer::VkIndexBuffer(VulkanDevice* device) : IndexBuffer(device), vk_owner_(device) {}

VkIndexBuffer::~VkIndexBuffer() {
  Release();
}

void VkIndexBuffer::Release() {
  if (buffer_ != VK_NULL_HANDLE) {
    vmaDestroyBuffer(vk_owner_->GetAllocator(), buffer_, allocation_);
    buffer_ = VK_NULL_HANDLE;
    allocation_ = VK_NULL_HANDLE;
    mapped_data_ = nullptr;
  }
}

bool VkIndexBuffer::Create(const uint32_t index_count, const uint32_t index_size) {
  Release();
  size_ = index_count * index_size;
  index_type_ = (index_size == 4) ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;

  VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  bufferInfo.size = size_;
  bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

  if (vmaCreateBuffer(vk_owner_->GetAllocator(), &bufferInfo, &allocInfo, &buffer_, &allocation_, &allocation_info_) != VK_SUCCESS) {
    Log_Err("Failed to create index buffer.");
    return false;
  }

  mapped_data_ = allocation_info_.pMappedData;
  return true;
}

bool VkIndexBuffer::Lock(void** result) {
  if (!mapped_data_) return false;
  *result = mapped_data_;
  return true;
}

bool VkIndexBuffer::Lock(void** result, const uint32_t index_offset, const int32_t index_count) {
  uint32_t offset = index_offset * (index_type_ == VK_INDEX_TYPE_UINT32 ? 4 : 2);
  uint32_t count_bytes = index_count * (index_type_ == VK_INDEX_TYPE_UINT32 ? 4 : 2);
  if (!mapped_data_ || offset + count_bytes > size_) return false;
  *result = static_cast<uint8_t*>(mapped_data_) + offset;
  return true;
}

bool VkIndexBuffer::UnLock() {
  vmaFlushAllocation(vk_owner_->GetAllocator(), allocation_, 0, VK_WHOLE_SIZE);
  return true;
}

bool VkIndexBuffer::Apply() {
  if (buffer_ == VK_NULL_HANDLE) return false;

  VkCommandBuffer cmd = vk_owner_->GetCurrentCommandBuffer();
  if (cmd == VK_NULL_HANDLE) return false;

  vkCmdBindIndexBuffer(cmd, buffer_, 0, index_type_);
  return true;
}

}  // namespace renderer
