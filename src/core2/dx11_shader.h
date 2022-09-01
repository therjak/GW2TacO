#pragma once
#include "src/core2/dx11_device.h"
#include "src/core2/shader.h"

class CCoreDX11VertexShader : public CCoreVertexShader {
 public:
  explicit CCoreDX11VertexShader(CCoreDX11Device* Device);
  ~CCoreDX11VertexShader() override;

  bool Create(void* Binary, int32_t Length) override;
  bool CompileAndCreate(std::string* Err) override;
  bool CreateFromBlob(void* Code, int32_t CodeSize) override;
  void* GetHandle() override { return VertexShaderHandle; }

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11VertexShader* VertexShaderHandle;
};

class CCoreDX11PixelShader : public CCorePixelShader {
 public:
  explicit CCoreDX11PixelShader(CCoreDX11Device* Device);
  ~CCoreDX11PixelShader() override;

  bool Create(void* Binary, int32_t Length) override;
  bool CompileAndCreate(std::string* Err) override;
  bool CreateFromBlob(void* Code, int32_t CodeSize) override;
  void* GetHandle() override { return PixelShaderHandle; }

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11PixelShader* PixelShaderHandle;
};

class CCoreDX11GeometryShader : public CCoreGeometryShader {
 public:
  explicit CCoreDX11GeometryShader(CCoreDX11Device* Device);
  ~CCoreDX11GeometryShader() override;

  bool Create(void* Binary, int32_t Length) override;
  bool CompileAndCreate(std::string* Err) override;
  bool CreateFromBlob(void* Code, int32_t CodeSize) override;
  void* GetHandle() override { return GeometryShaderHandle; }

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11GeometryShader* GeometryShaderHandle;
};

class CCoreDX11HullShader : public CCoreHullShader {
 public:
  explicit CCoreDX11HullShader(CCoreDX11Device* Device);
  ~CCoreDX11HullShader() override;

  bool Create(void* Binary, int32_t Length) override;
  bool CompileAndCreate(std::string* Err) override;
  bool CreateFromBlob(void* Code, int32_t CodeSize) override;
  void* GetHandle() override { return HullShaderHandle; }

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11HullShader* HullShaderHandle;
};

class CCoreDX11DomainShader : public CCoreDomainShader {
 public:
  explicit CCoreDX11DomainShader(CCoreDX11Device* Device);
  ~CCoreDX11DomainShader() override;

  bool Create(void* Binary, int32_t Length) override;
  bool CompileAndCreate(std::string* Err) override;
  bool CreateFromBlob(void* Code, int32_t CodeSize) override;
  void* GetHandle() override { return DomainShaderHandle; }

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11DomainShader* DomainShaderHandle;
};

class CCoreDX11ComputeShader : public CCoreComputeShader {
 public:
  explicit CCoreDX11ComputeShader(CCoreDX11Device* Device);
  ~CCoreDX11ComputeShader() override;

  bool Create(void* Binary, int32_t Length) override;
  bool CompileAndCreate(std::string* Err) override;
  bool CreateFromBlob(void* Code, int32_t CodeSize) override;
  void* GetHandle() override { return ComputeShaderHandle; }

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11ComputeShader* ComputeShaderHandle;
};
