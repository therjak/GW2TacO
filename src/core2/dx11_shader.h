#pragma once
#include <d3d11.h>

#include <cstdint>
#include <string>

#include "src/core2/dx11_device.h"
#include "src/core2/shader.h"

namespace renderer {

class CCoreDX11VertexShader : public CCoreVertexShader {
 public:
  explicit CCoreDX11VertexShader(CCoreDX11Device* device);
  ~CCoreDX11VertexShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return vertex_shader_handle_; }

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* d3d_device_context_;
  ID3D11VertexShader* vertex_shader_handle_;
};

class CCoreDX11PixelShader : public CCorePixelShader {
 public:
  explicit CCoreDX11PixelShader(CCoreDX11Device* device);
  ~CCoreDX11PixelShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return pixel_shader_handle_; }

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* d3d_device_context_;
  ID3D11PixelShader* pixel_shader_handle_;
};

class CCoreDX11GeometryShader : public CCoreGeometryShader {
 public:
  explicit CCoreDX11GeometryShader(CCoreDX11Device* device);
  ~CCoreDX11GeometryShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return geometry_shader_handle_; }

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* d3d_device_context_;
  ID3D11GeometryShader* geometry_shader_handle_;
};

class CCoreDX11HullShader : public CCoreHullShader {
 public:
  explicit CCoreDX11HullShader(CCoreDX11Device* device);
  ~CCoreDX11HullShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return hull_shader_handle_; }

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* d3d_device_context_;
  ID3D11HullShader* hull_shader_handle_;
};

class CCoreDX11DomainShader : public CCoreDomainShader {
 public:
  explicit CCoreDX11DomainShader(CCoreDX11Device* device);
  ~CCoreDX11DomainShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return domain_shader_handle_; }

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* d3d_device_context_;
  ID3D11DomainShader* domain_shader_handle_;
};

class CCoreDX11ComputeShader : public CCoreComputeShader {
 public:
  explicit CCoreDX11ComputeShader(CCoreDX11Device* device);
  ~CCoreDX11ComputeShader() override;

  bool Create(void* binary, int32_t length) override;
  bool CompileAndCreate(std::string* error) override;
  bool CreateFromBlob(void* code, int32_t code_size) override;
  void* GetHandle() override { return compute_shader_handle_; }

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* d3d_device_context_;
  ID3D11ComputeShader* compute_shader_handle_;
};

}  // namespace renderer
