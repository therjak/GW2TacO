#include "src/core2/vk_shader.h"

namespace renderer {

//////////////////////////////////////////////////////////////////////////
// VkVertexShader

VkVertexShader::VkVertexShader(VulkanDevice* device) : VertexShader(device) {}
VkVertexShader::~VkVertexShader() = default;
bool VkVertexShader::Create(void* /*binary*/, int32_t /*length*/) {
  return false;
}
bool VkVertexShader::CompileAndCreate(std::string* /*error*/) { return false; }
bool VkVertexShader::CreateFromBlob(void* /*code*/, int32_t /*code_size*/) {
  return false;
}
bool VkVertexShader::Apply() { return false; }

//////////////////////////////////////////////////////////////////////////
// VkPixelShader

VkPixelShader::VkPixelShader(VulkanDevice* device) : PixelShader(device) {}
VkPixelShader::~VkPixelShader() = default;
bool VkPixelShader::Create(void* /*binary*/, int32_t /*length*/) {
  return false;
}
bool VkPixelShader::CompileAndCreate(std::string* /*error*/) { return false; }
bool VkPixelShader::CreateFromBlob(void* /*code*/, int32_t /*code_size*/) {
  return false;
}
bool VkPixelShader::Apply() { return false; }

//////////////////////////////////////////////////////////////////////////
// VkGeometryShader

VkGeometryShader::VkGeometryShader(VulkanDevice* device)
    : GeometryShader(device) {}
VkGeometryShader::~VkGeometryShader() = default;
bool VkGeometryShader::Create(void* /*binary*/, int32_t /*length*/) {
  return false;
}
bool VkGeometryShader::CompileAndCreate(std::string* /*error*/) {
  return false;
}
bool VkGeometryShader::CreateFromBlob(void* /*code*/, int32_t /*code_size*/) {
  return false;
}
bool VkGeometryShader::Apply() { return false; }

//////////////////////////////////////////////////////////////////////////
// VkHullShader

VkHullShader::VkHullShader(VulkanDevice* device) : HullShader(device) {}
VkHullShader::~VkHullShader() = default;
bool VkHullShader::Create(void* /*binary*/, int32_t /*length*/) {
  return false;
}
bool VkHullShader::CompileAndCreate(std::string* /*error*/) { return false; }
bool VkHullShader::CreateFromBlob(void* /*code*/, int32_t /*code_size*/) {
  return false;
}
bool VkHullShader::Apply() { return false; }

//////////////////////////////////////////////////////////////////////////
// VkDomainShader

VkDomainShader::VkDomainShader(VulkanDevice* device) : DomainShader(device) {}
VkDomainShader::~VkDomainShader() = default;
bool VkDomainShader::Create(void* /*binary*/, int32_t /*length*/) {
  return false;
}
bool VkDomainShader::CompileAndCreate(std::string* /*error*/) { return false; }
bool VkDomainShader::CreateFromBlob(void* /*code*/, int32_t /*code_size*/) {
  return false;
}
bool VkDomainShader::Apply() { return false; }

//////////////////////////////////////////////////////////////////////////
// VkComputeShader

VkComputeShader::VkComputeShader(VulkanDevice* device)
    : ComputeShader(device) {}
VkComputeShader::~VkComputeShader() = default;
bool VkComputeShader::Create(void* /*binary*/, int32_t /*length*/) {
  return false;
}
bool VkComputeShader::CompileAndCreate(std::string* /*error*/) {
  return false;
}
bool VkComputeShader::CreateFromBlob(void* /*code*/, int32_t /*code_size*/) {
  return false;
}
bool VkComputeShader::Apply() { return false; }

}  // namespace renderer
