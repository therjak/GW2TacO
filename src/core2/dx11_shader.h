#pragma once
#include "src/core2/dx11_device.h"
#include "src/core2/shader.h"

class CCoreDX11VertexShader : public CCoreVertexShader {
  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11VertexShader* VertexShaderHandle;

  virtual void Release();
  bool Apply() override;

 public:
  explicit CCoreDX11VertexShader(CCoreDX11Device* Device);
  ~CCoreDX11VertexShader() override;

  bool Create(void* Binary, int32_t Length) override;
  bool CompileAndCreate(std::string* Err) override;
  bool CreateFromBlob(void* Code, int32_t CodeSize) override;
  void* GetHandle() override { return VertexShaderHandle; }
};

class CCoreDX11PixelShader : public CCorePixelShader {
  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11PixelShader* PixelShaderHandle;

  virtual void Release();
  bool Apply() override;

 public:
  explicit CCoreDX11PixelShader(CCoreDX11Device* Device);
  ~CCoreDX11PixelShader() override;

  bool Create(void* Binary, int32_t Length) override;
  bool CompileAndCreate(std::string* Err) override;
  bool CreateFromBlob(void* Code, int32_t CodeSize) override;
  void* GetHandle() override { return PixelShaderHandle; }
};

class CCoreDX11GeometryShader : public CCoreGeometryShader {
  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11GeometryShader* GeometryShaderHandle;

  virtual void Release();
  bool Apply() override;

 public:
  explicit CCoreDX11GeometryShader(CCoreDX11Device* Device);
  ~CCoreDX11GeometryShader() override;

  bool Create(void* Binary, int32_t Length) override;
  bool CompileAndCreate(std::string* Err) override;
  bool CreateFromBlob(void* Code, int32_t CodeSize) override;
  void* GetHandle() override { return GeometryShaderHandle; }
};

class CCoreDX11HullShader : public CCoreHullShader {
  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11HullShader* HullShaderHandle;

  virtual void Release();
  bool Apply() override;

 public:
  explicit CCoreDX11HullShader(CCoreDX11Device* Device);
  ~CCoreDX11HullShader() override;

  bool Create(void* Binary, int32_t Length) override;
  bool CompileAndCreate(std::string* Err) override;
  bool CreateFromBlob(void* Code, int32_t CodeSize) override;
  void* GetHandle() override { return HullShaderHandle; }
};

class CCoreDX11DomainShader : public CCoreDomainShader {
  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11DomainShader* DomainShaderHandle;

  virtual void Release();
  bool Apply() override;

 public:
  explicit CCoreDX11DomainShader(CCoreDX11Device* Device);
  ~CCoreDX11DomainShader() override;

  bool Create(void* Binary, int32_t Length) override;
  bool CompileAndCreate(std::string* Err) override;
  bool CreateFromBlob(void* Code, int32_t CodeSize) override;
  void* GetHandle() override { return DomainShaderHandle; }
};

class CCoreDX11ComputeShader : public CCoreComputeShader {
  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11ComputeShader* ComputeShaderHandle;

  virtual void Release();
  bool Apply() override;

 public:
  explicit CCoreDX11ComputeShader(CCoreDX11Device* Device);
  ~CCoreDX11ComputeShader() override;

  bool Create(void* Binary, int32_t Length) override;
  bool CompileAndCreate(std::string* Err) override;
  bool CreateFromBlob(void* Code, int32_t CodeSize) override;
  void* GetHandle() override { return ComputeShaderHandle; }
};
