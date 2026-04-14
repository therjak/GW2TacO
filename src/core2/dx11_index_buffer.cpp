#include "src/core2/dx11_index_buffer.h"

#include <comdef.h>
#include <d3d11.h>

#include "src/base/logger.h"

namespace renderer {

DX11IndexBuffer::DX11IndexBuffer(DX11Device* device) : IndexBuffer(device) {
  d3d_device_ = device->GetDevice();
  d3d_device_context_ = device->GetDeviceContext();
  index_buffer_handle_ = nullptr;
  index_count_ = 0;
  index_size_ = 0;
}

DX11IndexBuffer::~DX11IndexBuffer() { Release(); }

void DX11IndexBuffer::Release() {
  if (index_buffer_handle_) index_buffer_handle_->Release();
  index_buffer_handle_ = nullptr;
}

bool DX11IndexBuffer::Apply() {
  if (!index_buffer_handle_) return false;
  d3d_device_context_->IASetIndexBuffer(
      index_buffer_handle_,
      index_size_ == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
  return true;
}

bool DX11IndexBuffer::Create(const uint32_t index_count,
                             const uint32_t index_size) {
  if (index_count <= 0 || index_size <= 0) return false;
  if (index_size != 2 && index_size != 4) return false;

  Release();

  D3D11_BUFFER_DESC buffer_desc;
  ZeroMemory(&buffer_desc, sizeof(buffer_desc));

  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.ByteWidth = index_count * index_size;
  buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  const HRESULT result =
      d3d_device_->CreateBuffer(&buffer_desc, nullptr, &index_buffer_handle_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] CreateBuffer for indexbuffer failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  index_count_ = index_count;
  index_size_ = index_size;

  return true;
}

bool DX11IndexBuffer::Lock(void** result_ptr, const uint32_t index_offset,
                           const int32_t index_count) {
  if (!index_buffer_handle_) return false;

  D3D11_MAPPED_SUBRESOURCE mapped_resource;
  const HRESULT result =
      d3d_device_context_->Map(index_buffer_handle_, NULL,
                               D3D11_MAP_WRITE_DISCARD, NULL, &mapped_resource);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] Failed to lock indexbuffer failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  *result_ptr = mapped_resource.pData;
  return true;
}

bool DX11IndexBuffer::Lock(void** result_ptr) {
  return Lock(result_ptr, 0, index_count_);
}

bool DX11IndexBuffer::UnLock() {
  if (!index_buffer_handle_) return false;
  d3d_device_context_->Unmap(index_buffer_handle_, 0);
  return true;
}

}  // namespace renderer
