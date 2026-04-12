#pragma once

#include <d3d11.h>

#include <cstdint>
#include <vector>

#include "src/core2/dx11_device.h"
#include "src/core2/vertex_format.h"

namespace renderer {

class CCoreDX11VertexFormat : public CCoreVertexFormat {
 public:
  explicit CCoreDX11VertexFormat(CCoreDX11Device* device);
  ~CCoreDX11VertexFormat() override;

  bool Create(const std::vector<CoreVertexAttribute>& attributes,
              CCoreVertexShader* vertex_shader = nullptr) override;
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
