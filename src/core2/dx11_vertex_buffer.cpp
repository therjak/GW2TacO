#include "src/core2/dx11_vertex_buffer.h"

#include <comdef.h>

#include "src/base/logger.h"

CCoreDX11VertexBuffer::CCoreDX11VertexBuffer(CCoreDX11Device* dev)
    : CCoreVertexBuffer(dev) {
  Dev = dev->GetDevice();
  DeviceContext = dev->GetDeviceContext();
  VertexBufferHandle = nullptr;
  Size = 0;
  Dynamic = false;
}

CCoreDX11VertexBuffer::~CCoreDX11VertexBuffer() { Release(); }

void CCoreDX11VertexBuffer::Release() {
  if (VertexBufferHandle) VertexBufferHandle->Release();
  VertexBufferHandle = nullptr;
}

bool CCoreDX11VertexBuffer::Apply(const uint32_t Offset) {
  if (!VertexBufferHandle) return false;
  const uint32_t stride = Device->GetVertexFormatSize();
  DeviceContext->IASetVertexBuffers(0, 1, &VertexBufferHandle, &stride,
                                    &Offset);
  return true;
}

bool CCoreDX11VertexBuffer::Create(const uint8_t* Data, const uint32_t size) {
  if (!Data) return false;
  if (size <= 0) return false;
  Release();

  D3D11_BUFFER_DESC bd;
  memset(&bd, 0, sizeof(bd));

  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = size;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

  D3D11_SUBRESOURCE_DATA vxData;
  vxData.pSysMem = Data;
  vxData.SysMemPitch = 0;
  vxData.SysMemSlicePitch = 0;

  const HRESULT res = Dev->CreateBuffer(&bd, &vxData, &VertexBufferHandle);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[core] CreateBuffer for vertexbuffer failed (%s)",
            err.ErrorMessage());
    return false;
  }

  Size = size;
  return true;
}

bool CCoreDX11VertexBuffer::CreateDynamic(const uint32_t size) {
  if (size <= 0) return false;
  Release();

  D3D11_BUFFER_DESC bd;
  memset(&bd, 0, sizeof(bd));

  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.ByteWidth = size;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  const HRESULT res = Dev->CreateBuffer(&bd, nullptr, &VertexBufferHandle);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[core] CreateBuffer for vertexbuffer failed (%s)",
            err.ErrorMessage());
    return false;
  }

  Size = size;
  Dynamic = true;

  return true;
}

bool CCoreDX11VertexBuffer::Update(const int32_t Offset, const uint8_t* Data,
                                   const uint32_t Size) {
  if (!VertexBufferHandle || !Data || Dynamic) return false;
  if (!Size) return true;

  D3D11_BOX box;
  memset(&box, 0, sizeof(D3D11_BOX));
  box.left = Offset;
  box.right = Offset + Size;

  DeviceContext->UpdateSubresource(VertexBufferHandle, 0, &box, Data, Size, 1);

  return true;
}

bool CCoreDX11VertexBuffer::Lock(void** Result, const uint32_t Offset,
                                 const int32_t size, const int32_t Flags) {
  if (!Dynamic) {
    LOG_ERR("%s", "[core] Attempting to lock static vertexbuffer failed");
    return false;
  }

  if (!VertexBufferHandle) return false;

  D3D11_MAPPED_SUBRESOURCE ms;
  if (DeviceContext->Map(VertexBufferHandle, 0, D3D11_MAP_WRITE_DISCARD, 0,
                         &ms) != S_OK)
    return false;

  *Result = ms.pData;
  return true;
}

bool CCoreDX11VertexBuffer::Lock(void** Result) {
  return Lock(Result, 0, Size);
}

bool CCoreDX11VertexBuffer::UnLock() {
  if (!VertexBufferHandle) return false;
  DeviceContext->Unmap(VertexBufferHandle, 0);
  return true;
}
