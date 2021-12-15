#include "dx11_index_buffer.h"

#ifdef CORE_API_DX11

#include <comdef.h>

CCoreDX11IndexBuffer::CCoreDX11IndexBuffer(CCoreDX11Device* dev)
    : CCoreIndexBuffer(dev) {
  Dev = dev->GetDevice();
  DeviceContext = dev->GetDeviceContext();
  IndexBufferHandle = nullptr;
  IndexCount = 0;
  IndexSize = 0;
}

CCoreDX11IndexBuffer::~CCoreDX11IndexBuffer() { Release(); }

void CCoreDX11IndexBuffer::Release() {
  if (IndexBufferHandle) IndexBufferHandle->Release();
  IndexBufferHandle = nullptr;
}

bool CCoreDX11IndexBuffer::Apply() {
  if (!IndexBufferHandle) return false;
  DeviceContext->IASetIndexBuffer(
      IndexBufferHandle,
      IndexSize == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
  return true;
}

bool CCoreDX11IndexBuffer::Create(const uint32_t idxcount,
                                  const uint32_t idxsize) {
  if (idxcount <= 0 || idxsize <= 0) return false;
  if (idxsize != 2 && idxsize != 4) return false;

  Release();

  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));

  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.ByteWidth = idxcount * idxsize;
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  const HRESULT res = Dev->CreateBuffer(&bd, nullptr, &IndexBufferHandle);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[core] CreateBuffer for indexbuffer failed (%s)",
            err.ErrorMessage());
    return false;
  }

  IndexCount = idxcount;
  IndexSize = idxsize;

  return true;
}

bool CCoreDX11IndexBuffer::Lock(void** Result, const uint32_t idxoffset,
                                const int32_t idxcount) {
  if (!IndexBufferHandle) return false;

  D3D11_MAPPED_SUBRESOURCE ms;
  const HRESULT res = DeviceContext->Map(IndexBufferHandle, NULL,
                                         D3D11_MAP_WRITE_DISCARD, NULL, &ms);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[core] Failed to lock indexbuffer failed (%s)",
            err.ErrorMessage());
    return false;
  }

  *Result = ms.pData;
  return true;
}

bool CCoreDX11IndexBuffer::Lock(void** Result) {
  return Lock(Result, 0, IndexCount);
}

bool CCoreDX11IndexBuffer::UnLock() {
  if (!IndexBufferHandle) return false;
  DeviceContext->Unmap(IndexBufferHandle, 0);
  return true;
}

#else
NoEmptyFile();
#endif
