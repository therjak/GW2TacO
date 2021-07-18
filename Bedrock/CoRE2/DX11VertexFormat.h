#pragma once
#include "DX11Device.h"
#include "VertexFormat.h"

#ifdef CORE_API_DX11

class CCoreDX11VertexFormat : public CCoreVertexFormat {
  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11InputLayout* VertexFormatHandle;
  int32_t Size;

  virtual void Release();
  bool Apply() override;

 public:
  CCoreDX11VertexFormat(CCoreDX11Device* dev);
  ~CCoreDX11VertexFormat() override;

  bool Create(const CArray<COREVERTEXATTRIBUTE>& Attributes,
              CCoreVertexShader* vs = nullptr) override;
  int32_t GetSize() override;
};

#endif
