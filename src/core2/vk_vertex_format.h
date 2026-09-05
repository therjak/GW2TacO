#pragma once
#include <cstdint>
#include <vector>

#include "src/core2/vertex_format.h"
#include "src/core2/vk_device.h"

namespace renderer {

class VkVertexFormat : public VertexFormat {
 public:
  explicit VkVertexFormat(VulkanDevice* device);
  ~VkVertexFormat() override;

  bool Create(const std::vector<VertexAttribute>& attributes,
              VertexShader* vertex_shader = nullptr) override;
  int32_t GetSize() override;

 private:
  bool Apply() override;

  int32_t size_ = 0;
};

}  // namespace renderer
