#pragma once
#include "src/core2/dx11_device.h"
#include "src/core2/index_buffer.h"

class CCoreDX11IndexBuffer : public CCoreIndexBuffer {
  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11Buffer* IndexBufferHandle;

  int32_t IndexCount;
  int32_t IndexSize;

  virtual void Release();
  bool Apply() override;

 public:
  CCoreDX11IndexBuffer(CCoreDX11Device* dev);
  ~CCoreDX11IndexBuffer() override;

  bool Create(const uint32_t IndexCount, const uint32_t IndexSize = 2) override;
  bool Lock(void** Result) override;
  bool Lock(void** Result, const uint32_t IndexOffset,
            const int32_t IndexCount) override;
  bool UnLock() override;
  void* GetHandle() override { return IndexBufferHandle; }
};
