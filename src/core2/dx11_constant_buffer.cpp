#include "src/core2/dx11_constant_buffer.h"

#include <d3d11.h>

#include <cstring>

#include "src/base/logger.h"

namespace renderer {

CCoreDX11ConstantBuffer::CCoreDX11ConstantBuffer(CCoreDX11Device* device)
    : CCoreConstantBuffer(device) {
  buffer_ = nullptr;
  d3d_device_ = device->GetDevice();
  d3d_device_context_ = device->GetDeviceContext();
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

    D3D11_BUFFER_DESC buffer_desc;
    buffer_desc.ByteWidth = data_length_;
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    buffer_desc.MiscFlags = 0;
    buffer_desc.StructureByteStride = 0;

    buffer_ = nullptr;
    if (d3d_device_->CreateBuffer(&buffer_desc, nullptr, &buffer_) != S_OK) {
      buffer_ = nullptr;
      allocated_buffer_size_ = 0;
      Log_Err("[core] Error creating constant buffer of size {:d}", data_length_);
    } else {
      allocated_buffer_size_ = data_length_;
    }
  }

  if (!buffer_ || !data_ || !data_length_) return;

  // upload data
  D3D11_MAPPED_SUBRESOURCE mapped_resource;
  if (d3d_device_context_->Map(buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0,
                               &mapped_resource) != S_OK) {
    Log_Err("[core] Failed to map constant buffer resource!");
  } else {
    std::memcpy(mapped_resource.pData, data_.get(), data_length_);
    d3d_device_context_->Unmap(buffer_, 0);
  }
}

}  // namespace renderer
