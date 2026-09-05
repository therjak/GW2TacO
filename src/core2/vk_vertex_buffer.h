#pragma once
#include <cstdint>

#include "src/core2/vertex_buffer.h"
#include "src/core2/vk_device.h"

namespace renderer {

class VkVertexBuffer : public VertexBuffer {
 public:
  explicit VkVertexBuffer(VulkanDevice* device);
  ~VkVertexBuffer() override;

  bool Create(const uint8_t* data, const uint32_t size) override;
  bool CreateDynamic(const uint32_t size) override;
  bool Update(const int32_t offset, const uint8_t* data,
              const uint32_t size) override;
  bool Lock(void** result) override;
  bool Lock(void** result, const uint32_t offset, const int32_t size,
            const int32_t flags = 0) override;
  bool UnLock() override;
  void* GetHandle() override { return nullptr; }

 private:
  bool Apply(const uint32_t offset) override;
};

}  // namespace renderer
