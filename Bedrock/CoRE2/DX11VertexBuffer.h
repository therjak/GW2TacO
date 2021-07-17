#pragma once
#include "DX11Device.h"
#include "VertexBuffer.h"

#ifdef CORE_API_DX11

class CCoreDX11VertexBuffer : public CCoreVertexBuffer {
  ID3D11Device *Dev;
  ID3D11DeviceContext *DeviceContext;
  ID3D11Buffer *VertexBufferHandle;

  int32_t Size;
  TBOOL Dynamic;

  virtual void Release();
  TBOOL Apply(const uint32_t Offset) override;

 public:
  CCoreDX11VertexBuffer(CCoreDX11Device *Device);
  ~CCoreDX11VertexBuffer() override;

  TBOOL Create(const uint8_t *Data, const uint32_t Size) override;
  TBOOL CreateDynamic(const uint32_t Size) override;
  TBOOL Update(const int32_t Offset, const uint8_t *Data,
               const uint32_t Size) override;
  TBOOL Lock(void **Result) override;
  TBOOL Lock(void **Result, const uint32_t Offset, const int32_t size,
             const int32_t Flags = 0) override;
  TBOOL UnLock() override;
  void *GetHandle() override { return VertexBufferHandle; }
};

#endif
