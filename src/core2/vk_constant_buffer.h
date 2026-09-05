#pragma once
#include <cstdint>

#include "src/core2/constant_buffer.h"
#include "src/core2/vk_device.h"

namespace renderer {

class VkConstantBuffer : public ConstantBuffer {
 public:
  explicit VkConstantBuffer(VulkanDevice* device);
  ~VkConstantBuffer() override;

  void Upload() override;
  [[nodiscard]] void* GetBufferPointer() const override;
};

}  // namespace renderer
