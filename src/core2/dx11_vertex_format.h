#pragma once

#include <d3d11.h>

#include <cstdint>
#include <vector>

#include "src/core2/dx11_device.h"
#include "src/core2/vertex_format.h"

namespace renderer {

class DX11VertexFormat : public VertexFormat {
 public:
  explicit DX11VertexFormat(DX11Device* device);
  ~DX11VertexFormat() override;

  bool Create(const std::vector<VertexAttribute>& attributes,
              VertexShader* vertex_shader = nullptr) override;
  int32_t GetSize() override;

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* d3d_device_context_;
  ID3D11InputLayout* vertex_format_handle_;
  int32_t size_;
};

}  // namespace renderer
