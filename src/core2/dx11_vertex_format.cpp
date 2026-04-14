#include "src/core2/dx11_vertex_format.h"

#include <comdef.h>
#include <d3d11.h>

#include <vector>

#include "src/base/logger.h"

namespace renderer {

DX11VertexFormat::DX11VertexFormat(DX11Device* device)
    : VertexFormat(device) {
  d3d_device_ = device->GetDevice();
  d3d_device_context_ = device->GetDeviceContext();
  vertex_format_handle_ = nullptr;
  size_ = 0;
}

DX11VertexFormat::~DX11VertexFormat() { Release(); }

void DX11VertexFormat::Release() {
  if (vertex_format_handle_) vertex_format_handle_->Release();
  vertex_format_handle_ = nullptr;
}

bool DX11VertexFormat::Apply() {
  if (!vertex_format_handle_) return false;
  d3d_device_context_->IASetInputLayout(vertex_format_handle_);
  return true;
}

bool DX11VertexFormat::Create(
    const std::vector<VertexAttribute>& attributes, VertexShader* vertex_shader) {
  if (!vertex_shader) return false;
  if (!attributes.size()) return false;
  Release();

  int32_t pos_usages = 0;
  int32_t norm_usages = 0;
  int32_t uv_usages = 0;
  int32_t col_usages = 0;

  size_ = 0;

  auto vertex_decl =
      std::make_unique<D3D11_INPUT_ELEMENT_DESC[]>(attributes.size() + 1);
  memset(vertex_decl.get(), 0,
         sizeof(D3D11_INPUT_ELEMENT_DESC) * (attributes.size() + 1));

  for (size_t i = 0; i < attributes.size(); i++) {
    vertex_decl[i].InputSlot = 0;
    vertex_decl[i].AlignedByteOffset = static_cast<UINT32>(size_);
    vertex_decl[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    vertex_decl[i].InstanceDataStepRate = 0;

    switch (attributes[i]) {
      case VertexAttribute::kPosition3: {
        vertex_decl[i].SemanticName = "Position";
        vertex_decl[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        vertex_decl[i].SemanticIndex = pos_usages++;
        size_ += 12;
      } break;
      case VertexAttribute::kPosition4: {
        vertex_decl[i].SemanticName = "Position";
        vertex_decl[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        vertex_decl[i].SemanticIndex = pos_usages++;
        size_ += 16;
      } break;
      case VertexAttribute::kNormal3: {
        vertex_decl[i].SemanticName = "Normal";
        vertex_decl[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        vertex_decl[i].SemanticIndex = norm_usages++;
        size_ += 12;
      } break;
      case VertexAttribute::kTexCoord2: {
        vertex_decl[i].SemanticName = "Texcoord";
        vertex_decl[i].Format = DXGI_FORMAT_R32G32_FLOAT;
        vertex_decl[i].SemanticIndex = uv_usages++;
        size_ += 8;
      } break;
      case VertexAttribute::kTexCoord4: {
        vertex_decl[i].SemanticName = "Texcoord";
        vertex_decl[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        vertex_decl[i].SemanticIndex = uv_usages++;
        size_ += 16;
      } break;
      case VertexAttribute::kColor4: {
        vertex_decl[i].SemanticName = "Color";
        vertex_decl[i].Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        vertex_decl[i].SemanticIndex = col_usages++;
        size_ += 4;
      } break;
      case VertexAttribute::kColor16: {
        vertex_decl[i].SemanticName = "Color";
        vertex_decl[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        vertex_decl[i].SemanticIndex = col_usages++;
        size_ += 16;
      } break;
      case VertexAttribute::kPositionT4: {
        vertex_decl[i].SemanticName = "PositionT";
        vertex_decl[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        vertex_decl[i].SemanticIndex = pos_usages++;
        size_ += 16;
      } break;

      default: {
        // unhandled format
        return false;
      }
    }
  }

  const HRESULT result = d3d_device_->CreateInputLayout(
      vertex_decl.get(), static_cast<UINT>(attributes.size()),
      vertex_shader->GetBinary(),
      static_cast<SIZE_T>(vertex_shader->GetBinaryLength()),
      &vertex_format_handle_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] CreateInputLayout failed ({:s})", error.ErrorMessage());
    return false;
  }

  return true;
}

int32_t DX11VertexFormat::GetSize() { return size_; }

}  // namespace renderer
