#pragma once
#include "src/core2/constant_buffer.h"
#include "src/core2/dx11_device.h"

class CCoreDX11ConstantBuffer : public CCoreConstantBuffer {
 public:
  explicit CCoreDX11ConstantBuffer(CCoreDX11Device* Device);
  ~CCoreDX11ConstantBuffer() override;

  void Upload() override;
  [[nodiscard]] void* GetBufferPointer() const override;

 private:
  ID3D11Buffer* Buffer;
  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  int32_t AllocatedBufferSize;
};
