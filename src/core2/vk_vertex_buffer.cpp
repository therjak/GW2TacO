#include "src/core2/vk_vertex_buffer.h"

namespace renderer {

VkVertexBuffer::VkVertexBuffer(VulkanDevice* device)
    : VertexBuffer(device) {}
VkVertexBuffer::~VkVertexBuffer() = default;

bool VkVertexBuffer::Create(const uint8_t* /*data*/, const uint32_t /*size*/) {
  return false;
}
bool VkVertexBuffer::CreateDynamic(const uint32_t /*size*/) { return false; }
bool VkVertexBuffer::Update(const int32_t /*offset*/, const uint8_t* /*data*/,
                            const uint32_t /*size*/) {
  return false;
}
bool VkVertexBuffer::Lock(void** /*result*/) { return false; }
bool VkVertexBuffer::Lock(void** /*result*/, const uint32_t /*offset*/,
                          const int32_t /*size*/, const int32_t /*flags*/) {
  return false;
}
bool VkVertexBuffer::UnLock() { return false; }
bool VkVertexBuffer::Apply(const uint32_t /*offset*/) { return false; }

}  // namespace renderer
