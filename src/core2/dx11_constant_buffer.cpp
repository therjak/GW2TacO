#include "src/core2/dx11_constant_buffer.h"

#include <cstring>

#ifdef CORE_API_DX11
CCoreDX11ConstantBuffer::CCoreDX11ConstantBuffer(CCoreDX11Device* dev)
    : CCoreConstantBuffer(dev) {
  Buffer = nullptr;
  Dev = dev->GetDevice();
  DeviceContext = dev->GetDeviceContext();
  AllocatedBufferSize = 0;
}

CCoreDX11ConstantBuffer::~CCoreDX11ConstantBuffer() {
  if (Buffer) Buffer->Release();
}

void* CCoreDX11ConstantBuffer::GetBufferPointer() const { return Buffer; }

void CCoreDX11ConstantBuffer::Upload() {
  if (AllocatedBufferSize < DataLength) {
    // allocate appropriate size buffer
    if (Buffer) Buffer->Release();

    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = DataLength;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    Buffer = nullptr;
    if (Dev->CreateBuffer(&desc, nullptr, &Buffer) != S_OK) {
      Buffer = nullptr;
      AllocatedBufferSize = 0;
      LOG_ERR("[core] Error creating constant buffer of size %d", DataLength);
    } else {
      AllocatedBufferSize = DataLength;
    }
  }

  if (!Buffer || !Data || !DataLength) return;

  // upload data
  D3D11_MAPPED_SUBRESOURCE map;
  if (DeviceContext->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map) != S_OK) {
    LOG_ERR("[core] Failed to map constant buffer resource!");
  } else {
    memcpy(map.pData, Data.get(), DataLength);
    DeviceContext->Unmap(Buffer, 0);
  }
}

#endif
