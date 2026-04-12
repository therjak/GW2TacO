#pragma once
#include <d3d11.h>

#include <cstdint>

#include "src/core2/dx11_device.h"
#include "src/core2/index_buffer.h"

namespace renderer {

class CCoreDX11IndexBuffer : public CCoreIndexBuffer {
 public:
  explicit CCoreDX11IndexBuffer(CCoreDX11Device* device);
  ~CCoreDX11IndexBuffer() override;

  bool Create(const uint32_t index_count, const uint32_t index_size = 2) override;
  bool Lock(void** result) override;
  bool Lock(void** result, const uint32_t index_offset,
            const int32_t index_count) override;
  bool UnLock() override;
  void* GetHandle() override { return index_buffer_handle_; }

 private:
  virtual void Release();
  bool Apply() override;

  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* d3d_device_context_;
  ID3D11Buffer* index_buffer_handle_;

  int32_t index_count_;
  int32_t index_size_;
};

}  // namespace renderer
