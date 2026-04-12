#pragma once

#include <d3d11.h>

#include <cstdint>

#include "src/core2/constant_buffer.h"
#include "src/core2/dx11_device.h"

namespace renderer {

class CCoreDX11ConstantBuffer : public CCoreConstantBuffer {
 public:
  explicit CCoreDX11ConstantBuffer(CCoreDX11Device* device);
  ~CCoreDX11ConstantBuffer() override;

  void Upload() override;
  [[nodiscard]] void* GetBufferPointer() const override;

 private:
  ID3D11Buffer* buffer_ = nullptr;
  ID3D11Device* d3d_device_ = nullptr;
  ID3D11DeviceContext* d3d_device_context_ = nullptr;
  int32_t allocated_buffer_size_ = 0;
};

}  // namespace renderer
