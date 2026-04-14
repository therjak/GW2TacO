#pragma once
#include <d3d11.h>

#include <cstdint>
#include <string>

#include "src/core2/dx11_device.h"
#include "src/core2/shader.h"

namespace renderer {

class DX11VertexShader : public VertexShader {
 public:
  explicit DX11VertexShader(DX11Device* device);
  ~DX11VertexShader() override;

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

class DX11PixelShader : public PixelShader {
 public:
  explicit DX11PixelShader(DX11Device* device);
  ~DX11PixelShader() override;

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

class DX11GeometryShader : public GeometryShader {
 public:
  explicit DX11GeometryShader(DX11Device* device);
  ~DX11GeometryShader() override;

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

class DX11HullShader : public HullShader {
 public:
  explicit DX11HullShader(DX11Device* device);
  ~DX11HullShader() override;

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

class DX11DomainShader : public DomainShader {
 public:
  explicit DX11DomainShader(DX11Device* device);
  ~DX11DomainShader() override;

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

class DX11ComputeShader : public ComputeShader {
 public:
  explicit DX11ComputeShader(DX11Device* device);
  ~DX11ComputeShader() override;

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
