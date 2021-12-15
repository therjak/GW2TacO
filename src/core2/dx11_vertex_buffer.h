#pragma once
#include "src/core2/dx11_device.h"
#include "src/core2/vertex_buffer.h"

#ifdef CORE_API_DX11

class CCoreDX11VertexBuffer : public CCoreVertexBuffer {
  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11Buffer* VertexBufferHandle;

  int32_t Size;
  bool Dynamic;

  virtual void Release();
  bool Apply(const uint32_t Offset) override;

 public:
  CCoreDX11VertexBuffer(CCoreDX11Device* Device);
  ~CCoreDX11VertexBuffer() override;

  bool Create(const uint8_t* Data, const uint32_t Size) override;
  bool CreateDynamic(const uint32_t Size) override;
  bool Update(const int32_t Offset, const uint8_t* Data,
              const uint32_t Size) override;
  bool Lock(void** Result) override;
  bool Lock(void** Result, const uint32_t Offset, const int32_t size,
            const int32_t Flags = 0) override;
  bool UnLock() override;
  void* GetHandle() override { return VertexBufferHandle; }
};

#endif
