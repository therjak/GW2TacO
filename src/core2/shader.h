#pragma once
#include <memory>
#include <string>

#include "src/core2/resource.h"

#define CORESHADERTYPECOUNT 5

class CCoreShader : public CCoreResource {
  friend class CCoreDevice;

 public:
  explicit CCoreShader(CCoreDevice* Device) : CCoreResource(Device) {
    BinaryLength = 0;
  };
  ~CCoreShader() override;

  virtual bool Create(void* Binary, int32_t Length) = 0;

  void* GetBinary() { return Binary.get(); }

  int32_t GetBinaryLength() { return BinaryLength; }

  void SetCode(std::string_view code, std::string_view entry,
               std::string_view shaderversion) {
    Code = code;
    EntryFunction = entry;
    ShaderVersion = shaderversion;
  }

  std::string& GetCode() { return Code; }

  virtual bool CompileAndCreate(std::string* Err) = 0;
  virtual bool CreateFromBlob(void* Code, int32_t CodeSize) = 0;

  virtual void* GetHandle() = 0;

 protected:
  void FetchBinary(void* binary, int32_t length) {
    Binary = std::make_unique<uint8_t[]>(length);
    memcpy(Binary.get(), binary, length);
    BinaryLength = length;
  }

  std::string Code;
  std::string EntryFunction;
  std::string ShaderVersion;

 private:
  virtual bool Apply() = 0;

  std::unique_ptr<uint8_t[]> Binary;
  int32_t BinaryLength;
};

class CCorePixelShader : public CCoreShader {
 public:
  explicit CCorePixelShader(CCoreDevice* Device) : CCoreShader(Device){};
  ~CCorePixelShader() override;
  void* GetHandle() override = 0;
};

class CCoreGeometryShader : public CCoreShader {
 public:
  explicit CCoreGeometryShader(CCoreDevice* Device) : CCoreShader(Device){};
  ~CCoreGeometryShader() override;
};

class CCoreVertexShader : public CCoreShader {
 public:
  explicit CCoreVertexShader(CCoreDevice* Device) : CCoreShader(Device){};
  ~CCoreVertexShader() override;
};

class CCoreHullShader : public CCoreShader {
 public:
  explicit CCoreHullShader(CCoreDevice* Device) : CCoreShader(Device){};
  ~CCoreHullShader() override;
  void* GetHandle() override = 0;
};

class CCoreDomainShader : public CCoreShader {
 public:
  explicit CCoreDomainShader(CCoreDevice* Device) : CCoreShader(Device){};
  ~CCoreDomainShader() override;
  void* GetHandle() override = 0;
};

class CCoreComputeShader : public CCoreShader {
 public:
  explicit CCoreComputeShader(CCoreDevice* Device) : CCoreShader(Device){};
  ~CCoreComputeShader() override;
  void* GetHandle() override = 0;
};
