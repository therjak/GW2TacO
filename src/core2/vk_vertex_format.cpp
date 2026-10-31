#include "src/core2/vk_vertex_format.h"

namespace renderer {

VkVertexFormat::VkVertexFormat(VulkanDevice* device) : VertexFormat(device) {}
VkVertexFormat::~VkVertexFormat() = default;

bool VkVertexFormat::Create(const std::vector<VertexAttribute>& attributes,
                            VertexShader* /*vertex_shader*/) {
  total_size_ = 0;
  attributes_.clear();

  uint32_t location = 0;
  for (const auto& attr : attributes) {
    VertexAttributeDesc desc{};
    desc.attr.binding = 0;
    desc.attr.location = location++;
    desc.attr.offset = total_size_;

    switch (attr) {
      case VertexAttribute::kPosition3:
        desc.attr.format = VK_FORMAT_R32G32B32_SFLOAT;
        desc.size = 12;
        break;
      case VertexAttribute::kPosition4:
      case VertexAttribute::kPositionT4:
      case VertexAttribute::kTexCoord4:
        desc.attr.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        desc.size = 16;
        break;
      case VertexAttribute::kNormal3:
        desc.attr.format = VK_FORMAT_R32G32B32_SFLOAT;
        desc.size = 12;
        break;
      case VertexAttribute::kTexCoord2:
        desc.attr.format = VK_FORMAT_R32G32_SFLOAT;
        desc.size = 8;
        break;
      case VertexAttribute::kColor4:
        desc.attr.format = VK_FORMAT_B8G8R8A8_UNORM;
        desc.size = 4;
        break;
      case VertexAttribute::kColor16:
        desc.attr.format = VK_FORMAT_R32G32B32A32_SFLOAT;
        desc.size = 16;
        break;
      default:
        return false;
    }
    
    total_size_ += desc.size;
    attributes_.push_back(desc);
  }

  binding_.binding = 0;
  binding_.stride = total_size_;
  binding_.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return true;
}

size_t VkVertexFormat::GetHash() const {
  size_t hash = binding_.stride;
  for (const auto& a : attributes_) {
    hash = hash * 31 + a.attr.format;
    hash = hash * 31 + a.attr.offset;
  }
  return hash;
}

bool VkVertexFormat::Apply() {
  return true;
}

}  // namespace renderer
