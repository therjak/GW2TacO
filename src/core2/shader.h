#pragma once
#include <memory>

#include "src/core2/resource.h"

#define CORESHADERTYPECOUNT 5

class CCoreShader : public CCoreResource {
  friend class CCoreDevice;
  virtual bool Apply() = 0;

  std::unique_ptr<uint8_t[]> Binary;
  int32_t BinaryLength;

 protected:
  std::string Code;
  std::string EntryFunction;
  std::string ShaderVersion;

  INLINE void FetchBinary(void* binary, int32_t length) {
    Binary = std::make_unique<uint8_t[]>(length);
    memcpy(Binary.get(), binary, length);
    BinaryLength = length;
  }

 public:
  INLINE explicit CCoreShader(CCoreDevice* Device) : CCoreResource(Device) {
    BinaryLength = 0;
  };
  ~CCoreShader() override;

  virtual bool Create(void* Binary, int32_t Length) = 0;

  INLINE void* GetBinary() { return Binary.get(); }

  INLINE int32_t GetBinaryLength() { return BinaryLength; }

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
};

class CCorePixelShader : public CCoreShader {
 public:
  INLINE explicit CCorePixelShader(CCoreDevice* Device) : CCoreShader(Device){};
  ~CCorePixelShader() override;
  void* GetHandle() override = 0;
};

class CCoreGeometryShader : public CCoreShader {
 public:
  INLINE explicit CCoreGeometryShader(CCoreDevice* Device)
      : CCoreShader(Device){};
  ~CCoreGeometryShader() override;
};

class CCoreVertexShader : public CCoreShader {
 public:
  INLINE explicit CCoreVertexShader(CCoreDevice* Device)
      : CCoreShader(Device){};
  ~CCoreVertexShader() override;
};

class CCoreHullShader : public CCoreShader {
 public:
  INLINE explicit CCoreHullShader(CCoreDevice* Device) : CCoreShader(Device){};
  ~CCoreHullShader() override;
  void* GetHandle() override = 0;
};

class CCoreDomainShader : public CCoreShader {
 public:
  INLINE explicit CCoreDomainShader(CCoreDevice* Device)
      : CCoreShader(Device){};
  ~CCoreDomainShader() override;
  void* GetHandle() override = 0;
};

class CCoreComputeShader : public CCoreShader {
 public:
  INLINE explicit CCoreComputeShader(CCoreDevice* Device)
      : CCoreShader(Device){};
  ~CCoreComputeShader() override;
  void* GetHandle() override = 0;
};
