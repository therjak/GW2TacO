#pragma once
#include <d3d11.h>

#include <cstdint>

#include "src/core2/dx11_device.h"
#include "src/core2/vertex_buffer.h"

namespace renderer {

class CCoreDX11VertexBuffer : public CCoreVertexBuffer {
 public:
  explicit CCoreDX11VertexBuffer(CCoreDX11Device* device);
  ~CCoreDX11VertexBuffer() override;

  bool Create(const uint8_t* data, const uint32_t size) override;
  bool CreateDynamic(const uint32_t size) override;
  bool Update(const int32_t offset, const uint8_t* data,
              const uint32_t size) override;
  bool Lock(void** result) override;
  bool Lock(void** result, const uint32_t offset, const int32_t size,
            const int32_t flags = 0) override;
  bool UnLock() override;
  void* GetHandle() override { return vertex_buffer_handle_; }

 private:
  virtual void Release();
  bool Apply(const uint32_t offset) override;

  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* d3d_device_context_;
  ID3D11Buffer* vertex_buffer_handle_;

  int32_t size_;
  bool dynamic_;
};

}  // namespace renderer
