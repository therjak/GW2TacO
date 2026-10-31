#include "src/core2/vk_shader.h"

#include <atlbase.h>    // CComPtr
#include <dxcapi.h>     // IDxcCompiler3, IDxcUtils, IDxcResult
#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

#include "src/base/logger.h"

// dxcompiler.lib — loaded from the Windows SDK
#pragma comment(lib, "dxcompiler.lib")

namespace renderer {

// ---------------------------------------------------------------------------
// Free helpers
// ---------------------------------------------------------------------------

bool CompileHlslToSpirv(const std::string& hlsl_source,
                        const std::string& entry_point,
                        const std::string& target_profile,
                        std::vector<uint32_t>* spirv_out,
                        std::string* error) {
  spirv_out->clear();
  if (error) error->clear();

  CComPtr<IDxcUtils> utils;
  CComPtr<IDxcCompiler3> compiler;

  if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils))) ||
      FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)))) {
    if (error) *error = "DXC: failed to create compiler instances";
    Log_Err("[vk] DXC: failed to create IDxcUtils / IDxcCompiler3");
    return false;
  }

  // Convert source to wide strings for DXC.
  std::wstring w_entry(entry_point.begin(), entry_point.end());
  std::wstring w_profile(target_profile.begin(), target_profile.end());

  // Build argument list.
  std::vector<LPCWSTR> args = {
      L"-spirv",                      // output SPIR-V
      L"-fspv-target-env=vulkan1.3",  // target Vulkan 1.3
      L"-E", w_entry.c_str(),         // entry point
      L"-T", w_profile.c_str(),       // target profile (vs_6_0, ps_6_0, …)
      L"-fvk-use-dx-layout",          // match HLSL cbuffer layout
  };

#ifdef ENABLE_CORE_DEBUG_MODE
  args.push_back(L"-Zi");    // debug info
  args.push_back(L"-Od");    // no optimisation in debug
#else
  args.push_back(L"-O3");
#endif

  DxcBuffer source_buf{};
  source_buf.Ptr = hlsl_source.data();
  source_buf.Size = hlsl_source.size();
  source_buf.Encoding = DXC_CP_UTF8;

  CComPtr<IDxcResult> result;
  HRESULT hr = compiler->Compile(&source_buf, args.data(),
                                 static_cast<uint32_t>(args.size()),
                                 nullptr, IID_PPV_ARGS(&result));
  if (FAILED(hr)) {
    if (error) *error = "DXC: Compile() call failed (HRESULT)";
    return false;
  }

  // Retrieve error/warning buffer.
  CComPtr<IDxcBlobUtf8> errors_blob;
  result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors_blob), nullptr);
  if (errors_blob && errors_blob->GetStringLength() > 0) {
    std::string msg(errors_blob->GetStringPointer(),
                    errors_blob->GetStringLength());
    if (error) *error = msg;
    Log_Warn("[vk] DXC: {}", msg);
  }

  HRESULT status;
  result->GetStatus(&status);
  if (FAILED(status)) {
    if (error && error->empty()) *error = "DXC: compilation failed";
    return false;
  }

  CComPtr<IDxcBlob> spirv_blob;
  result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&spirv_blob), nullptr);
  if (!spirv_blob || spirv_blob->GetBufferSize() == 0) {
    if (error) *error = "DXC: empty SPIR-V output";
    return false;
  }

  // Copy SPIR-V words into output vector.
  const size_t word_count = spirv_blob->GetBufferSize() / sizeof(uint32_t);
  spirv_out->resize(word_count);
  memcpy(spirv_out->data(), spirv_blob->GetBufferPointer(),
         spirv_blob->GetBufferSize());
  return true;
}

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
