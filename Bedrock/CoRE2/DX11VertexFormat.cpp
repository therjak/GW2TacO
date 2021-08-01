#include "DX11VertexFormat.h"

#ifdef CORE_API_DX11

#include <comdef.h>

#include <vector>

CCoreDX11VertexFormat::CCoreDX11VertexFormat(CCoreDX11Device* dev)
    : CCoreVertexFormat(dev) {
  Dev = dev->GetDevice();
  DeviceContext = dev->GetDeviceContext();
  VertexFormatHandle = nullptr;
  Size = 0;
}

CCoreDX11VertexFormat::~CCoreDX11VertexFormat() { Release(); }

void CCoreDX11VertexFormat::Release() {
  if (VertexFormatHandle) VertexFormatHandle->Release();
  VertexFormatHandle = nullptr;
}

bool CCoreDX11VertexFormat::Apply() {
  if (!VertexFormatHandle) return false;
  DeviceContext->IASetInputLayout(VertexFormatHandle);
  return true;
}

bool CCoreDX11VertexFormat::Create(
    const std::vector<COREVERTEXATTRIBUTE>& Attributes, CCoreVertexShader* vs) {
  if (!vs) return false;
  if (!Attributes.size()) return false;
  Release();

  int32_t PosUsages = 0;
  int32_t NormUsages = 0;
  int32_t UVUsages = 0;
  int32_t ColUsages = 0;

  Size = 0;

  auto vxdecl =
      std::make_unique<D3D11_INPUT_ELEMENT_DESC[]>(Attributes.size() + 1);
  memset(vxdecl.get(), 0,
         sizeof(D3D11_INPUT_ELEMENT_DESC) * (Attributes.size() + 1));

  for (int32_t x = 0; x < Attributes.size(); x++) {
    vxdecl[x].InputSlot = 0;
    vxdecl[x].AlignedByteOffset = Size;
    vxdecl[x].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    vxdecl[x].InstanceDataStepRate = 0;

    switch (Attributes[x]) {
      case COREVERTEXATTRIBUTE::COREVXATTR_POSITION3: {
        vxdecl[x].SemanticName = "Position";
        vxdecl[x].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        vxdecl[x].SemanticIndex = PosUsages++;
        Size += 12;
      } break;
      case COREVERTEXATTRIBUTE::COREVXATTR_POSITION4: {
        vxdecl[x].SemanticName = "Position";
        vxdecl[x].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        vxdecl[x].SemanticIndex = PosUsages++;
        Size += 16;
      } break;
      case COREVERTEXATTRIBUTE::COREVXATTR_NORMAL3: {
        vxdecl[x].SemanticName = "Normal";
        vxdecl[x].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        vxdecl[x].SemanticIndex = NormUsages++;
        Size += 12;
      } break;
      case COREVERTEXATTRIBUTE::COREVXATTR_TEXCOORD2: {
        vxdecl[x].SemanticName = "Texcoord";
        vxdecl[x].Format = DXGI_FORMAT_R32G32_FLOAT;
        vxdecl[x].SemanticIndex = UVUsages++;
        Size += 8;
      } break;
      case COREVERTEXATTRIBUTE::COREVXATTR_TEXCOORD4: {
        vxdecl[x].SemanticName = "Texcoord";
        vxdecl[x].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        vxdecl[x].SemanticIndex = UVUsages++;
        Size += 16;
      } break;
      case COREVERTEXATTRIBUTE::COREVXATTR_COLOR4: {
        vxdecl[x].SemanticName = "Color";
        vxdecl[x].Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        vxdecl[x].SemanticIndex = ColUsages++;
        Size += 4;
      } break;
      case COREVERTEXATTRIBUTE::COREVXATTR_COLOR16: {
        vxdecl[x].SemanticName = "Color";
        vxdecl[x].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        vxdecl[x].SemanticIndex = ColUsages++;
        Size += 16;
      } break;
      case COREVERTEXATTRIBUTE::COREVXATTR_POSITIONT4: {
        vxdecl[x].SemanticName = "PositionT";
        vxdecl[x].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        vxdecl[x].SemanticIndex = PosUsages++;
        Size += 16;
      } break;

      default: {
        // unhandled format
        return false;
      }
    }
  }

  const HRESULT res =
      Dev->CreateInputLayout(vxdecl.get(), Attributes.size(), vs->GetBinary(),
                             vs->GetBinaryLength(), &VertexFormatHandle);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[core] CreateInputLayout failed (%s)", err.ErrorMessage());
    return false;
  }

  return true;
}

int32_t CCoreDX11VertexFormat::GetSize() { return Size; }

#else
NoEmptyFile();
#endif
