#include "src/core2/vk_constant_buffer.h"

namespace renderer {

VkConstantBuffer::VkConstantBuffer(VulkanDevice* device)
    : ConstantBuffer(device) {}
VkConstantBuffer::~VkConstantBuffer() = default;

void VkConstantBuffer::Upload() {}

void* VkConstantBuffer::GetBufferPointer() const { return nullptr; }

}  // namespace renderer
