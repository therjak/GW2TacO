#pragma once
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

#include "src/core2/resource.h"

namespace renderer {

class Shader : public Resource {
  friend class Device;

 public:
  explicit Shader(Device* device) : Resource(device) { binary_length_ = 0; }
  ~Shader() override;

  virtual bool Create(void* binary, int32_t length) = 0;

  const uint8_t* GetBinary() { return binary_.get(); }

  int32_t GetBinaryLength() { return binary_length_; }

  void SetCode(std::string_view code, std::string_view entry,
               std::string_view shader_version) {
    code_ = code;
    entry_function_ = entry;
    shader_version_ = shader_version;
  }

  std::string& GetCode() { return code_; }

  virtual bool CompileAndCreate(std::string* err) = 0;
  virtual bool CreateFromBlob(void* code, int32_t code_size) = 0;

  virtual void* GetHandle() = 0;

 protected:
  void FetchBinary(const void* binary, int32_t length) {
    binary_ = std::make_unique<uint8_t[]>(length);
    memcpy(binary_.get(), binary, length);
    binary_length_ = length;
  }

  std::string code_;
  std::string entry_function_;
  std::string shader_version_;

 private:
  virtual bool Apply() = 0;

  std::unique_ptr<uint8_t[]> binary_;
  int32_t binary_length_;
};

class PixelShader : public Shader {
 public:
  explicit PixelShader(Device* device) : Shader(device) {}
  ~PixelShader() override;
  void* GetHandle() override = 0;
};

class GeometryShader : public Shader {
 public:
  explicit GeometryShader(Device* device) : Shader(device) {}
  ~GeometryShader() override;
};

class VertexShader : public Shader {
 public:
  explicit VertexShader(Device* device) : Shader(device) {}
  ~VertexShader() override;
};

class HullShader : public Shader {
 public:
  explicit HullShader(Device* device) : Shader(device) {}
  ~HullShader() override;
  void* GetHandle() override = 0;
};

class DomainShader : public Shader {
 public:
  explicit DomainShader(Device* device) : Shader(device) {}
  ~DomainShader() override;
  void* GetHandle() override = 0;
};

class ComputeShader : public Shader {
 public:
  explicit ComputeShader(Device* device) : Shader(device) {}
  ~ComputeShader() override;
  void* GetHandle() override = 0;
};

}  // namespace renderer
