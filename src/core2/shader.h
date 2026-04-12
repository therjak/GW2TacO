#pragma once
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

#include "src/core2/resource.h"

namespace renderer {

class CCoreShader : public CCoreResource {
  friend class CCoreDevice;

 public:
  explicit CCoreShader(CCoreDevice* device) : CCoreResource(device) {
    binary_length_ = 0;
  };
  ~CCoreShader() override;

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

class CCorePixelShader : public CCoreShader {
 public:
  explicit CCorePixelShader(CCoreDevice* device) : CCoreShader(device){};
  ~CCorePixelShader() override;
  void* GetHandle() override = 0;
};

class CCoreGeometryShader : public CCoreShader {
 public:
  explicit CCoreGeometryShader(CCoreDevice* device) : CCoreShader(device){};
  ~CCoreGeometryShader() override;
};

class CCoreVertexShader : public CCoreShader {
 public:
  explicit CCoreVertexShader(CCoreDevice* device) : CCoreShader(device){};
  ~CCoreVertexShader() override;
};

class CCoreHullShader : public CCoreShader {
 public:
  explicit CCoreHullShader(CCoreDevice* device) : CCoreShader(device){};
  ~CCoreHullShader() override;
  void* GetHandle() override = 0;
};

class CCoreDomainShader : public CCoreShader {
 public:
  explicit CCoreDomainShader(CCoreDevice* device) : CCoreShader(device){};
  ~CCoreDomainShader() override;
  void* GetHandle() override = 0;
};

class CCoreComputeShader : public CCoreShader {
 public:
  explicit CCoreComputeShader(CCoreDevice* device) : CCoreShader(device){};
  ~CCoreComputeShader() override;
  void* GetHandle() override = 0;
};

}  // namespace renderer
