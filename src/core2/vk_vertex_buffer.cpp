#include "src/core2/vk_vertex_buffer.h"
#include <cstring>
#include "src/base/logger.h"

namespace renderer {

VkVertexBuffer::VkVertexBuffer(VulkanDevice* device) : VertexBuffer(device), vk_owner_(device) {}

VkVertexBuffer::~VkVertexBuffer() {
  Release();
}

void VkVertexBuffer::Release() {
  if (buffer_ != VK_NULL_HANDLE) {
    vmaDestroyBuffer(vk_owner_->GetAllocator(), buffer_, allocation_);
    buffer_ = VK_NULL_HANDLE;
    allocation_ = VK_NULL_HANDLE;
  }
}

bool VkVertexBuffer::Create(const uint8_t* data, const uint32_t size) {
  Release();
  size_ = size;
  dynamic_ = false;

  VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  bufferInfo.size = size;
  bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

  if (vmaCreateBuffer(vk_owner_->GetAllocator(), &bufferInfo, &allocInfo, &buffer_, &allocation_, &allocation_info_) != VK_SUCCESS) {
    Log_Err("Failed to create static vertex buffer.");
    return false;
  }

  mapped_data_ = allocation_info_.pMappedData;
  if (data && mapped_data_) {
    std::memcpy(mapped_data_, data, size);
    vmaFlushAllocation(vk_owner_->GetAllocator(), allocation_, 0, VK_WHOLE_SIZE);
  }

  return true;
}

bool VkVertexBuffer::CreateDynamic(const uint32_t size) {
  Release();
  size_ = size;
  dynamic_ = true;

  VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
  bufferInfo.size = size;
  bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

  if (vmaCreateBuffer(vk_owner_->GetAllocator(), &bufferInfo, &allocInfo, &buffer_, &allocation_, &allocation_info_) != VK_SUCCESS) {
    Log_Err("Failed to create dynamic vertex buffer.");
    return false;
  }

  mapped_data_ = allocation_info_.pMappedData;
  return true;
}

bool VkVertexBuffer::Update(const int32_t offset, const uint8_t* data, const uint32_t size) {
  if (!mapped_data_ || offset + size > size_) return false;
  std::memcpy(static_cast<uint8_t*>(mapped_data_) + offset, data, size);
  vmaFlushAllocation(vk_owner_->GetAllocator(), allocation_, offset, size);
  return true;
}

bool VkVertexBuffer::Lock(void** result) {
  if (!mapped_data_) return false;
  *result = mapped_data_;
  return true;
}

bool VkVertexBuffer::Lock(void** result, const uint32_t offset, const int32_t size, const int32_t flags) {
  if (!mapped_data_ || offset + size > size_) return false;
  *result = static_cast<uint8_t*>(mapped_data_) + offset;
  return true;
}

bool VkVertexBuffer::UnLock() {
  vmaFlushAllocation(vk_owner_->GetAllocator(), allocation_, 0, VK_WHOLE_SIZE);
  return true;
}

bool VkVertexBuffer::Apply(const uint32_t offset) {
  if (buffer_ == VK_NULL_HANDLE) return false;
  
  VkCommandBuffer cmd = vk_owner_->GetCurrentCommandBuffer();
  if (cmd == VK_NULL_HANDLE) return false;

  VkDeviceSize vk_offset = offset;
  vkCmdBindVertexBuffers(cmd, 0, 1, &buffer_, &vk_offset);
  return true;
}

}  // namespace renderer
