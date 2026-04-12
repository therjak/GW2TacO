#include "src/core2/dx11_vertex_buffer.h"

#include <comdef.h>
#include <d3d11.h>

#include "src/base/logger.h"

namespace renderer {

CCoreDX11VertexBuffer::CCoreDX11VertexBuffer(CCoreDX11Device* device)
    : CCoreVertexBuffer(device) {
  d3d_device_ = device->GetDevice();
  d3d_device_context_ = device->GetDeviceContext();
  vertex_buffer_handle_ = nullptr;
  size_ = 0;
  dynamic_ = false;
}

CCoreDX11VertexBuffer::~CCoreDX11VertexBuffer() { Release(); }

void CCoreDX11VertexBuffer::Release() {
  if (vertex_buffer_handle_) vertex_buffer_handle_->Release();
  vertex_buffer_handle_ = nullptr;
}

bool CCoreDX11VertexBuffer::Apply(const uint32_t offset) {
  if (!vertex_buffer_handle_) return false;
  const uint32_t stride = device_->GetVertexFormatSize();
  d3d_device_context_->IASetVertexBuffers(0, 1, &vertex_buffer_handle_, &stride,
                                          &offset);
  return true;
}

bool CCoreDX11VertexBuffer::Create(const uint8_t* data, const uint32_t size) {
  if (!data) return false;
  if (size <= 0) return false;
  Release();

  D3D11_BUFFER_DESC buffer_desc;
  memset(&buffer_desc, 0, sizeof(buffer_desc));

  buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  buffer_desc.ByteWidth = size;
  buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

  D3D11_SUBRESOURCE_DATA vertex_data;
  vertex_data.pSysMem = data;
  vertex_data.SysMemPitch = 0;
  vertex_data.SysMemSlicePitch = 0;

  const HRESULT result =
      d3d_device_->CreateBuffer(&buffer_desc, &vertex_data, &vertex_buffer_handle_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] CreateBuffer for vertexbuffer failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  size_ = size;
  return true;
}

bool CCoreDX11VertexBuffer::CreateDynamic(const uint32_t size) {
  if (size <= 0) return false;
  Release();

  D3D11_BUFFER_DESC buffer_desc;
  memset(&buffer_desc, 0, sizeof(buffer_desc));

  buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  buffer_desc.ByteWidth = size;
  buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  const HRESULT result =
      d3d_device_->CreateBuffer(&buffer_desc, nullptr, &vertex_buffer_handle_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] CreateBuffer for vertexbuffer failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  size_ = size;
  dynamic_ = true;

  return true;
}

bool CCoreDX11VertexBuffer::Update(const int32_t offset, const uint8_t* data,
                                   const uint32_t size) {
  if (!vertex_buffer_handle_ || !data || dynamic_) return false;
  if (!size) return true;

  D3D11_BOX box;
  memset(&box, 0, sizeof(D3D11_BOX));
  box.left = offset;
  box.right = offset + size;

  d3d_device_context_->UpdateSubresource(vertex_buffer_handle_, 0, &box, data,
                                         size, 1);

  return true;
}

bool CCoreDX11VertexBuffer::Lock(void** result_ptr, const uint32_t offset,
                                 const int32_t size, const int32_t flags) {
  if (!dynamic_) {
    Log_Err("[core] Attempting to lock static vertexbuffer failed");
    return false;
  }

  if (!vertex_buffer_handle_) return false;

  D3D11_MAPPED_SUBRESOURCE mapped_resource;
  if (d3d_device_context_->Map(vertex_buffer_handle_, 0, D3D11_MAP_WRITE_DISCARD,
                               0, &mapped_resource) != S_OK) {
    return false;
  }

  *result_ptr = mapped_resource.pData;
  return true;
}

bool CCoreDX11VertexBuffer::Lock(void** result_ptr) {
  return Lock(result_ptr, 0, size_);
}

bool CCoreDX11VertexBuffer::UnLock() {
  if (!vertex_buffer_handle_) return false;
  d3d_device_context_->Unmap(vertex_buffer_handle_, 0);
  return true;
}

}  // namespace renderer
