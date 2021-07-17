#pragma once
#include "ConstantBuffer.h"
#include "DX11Device.h"

#ifdef CORE_API_DX11
class CCoreDX11ConstantBuffer : public CCoreConstantBuffer {
  ID3D11Buffer *Buffer;
  ID3D11Device *Dev;
  ID3D11DeviceContext *DeviceContext;
  int32_t AllocatedBufferSize;

 public:
  CCoreDX11ConstantBuffer(CCoreDX11Device *Device);
  ~CCoreDX11ConstantBuffer() override;

  void Upload() override;
  void *GetBufferPointer() const override;
};
#endif
