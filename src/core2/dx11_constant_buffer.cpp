#include "src/core2/dx11_constant_buffer.h"

#include <d3d11.h>

#include <cstring>

#include "src/base/logger.h"

namespace renderer {

CCoreDX11ConstantBuffer::CCoreDX11ConstantBuffer(CCoreDX11Device* dev)
    : CCoreConstantBuffer(dev) {
  buffer_ = nullptr;
  device_ = dev->GetDevice();
  device_context_ = dev->GetDeviceContext();
  allocated_buffer_size_ = 0;
}

CCoreDX11ConstantBuffer::~CCoreDX11ConstantBuffer() {
  if (buffer_) buffer_->Release();
}

void* CCoreDX11ConstantBuffer::GetBufferPointer() const { return buffer_; }

void CCoreDX11ConstantBuffer::Upload() {
  if (allocated_buffer_size_ < data_length_) {
    // allocate appropriate size buffer
    if (buffer_) buffer_->Release();

    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = data_length_;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    buffer_ = nullptr;
    if (device_->CreateBuffer(&desc, nullptr, &buffer_) != S_OK) {
      buffer_ = nullptr;
      allocated_buffer_size_ = 0;
      Log_Err("[core] Error creating constant buffer of size {:d}", data_length_);
    } else {
      allocated_buffer_size_ = data_length_;
    }
  }

  if (!buffer_ || !data_ || !data_length_) return;

  // upload data
  D3D11_MAPPED_SUBRESOURCE map;
  if (device_context_->Map(buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &map) != S_OK) {
    Log_Err("[core] Failed to map constant buffer resource!");
  } else {
    std::memcpy(map.pData, data_.get(), data_length_);
    device_context_->Unmap(buffer_, 0);
  }
}

}  // namespace renderer
