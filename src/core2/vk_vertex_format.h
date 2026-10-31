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
  int32_t GetSize() override { return total_size_; }

  VkVertexInputBindingDescription GetBindingDescription() const { return binding_; }
  std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() const {
    std::vector<VkVertexInputAttributeDescription> res;
    for (const auto& a : attributes_) res.push_back(a.attr);
    return res;
  }
  size_t GetHash() const;

 private:
  bool Apply() override;

  struct VertexAttributeDesc {
    VkVertexInputAttributeDescription attr;
    uint32_t size;
  };
  std::vector<VertexAttributeDesc> attributes_;
  VkVertexInputBindingDescription binding_{};
  int32_t total_size_ = 0;
};

}  // namespace renderer
