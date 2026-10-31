#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "src/core2/shader.h"
#include "src/core2/vk_device.h"

namespace renderer {

// Compiles HLSL source to SPIR-V using the DXC COM API.
// Returns true on success; fills |spirv_out| with the SPIR-V words and
// optionally writes a human-readable error message into |error|.
bool CompileHlslToSpirv(const std::string& hlsl_source,
                        const std::string& entry_point,
                        const std::string& target_profile,
                        std::vector<uint32_t>* spirv_out,
                        std::string* error);

// Creates a VkShaderModule from raw SPIR-V bytes (size in bytes, not words).
bool CreateShaderModule(VkDevice device, const void* spirv_data,
                        size_t spirv_bytes, VkShaderModule* out_module);

// ---------------------------------------------------------------------------
// Concrete Vulkan shader classes
// ---------------------------------------------------------------------------

class VkVertexShader : public VertexShader {
 public:
  explicit VkVertexShader(VulkanDevice* device);
  ~VkVertexShader() override;

  // Create from pre-compiled SPIR-V binary.
  bool Create(void* binary, int32_t length) override;
  // Compile code_ / entry_function_ / shader_version_ → SPIR-V → module.
  bool CompileAndCreate(std::string* error) override;
  // Create from raw SPIR-V blob.
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return module_; }

 private:
  bool Apply() override;
  void Release();

  VulkanDevice* vk_owner_;
  VkShaderModule module_ = VK_NULL_HANDLE;
};

class VkPixelShader : public PixelShader {
 public:
  explicit VkPixelShader(VulkanDevice* device);
  ~VkPixelShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return module_; }

 private:
  bool Apply() override;
  void Release();

  VulkanDevice* vk_owner_;
  VkShaderModule module_ = VK_NULL_HANDLE;
};

class VkGeometryShader : public GeometryShader {
 public:
  explicit VkGeometryShader(VulkanDevice* device);
  ~VkGeometryShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return module_; }

 private:
  bool Apply() override;
  void Release();

  VulkanDevice* vk_owner_;
  VkShaderModule module_ = VK_NULL_HANDLE;
};

class VkHullShader : public HullShader {
 public:
  explicit VkHullShader(VulkanDevice* device);
  ~VkHullShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return module_; }

 private:
  bool Apply() override;
  void Release();

  VulkanDevice* vk_owner_;
  VkShaderModule module_ = VK_NULL_HANDLE;
};

class VkDomainShader : public DomainShader {
 public:
  explicit VkDomainShader(VulkanDevice* device);
  ~VkDomainShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return module_; }

 private:
  bool Apply() override;
  void Release();

  VulkanDevice* vk_owner_;
  VkShaderModule module_ = VK_NULL_HANDLE;
};

class VkComputeShader : public ComputeShader {
 public:
  explicit VkComputeShader(VulkanDevice* device);
  ~VkComputeShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return module_; }

 private:
  bool Apply() override;
  void Release();

  VulkanDevice* vk_owner_;
  VkShaderModule module_ = VK_NULL_HANDLE;
};

}  // namespace renderer
