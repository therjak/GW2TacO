#pragma once
#include <cstdint>

#include "src/core2/index_buffer.h"
#include "src/core2/vk_device.h"

namespace renderer {

class VkIndexBuffer : public IndexBuffer {
 public:
  explicit VkIndexBuffer(VulkanDevice* device);
  ~VkIndexBuffer() override;

  bool Create(const uint32_t index_count,
              const uint32_t index_size = 2) override;
  bool Lock(void** result) override;
  bool Lock(void** result, const uint32_t index_offset,
            const int32_t index_count) override;
  bool UnLock() override;
  void* GetHandle() override { return nullptr; }

 private:
  bool Apply() override;
};

}  // namespace renderer
