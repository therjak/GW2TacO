#pragma once
#include "DX11Device.h"
#include "IndexBuffer.h"

#ifdef CORE_API_DX11

class CCoreDX11IndexBuffer : public CCoreIndexBuffer {
  ID3D11Device *Dev;
  ID3D11DeviceContext *DeviceContext;
  ID3D11Buffer *IndexBufferHandle;

  int32_t IndexCount;
  int32_t IndexSize;

  virtual void Release();
  TBOOL Apply() override;

 public:
  CCoreDX11IndexBuffer(CCoreDX11Device *dev);
  ~CCoreDX11IndexBuffer() override;

  TBOOL Create(const uint32_t IndexCount,
               const uint32_t IndexSize = 2) override;
  TBOOL Lock(void **Result) override;
  TBOOL Lock(void **Result, const uint32_t IndexOffset,
             const int32_t IndexCount) override;
  TBOOL UnLock() override;
  void *GetHandle() override { return IndexBufferHandle; }
};

#endif
