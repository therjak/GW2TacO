#pragma once
#include <cstdint>
#include <string>

#include "src/core2/shader.h"
#include "src/core2/vk_device.h"

namespace renderer {

class VkVertexShader : public VertexShader {
 public:
  explicit VkVertexShader(VulkanDevice* device);
  ~VkVertexShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return nullptr; }

 private:
  bool Apply() override;
};

class VkPixelShader : public PixelShader {
 public:
  explicit VkPixelShader(VulkanDevice* device);
  ~VkPixelShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return nullptr; }

 private:
  bool Apply() override;
};

class VkGeometryShader : public GeometryShader {
 public:
  explicit VkGeometryShader(VulkanDevice* device);
  ~VkGeometryShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return nullptr; }

 private:
  bool Apply() override;
};

class VkHullShader : public HullShader {
 public:
  explicit VkHullShader(VulkanDevice* device);
  ~VkHullShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return nullptr; }

 private:
  bool Apply() override;
};

class VkDomainShader : public DomainShader {
 public:
  explicit VkDomainShader(VulkanDevice* device);
  ~VkDomainShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return nullptr; }

 private:
  bool Apply() override;
};

class VkComputeShader : public ComputeShader {
 public:
  explicit VkComputeShader(VulkanDevice* device);
  ~VkComputeShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return nullptr; }

 private:
  bool Apply() override;
};

}  // namespace renderer
