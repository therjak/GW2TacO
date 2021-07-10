#pragma once
#include "Resource.h"

#define CORESHADERTYPECOUNT 5

enum CORESHADERTYPE {
  CORE_SHADER_VERTEX = 0,
  CORE_SHADER_PIXEL = 1,
  CORE_SHADER_GEOMETRY = 2,
  CORE_SHADER_HULL = 3,
  CORE_SHADER_DOMAIN = 4,
};

class CCoreShader : public CCoreResource {
  friend class CCoreDevice;
  virtual TBOOL Apply() = 0;

  void *Binary;
  int32_t BinaryLength;

 protected:
  std::string Code;
  std::string EntryFunction;
  std::string ShaderVersion;

  INLINE void FetchBinary(void *binary, int32_t length) {
    Binary = new uint8_t[length];
    memcpy(Binary, binary, length);
    BinaryLength = length;
  }

 public:
  INLINE CCoreShader(CCoreDevice *Device) : CCoreResource(Device) {
    Binary = NULL;
    BinaryLength = 0;
  };
  virtual ~CCoreShader();

  virtual TBOOL Create(void *Binary, int32_t Length) = 0;

  INLINE void *GetBinary() { return Binary; }

  INLINE int32_t GetBinaryLength() { return BinaryLength; }

  void SetCode(std::string_view code, std::string_view entry,
               std::string_view shaderversion) {
    Code = code;
    EntryFunction = entry;
    ShaderVersion = shaderversion;
  }

  std::string &GetCode() { return Code; }

  virtual TBOOL CompileAndCreate(std::string *Err) = 0;
  virtual TBOOL CreateFromBlob(void *Code, int32_t CodeSize) = 0;

  // virtual int32_t GetConstantBufferIndex(TS8 *Name) = 0;
  virtual void *GetHandle() = 0;
};

class CCorePixelShader : public CCoreShader {
 public:
  INLINE CCorePixelShader(CCoreDevice *Device) : CCoreShader(Device){};
  virtual ~CCorePixelShader();
  virtual void *GetHandle() = 0;
};

class CCoreGeometryShader : public CCoreShader {
 public:
  INLINE CCoreGeometryShader(CCoreDevice *Device) : CCoreShader(Device){};
  virtual ~CCoreGeometryShader();
};

class CCoreVertexShader : public CCoreShader {
 public:
  INLINE CCoreVertexShader(CCoreDevice *Device) : CCoreShader(Device){};
  virtual ~CCoreVertexShader();
};

class CCoreHullShader : public CCoreShader {
 public:
  INLINE CCoreHullShader(CCoreDevice *Device) : CCoreShader(Device){};
  virtual ~CCoreHullShader();
  virtual void *GetHandle() = 0;
};

class CCoreDomainShader : public CCoreShader {
 public:
  INLINE CCoreDomainShader(CCoreDevice *Device) : CCoreShader(Device){};
  virtual ~CCoreDomainShader();
  virtual void *GetHandle() = 0;
};

class CCoreComputeShader : public CCoreShader {
 public:
  INLINE CCoreComputeShader(CCoreDevice *Device) : CCoreShader(Device){};
  virtual ~CCoreComputeShader();
  virtual void *GetHandle() = 0;
};
