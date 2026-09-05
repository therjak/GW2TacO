#include "src/core2/vk_index_buffer.h"

namespace renderer {

VkIndexBuffer::VkIndexBuffer(VulkanDevice* device) : IndexBuffer(device) {}
VkIndexBuffer::~VkIndexBuffer() = default;

bool VkIndexBuffer::Create(const uint32_t /*index_count*/,
                           const uint32_t /*index_size*/) {
  return false;
}
bool VkIndexBuffer::Lock(void** /*result*/) { return false; }
bool VkIndexBuffer::Lock(void** /*result*/, const uint32_t /*index_offset*/,
                         const int32_t /*index_count*/) {
  return false;
}
bool VkIndexBuffer::UnLock() { return false; }
bool VkIndexBuffer::Apply() { return false; }

}  // namespace renderer
