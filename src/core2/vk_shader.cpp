#include "src/core2/vk_shader.h"

#include <cstdint>
#include <string>
#include <vector>

#include "src/base/logger.h"
#include "src/core2/dxc_compiler.h"

namespace renderer {

// ---------------------------------------------------------------------------
// Free helpers
// ---------------------------------------------------------------------------

bool CreateShaderModule(VkDevice device, const void* spirv_data,
                        size_t spirv_bytes, VkShaderModule* out_module) {
  VkShaderModuleCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  info.codeSize = spirv_bytes;
  info.pCode = reinterpret_cast<const uint32_t*>(spirv_data);

  return vkCreateShaderModule(device, &info, nullptr, out_module) == VK_SUCCESS;
}

// ---------------------------------------------------------------------------
// Shared helper: map shader_version_ string (e.g. "vs_5_0") → DXC profile
// ---------------------------------------------------------------------------
namespace {

// Maps a D3D-style shader model string (vs_5_0, ps_6_0, …) to the DXC
// target profile that supports SPIR-V output.  DXC requires model >= 6_0 for
// SPIR-V; we floor anything older to the 6_0 equivalent.
std::string ToDxcProfile(const std::string& ver) {
  std::string stage;
  if (ver.rfind("vs", 0) == 0) {
    stage = "vs";
  } else if (ver.rfind("ps", 0) == 0) {
    stage = "ps";
  } else if (ver.rfind("gs", 0) == 0) {
    stage = "gs";
  } else if (ver.rfind("cs", 0) == 0) {
    stage = "cs";
  } else if (ver.rfind("hs", 0) == 0) {
    stage = "hs";
  } else if (ver.rfind("ds", 0) == 0) {
    stage = "ds";
  } else if (ver.rfind("lib", 0) == 0) {
    stage = "lib";
  } else {
    return ver;
  }

  if (ver.size() >= stage.size() + 4 && ver[stage.size()] == '_') {
    char major = ver[stage.size() + 1];
    if (major >= '6') {
      return ver;
    }
  }
  return stage + "_6_0";
}

}  // namespace

// ---------------------------------------------------------------------------
// VkVertexShader
// ---------------------------------------------------------------------------

VkVertexShader::VkVertexShader(VulkanDevice* device)
    : VertexShader(device), vk_owner_(device) {}

VkVertexShader::~VkVertexShader() { Release(); }

void VkVertexShader::Release() {
  if (module_ != VK_NULL_HANDLE) {
    vkDestroyShaderModule(vk_owner_->GetVkDevice(), module_, nullptr);
    module_ = VK_NULL_HANDLE;
  }
}

bool VkVertexShader::Create(void* binary, int32_t length) {
  Release();
  return CreateShaderModule(vk_owner_->GetVkDevice(), binary,
                            static_cast<size_t>(length), &module_);
}

bool VkVertexShader::CompileAndCreate(std::string* error) {
  std::vector<uint32_t> spirv;
  if (!CompileHlslToSpirv(code_, entry_function_,
                           ToDxcProfile(shader_version_), &spirv, error)) {
    return false;
  }
  Release();
  return CreateShaderModule(vk_owner_->GetVkDevice(), spirv.data(),
                            spirv.size() * sizeof(uint32_t), &module_);
}

bool VkVertexShader::CreateFromBlob(void* code, int32_t code_size) {
  return Create(code, code_size);
}

bool VkVertexShader::Apply() { return module_ != VK_NULL_HANDLE; }

// ---------------------------------------------------------------------------
// VkPixelShader
// ---------------------------------------------------------------------------

VkPixelShader::VkPixelShader(VulkanDevice* device)
    : PixelShader(device), vk_owner_(device) {}

VkPixelShader::~VkPixelShader() { Release(); }

void VkPixelShader::Release() {
  if (module_ != VK_NULL_HANDLE) {
    vkDestroyShaderModule(vk_owner_->GetVkDevice(), module_, nullptr);
    module_ = VK_NULL_HANDLE;
  }
}

bool VkPixelShader::Create(void* binary, int32_t length) {
  Release();
  return CreateShaderModule(vk_owner_->GetVkDevice(), binary,
                            static_cast<size_t>(length), &module_);
}

bool VkPixelShader::CompileAndCreate(std::string* error) {
  std::vector<uint32_t> spirv;
  if (!CompileHlslToSpirv(code_, entry_function_,
                           ToDxcProfile(shader_version_), &spirv, error)) {
    return false;
  }
  Release();
  return CreateShaderModule(vk_owner_->GetVkDevice(), spirv.data(),
                            spirv.size() * sizeof(uint32_t), &module_);
}

bool VkPixelShader::CreateFromBlob(void* code, int32_t code_size) {
  return Create(code, code_size);
}

bool VkPixelShader::Apply() { return module_ != VK_NULL_HANDLE; }

// ---------------------------------------------------------------------------
// VkGeometryShader
// ---------------------------------------------------------------------------

VkGeometryShader::VkGeometryShader(VulkanDevice* device)
    : GeometryShader(device), vk_owner_(device) {}

VkGeometryShader::~VkGeometryShader() { Release(); }

void VkGeometryShader::Release() {
  if (module_ != VK_NULL_HANDLE) {
    vkDestroyShaderModule(vk_owner_->GetVkDevice(), module_, nullptr);
    module_ = VK_NULL_HANDLE;
  }
}

bool VkGeometryShader::Create(void* binary, int32_t length) {
  Release();
  return CreateShaderModule(vk_owner_->GetVkDevice(), binary,
                            static_cast<size_t>(length), &module_);
}

bool VkGeometryShader::CompileAndCreate(std::string* error) {
  std::vector<uint32_t> spirv;
  if (!CompileHlslToSpirv(code_, entry_function_,
                           ToDxcProfile(shader_version_), &spirv, error)) {
    return false;
  }
  Release();
  return CreateShaderModule(vk_owner_->GetVkDevice(), spirv.data(),
                            spirv.size() * sizeof(uint32_t), &module_);
}

bool VkGeometryShader::CreateFromBlob(void* code, int32_t code_size) {
  return Create(code, code_size);
}

bool VkGeometryShader::Apply() { return module_ != VK_NULL_HANDLE; }

// ---------------------------------------------------------------------------
// VkHullShader
// ---------------------------------------------------------------------------

VkHullShader::VkHullShader(VulkanDevice* device)
    : HullShader(device), vk_owner_(device) {}

VkHullShader::~VkHullShader() { Release(); }

void VkHullShader::Release() {
  if (module_ != VK_NULL_HANDLE) {
    vkDestroyShaderModule(vk_owner_->GetVkDevice(), module_, nullptr);
    module_ = VK_NULL_HANDLE;
  }
}

bool VkHullShader::Create(void* binary, int32_t length) {
  Release();
  return CreateShaderModule(vk_owner_->GetVkDevice(), binary,
                            static_cast<size_t>(length), &module_);
}

bool VkHullShader::CompileAndCreate(std::string* error) {
  std::vector<uint32_t> spirv;
  if (!CompileHlslToSpirv(code_, entry_function_,
                           ToDxcProfile(shader_version_), &spirv, error)) {
    return false;
  }
  Release();
  return CreateShaderModule(vk_owner_->GetVkDevice(), spirv.data(),
                            spirv.size() * sizeof(uint32_t), &module_);
}

bool VkHullShader::CreateFromBlob(void* code, int32_t code_size) {
  return Create(code, code_size);
}

bool VkHullShader::Apply() { return module_ != VK_NULL_HANDLE; }

// ---------------------------------------------------------------------------
// VkDomainShader
// ---------------------------------------------------------------------------

VkDomainShader::VkDomainShader(VulkanDevice* device)
    : DomainShader(device), vk_owner_(device) {}

VkDomainShader::~VkDomainShader() { Release(); }

void VkDomainShader::Release() {
  if (module_ != VK_NULL_HANDLE) {
    vkDestroyShaderModule(vk_owner_->GetVkDevice(), module_, nullptr);
    module_ = VK_NULL_HANDLE;
  }
}

bool VkDomainShader::Create(void* binary, int32_t length) {
  Release();
  return CreateShaderModule(vk_owner_->GetVkDevice(), binary,
                            static_cast<size_t>(length), &module_);
}

bool VkDomainShader::CompileAndCreate(std::string* error) {
  std::vector<uint32_t> spirv;
  if (!CompileHlslToSpirv(code_, entry_function_,
                           ToDxcProfile(shader_version_), &spirv, error)) {
    return false;
  }
  Release();
  return CreateShaderModule(vk_owner_->GetVkDevice(), spirv.data(),
                            spirv.size() * sizeof(uint32_t), &module_);
}

bool VkDomainShader::CreateFromBlob(void* code, int32_t code_size) {
  return Create(code, code_size);
}

bool VkDomainShader::Apply() { return module_ != VK_NULL_HANDLE; }

// ---------------------------------------------------------------------------
// VkComputeShader
// ---------------------------------------------------------------------------

VkComputeShader::VkComputeShader(VulkanDevice* device)
    : ComputeShader(device), vk_owner_(device) {}

VkComputeShader::~VkComputeShader() { Release(); }

void VkComputeShader::Release() {
  if (module_ != VK_NULL_HANDLE) {
    vkDestroyShaderModule(vk_owner_->GetVkDevice(), module_, nullptr);
    module_ = VK_NULL_HANDLE;
  }
}

bool VkComputeShader::Create(void* binary, int32_t length) {
  Release();
  return CreateShaderModule(vk_owner_->GetVkDevice(), binary,
                            static_cast<size_t>(length), &module_);
}

bool VkComputeShader::CompileAndCreate(std::string* error) {
  std::vector<uint32_t> spirv;
  if (!CompileHlslToSpirv(code_, entry_function_,
                           ToDxcProfile(shader_version_), &spirv, error)) {
    return false;
  }
  Release();
  return CreateShaderModule(vk_owner_->GetVkDevice(), spirv.data(),
                            spirv.size() * sizeof(uint32_t), &module_);
}

bool VkComputeShader::CreateFromBlob(void* code, int32_t code_size) {
  return Create(code, code_size);
}

bool VkComputeShader::Apply() { return module_ != VK_NULL_HANDLE; }

}  // namespace renderer
