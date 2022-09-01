#pragma once

#include <vector>

#include "src/core2/dx11_device.h"
#include "src/core2/vertex_format.h"

class CCoreDX11VertexFormat : public CCoreVertexFormat {
 public:
  explicit CCoreDX11VertexFormat(CCoreDX11Device* dev);
  ~CCoreDX11VertexFormat() override;

  bool Create(const std::vector<COREVERTEXATTRIBUTE>& Attributes,
              CCoreVertexShader* vs = nullptr) override;
  int32_t GetSize() override;

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11InputLayout* VertexFormatHandle;
  int32_t Size;
};
