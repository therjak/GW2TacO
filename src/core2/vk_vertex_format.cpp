#include "src/core2/vk_vertex_format.h"

namespace renderer {

VkVertexFormat::VkVertexFormat(VulkanDevice* device)
    : VertexFormat(device) {}
VkVertexFormat::~VkVertexFormat() = default;

bool VkVertexFormat::Create(const std::vector<VertexAttribute>& /*attributes*/,
                            VertexShader* /*vertex_shader*/) {
  return false;
}
int32_t VkVertexFormat::GetSize() { return size_; }
bool VkVertexFormat::Apply() { return false; }

}  // namespace renderer
