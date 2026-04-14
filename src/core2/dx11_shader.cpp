#include "src/core2/dx11_shader.h"

#include <comdef.h>
#include <d3d11.h>

#include <array>
#include <cfloat>
#include <string>

#include "src/base/logger.h"

namespace renderer {

typedef HRESULT(__stdcall d3d_compile_func)(
    LPCVOID pSrcData, SIZE_T SrcDataSize, LPCSTR pSourceName,
    const D3D_SHADER_MACRO* pDefines, ID3DInclude* pInclude, LPCSTR pEntrypoint,
    LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode,
    ID3DBlob** ppErrorMsgs);
typedef HRESULT(__stdcall d3d_reflect_func)(LPCVOID pSrcData,
                                            SIZE_T SrcDataSize,
                                            REFIID pInterface,
                                            void** ppReflector);

typedef HRESULT(__stdcall D3DXCompileShader(
    LPCSTR pSrcData, UINT srcDataLen, const void* pDefines, void* pInclude,
    LPCSTR pFunctionName, LPCSTR pProfile, DWORD Flags, void** ppShader,
    void** ppErrorMsgs, void** ppConstantTable));

d3d_compile_func* D3DCompileFunc = nullptr;
D3DXCompileShader* D3DXCompileFunc = nullptr;

void* GetFunctionFromD3DCompileDLL(const std::string& FunctName) {
  HMODULE dll = nullptr;

  constexpr std::array<std::string_view, 15> CompilerDLLs = {
      "d3dcompiler_47.dll", "d3dcompiler_46.dll", "d3dcompiler_45.dll",
      "d3dcompiler_44.dll", "d3dcompiler_43.dll", "d3dcompiler_42.dll",
      "d3dcompiler_41.dll", "d3dcompiler_40.dll", "d3dcompiler_39.dll",
      "d3dcompiler_38.dll", "d3dcompiler_37.dll", "d3dcompiler_36.dll",
      "d3dcompiler_35.dll", "d3dcompiler_34.dll", "d3dcompiler_33.dll"};

  for (const auto& cd : CompilerDLLs) {
    dll = LoadLibraryA(cd.data());
    if (dll) {
      void* func = GetProcAddress(dll, FunctName.c_str());
      if (func) {
        Log_Nfo("[core] Successfully loaded {:s} from {:s}", FunctName, cd);
        return func;
      }
    }
  }

  Log_Err("[core] Failed to load {:s} from d3dcompile_xx.dll!", FunctName);
  return nullptr;
}

bool InitShaderCompiler() {
  if (D3DCompileFunc || D3DXCompileFunc) return true;
  D3DCompileFunc = static_cast<d3d_compile_func*>(
      GetFunctionFromD3DCompileDLL("D3DCompile"));
  return D3DCompileFunc != nullptr;
}

CCoreDX11VertexShader::CCoreDX11VertexShader(CCoreDX11Device* device)
    : CCoreVertexShader(device) {
  d3d_device_ = device->GetDevice();
  d3d_device_context_ = device->GetDeviceContext();
  vertex_shader_handle_ = nullptr;
}

CCoreDX11VertexShader::~CCoreDX11VertexShader() { Release(); }

bool CCoreDX11VertexShader::Create(void* binary, int32_t length) {
  if (!binary || length <= 0) return false;
  FetchBinary(binary, length);
  const HRESULT result =
      d3d_device_->CreateVertexShader(binary, length, nullptr, &vertex_shader_handle_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] VertexShader Creation error ({:s})", error.ErrorMessage());
  }
  return result == S_OK;
}

void CCoreDX11VertexShader::Release() {
  if (vertex_shader_handle_) vertex_shader_handle_->Release();
  vertex_shader_handle_ = nullptr;
}

bool CCoreDX11VertexShader::Apply() {
  if (!vertex_shader_handle_) return false;
  d3d_device_context_->VSSetShader(vertex_shader_handle_, nullptr, 0);
  return true;
}

bool CCoreDX11VertexShader::CompileAndCreate(std::string* error) {
  if (!D3DCompileFunc && !InitShaderCompiler()) return false;

  Release();

  bool success = true;

  ID3D10Blob* ps_blob = nullptr;
  ID3D10Blob* error_blob = nullptr;

  uint32_t fpu_control_word = 0;
  _controlfp_s(&fpu_control_word, _RC_NEAR, _MCW_RC);

  if (D3DCompileFunc(code_.c_str(), code_.size(), nullptr, nullptr, nullptr,
                     entry_function_.c_str(), shader_version_.c_str(), 0, 0,
                     &ps_blob, &error_blob) != S_OK) {
    if (!error) {
      Log_Err("[core] VertexShader compilation error: {:s}",
              static_cast<char*>(error_blob->GetBufferPointer()));
    }
    success = false;
  }

  if (error) {
    *error = error_blob
                 ? std::string(static_cast<char*>(error_blob->GetBufferPointer()))
                 : "";
  }

  if (error_blob) {
    error_blob->Release();
    error_blob = nullptr;
  }

  if (success) {
    success = Create(ps_blob->GetBufferPointer(),
                     static_cast<int32_t>(ps_blob->GetBufferSize()));
    ps_blob->Release();
  }

#ifndef _WIN64
  _controlfp_s(&fpu_control_word, fpu_control_word, 0xffffffff);
#endif

  return success;
}

bool CCoreDX11VertexShader::CreateFromBlob(void* code_blob,
                                           int32_t code_blob_size) {
  Release();
  bool success = true;
  uint32_t fpu_control_word = 0;
  _controlfp_s(&fpu_control_word, _RC_NEAR, _MCW_RC);
  success = Create(code_blob, code_blob_size);
#ifndef _WIN64
  _controlfp_s(&fpu_control_word, fpu_control_word, 0xffffffff);
#endif
  return success;
}

CCoreDX11PixelShader::CCoreDX11PixelShader(CCoreDX11Device* device)
    : CCorePixelShader(device) {
  d3d_device_ = device->GetDevice();
  d3d_device_context_ = device->GetDeviceContext();
  pixel_shader_handle_ = nullptr;
}

CCoreDX11PixelShader::~CCoreDX11PixelShader() { Release(); }

bool CCoreDX11PixelShader::Create(void* binary, int32_t length) {
  if (!binary || length <= 0) return false;
  FetchBinary(binary, length);
  const HRESULT result =
      d3d_device_->CreatePixelShader(binary, length, nullptr, &pixel_shader_handle_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] PixelShader Creation error ({:s})", error.ErrorMessage());
  }
  return result == S_OK;
}

void CCoreDX11PixelShader::Release() {
  if (pixel_shader_handle_) pixel_shader_handle_->Release();
  pixel_shader_handle_ = nullptr;
}

bool CCoreDX11PixelShader::Apply() {
  if (!pixel_shader_handle_) return false;
  d3d_device_context_->PSSetShader(pixel_shader_handle_, nullptr, 0);
  return true;
}

bool CCoreDX11PixelShader::CompileAndCreate(std::string* error) {
  if (!D3DCompileFunc && !InitShaderCompiler()) return false;

  Release();

  bool success = true;

  ID3D10Blob* ps_blob = nullptr;
  ID3D10Blob* error_blob = nullptr;

  uint32_t fpu_control_word = 0;
  _controlfp_s(&fpu_control_word, _RC_NEAR, _MCW_RC);

  if (D3DCompileFunc(code_.c_str(), code_.size(), nullptr, nullptr, nullptr,
                     entry_function_.c_str(), shader_version_.c_str(), 0, 0,
                     &ps_blob, &error_blob) != S_OK) {
    if (!error) {
      Log_Err("[core] PixelShader compilation error: {:s}",
              static_cast<char*>(error_blob->GetBufferPointer()));
    }
    success = false;
  }

  if (error) {
    *error = error_blob
                 ? std::string(static_cast<char*>(error_blob->GetBufferPointer()))
                 : "";
  }

  if (error_blob) {
    error_blob->Release();
    error_blob = nullptr;
  }

  if (success) {
    success = Create(ps_blob->GetBufferPointer(),
                     static_cast<int32_t>(ps_blob->GetBufferSize()));
    ps_blob->Release();
  }

#ifndef _WIN64
  _controlfp_s(&fpu_control_word, fpu_control_word, 0xffffffff);
#endif

  return success;
}

bool CCoreDX11PixelShader::CreateFromBlob(void* code_blob,
                                          int32_t code_blob_size) {
  Release();
  bool success = true;
  uint32_t tmp = 0;
  _controlfp_s(&tmp, _RC_NEAR, _MCW_RC);
  success = Create(code_blob, code_blob_size);
#ifndef _WIN64
  _controlfp_s(&tmp, tmp, 0xffffffff);
#endif
  return success;
}

CCoreDX11GeometryShader::CCoreDX11GeometryShader(CCoreDX11Device* device)
    : CCoreGeometryShader(device) {
  d3d_device_ = device->GetDevice();
  d3d_device_context_ = device->GetDeviceContext();
  geometry_shader_handle_ = nullptr;
}

CCoreDX11GeometryShader::~CCoreDX11GeometryShader() { Release(); }

bool CCoreDX11GeometryShader::Create(void* binary, int32_t length) {
  if (!binary || length <= 0) return false;
  FetchBinary(binary, length);
  const HRESULT result =
      d3d_device_->CreateGeometryShader(binary, length, nullptr, &geometry_shader_handle_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] GeometryShader Creation error ({:s})", error.ErrorMessage());
  }
  return result == S_OK;
}

void CCoreDX11GeometryShader::Release() {
  if (geometry_shader_handle_) geometry_shader_handle_->Release();
  geometry_shader_handle_ = nullptr;
}

bool CCoreDX11GeometryShader::Apply() {
  if (!geometry_shader_handle_) return false;
  d3d_device_context_->GSSetShader(geometry_shader_handle_, nullptr, 0);
  return true;
}

bool CCoreDX11GeometryShader::CompileAndCreate(std::string* error) {
  if (!D3DCompileFunc && !InitShaderCompiler()) return false;

  Release();

  bool success = true;

  ID3D10Blob* ps_blob = nullptr;
  ID3D10Blob* error_blob = nullptr;

  uint32_t fpu_control_word = 0;
  _controlfp_s(&fpu_control_word, _RC_NEAR, _MCW_RC);

  if (D3DCompileFunc(code_.c_str(), code_.size(), nullptr, nullptr, nullptr,
                     entry_function_.c_str(), shader_version_.c_str(), 0, 0,
                     &ps_blob, &error_blob) != S_OK) {
    if (!error) {
      Log_Err("[core] GeometryShader compilation error: {:s}",
              static_cast<char*>(error_blob->GetBufferPointer()));
    }
    success = false;
  }

  if (error) {
    *error = error_blob
                 ? std::string(static_cast<char*>(error_blob->GetBufferPointer()))
                 : "";
  }

  if (error_blob) {
    error_blob->Release();
    error_blob = nullptr;
  }

  if (success) {
    success = Create(ps_blob->GetBufferPointer(),
                     static_cast<int32_t>(ps_blob->GetBufferSize()));
    ps_blob->Release();
  }

#ifndef _WIN64
  _controlfp_s(&fpu_control_word, fpu_control_word, 0xffffffff);
#endif

  return success;
}

bool CCoreDX11GeometryShader::CreateFromBlob(void* code_blob,
                                             int32_t code_blob_size) {
  Release();
  bool success = true;
  uint32_t fpu_control_word = 0;
  _controlfp_s(&fpu_control_word, _RC_NEAR, _MCW_RC);
  success = Create(code_blob, code_blob_size);
#ifndef _WIN64
  _controlfp_s(&fpu_control_word, fpu_control_word, 0xffffffff);
#endif
  return success;
}

CCoreDX11DomainShader::CCoreDX11DomainShader(CCoreDX11Device* device)
    : CCoreDomainShader(device) {
  d3d_device_ = device->GetDevice();
  d3d_device_context_ = device->GetDeviceContext();
  domain_shader_handle_ = nullptr;
}

CCoreDX11DomainShader::~CCoreDX11DomainShader() { Release(); }

bool CCoreDX11DomainShader::Create(void* binary, int32_t length) {
  if (!binary || length <= 0) return false;
  FetchBinary(binary, length);
  const HRESULT result =
      d3d_device_->CreateDomainShader(binary, length, nullptr, &domain_shader_handle_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DomainShader Creation error ({:s})", error.ErrorMessage());
  }
  return result == S_OK;
}

void CCoreDX11DomainShader::Release() {
  if (domain_shader_handle_) domain_shader_handle_->Release();
  domain_shader_handle_ = nullptr;
}

bool CCoreDX11DomainShader::Apply() {
  if (!domain_shader_handle_) return false;
  d3d_device_context_->DSSetShader(domain_shader_handle_, nullptr, 0);
  return true;
}

bool CCoreDX11DomainShader::CompileAndCreate(std::string* error) {
  if (!D3DCompileFunc && !InitShaderCompiler()) return false;

  Release();

  bool success = true;

  ID3D10Blob* ps_blob = nullptr;
  ID3D10Blob* error_blob = nullptr;

  uint32_t fpu_control_word = 0;
  _controlfp_s(&fpu_control_word, _RC_NEAR, _MCW_RC);

  if (D3DCompileFunc(code_.c_str(), code_.size(), nullptr, nullptr, nullptr,
                     entry_function_.c_str(), shader_version_.c_str(), 0, 0,
                     &ps_blob, &error_blob) != S_OK) {
    if (!error) {
      Log_Err("[core] DomainShader compilation error: {:s}",
              static_cast<char*>(error_blob->GetBufferPointer()));
    }
    success = false;
  }

  if (error) {
    *error = error_blob
                 ? std::string(static_cast<char*>(error_blob->GetBufferPointer()))
                 : "";
  }

  if (error_blob) {
    error_blob->Release();
    error_blob = nullptr;
  }

  if (success) {
    success = Create(ps_blob->GetBufferPointer(),
                     static_cast<int32_t>(ps_blob->GetBufferSize()));
    ps_blob->Release();
  }

#ifndef _WIN64
  _controlfp_s(&fpu_control_word, fpu_control_word, 0xffffffff);
#endif

  return success;
}

bool CCoreDX11DomainShader::CreateFromBlob(void* code_blob,
                                           int32_t code_blob_size) {
  Release();
  bool success = true;
  uint32_t tmp = 0;
  _controlfp_s(&tmp, _RC_NEAR, _MCW_RC);
  success = Create(code_blob, code_blob_size);
#ifndef _WIN64
  _controlfp_s(&tmp, tmp, 0xffffffff);
#endif
  return success;
}

CCoreDX11HullShader::CCoreDX11HullShader(CCoreDX11Device* device)
    : CCoreHullShader(device) {
  d3d_device_ = device->GetDevice();
  d3d_device_context_ = device->GetDeviceContext();
  hull_shader_handle_ = nullptr;
}

CCoreDX11HullShader::~CCoreDX11HullShader() { Release(); }

bool CCoreDX11HullShader::Create(void* binary, int32_t length) {
  if (!binary || length <= 0) return false;
  FetchBinary(binary, length);
  const HRESULT result =
      d3d_device_->CreateHullShader(binary, length, nullptr, &hull_shader_handle_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] HullShader Creation error ({:s})", error.ErrorMessage());
  }
  return result == S_OK;
}

void CCoreDX11HullShader::Release() {
  if (hull_shader_handle_) hull_shader_handle_->Release();
  hull_shader_handle_ = nullptr;
}

bool CCoreDX11HullShader::Apply() {
  if (!hull_shader_handle_) return false;
  d3d_device_context_->HSSetShader(hull_shader_handle_, nullptr, 0);
  return true;
}

bool CCoreDX11HullShader::CompileAndCreate(std::string* error) {
  if (!D3DCompileFunc && !InitShaderCompiler()) return false;

  Release();

  bool success = true;

  ID3D10Blob* ps_blob = nullptr;
  ID3D10Blob* error_blob = nullptr;

  uint32_t fpu_control_word = 0;
  _controlfp_s(&fpu_control_word, _RC_NEAR, _MCW_RC);

  if (D3DCompileFunc(code_.c_str(), code_.size(), nullptr, nullptr, nullptr,
                     entry_function_.c_str(), shader_version_.c_str(), 0, 0,
                     &ps_blob, &error_blob) != S_OK) {
    if (!error) {
      Log_Err("[core] HullShader compilation error: {:s}",
              static_cast<char*>(error_blob->GetBufferPointer()));
    }
    success = false;
  }

  if (error) {
    *error = error_blob
                 ? std::string(static_cast<char*>(error_blob->GetBufferPointer()))
                 : "";
  }

  if (error_blob) {
    error_blob->Release();
    error_blob = nullptr;
  }

  if (success) {
    success = Create(ps_blob->GetBufferPointer(),
                     static_cast<int32_t>(ps_blob->GetBufferSize()));
    ps_blob->Release();
  }

#ifndef _WIN64
  _controlfp_s(&fpu_control_word, fpu_control_word, 0xffffffff);
#endif

  return success;
}

bool CCoreDX11HullShader::CreateFromBlob(void* code_blob, int32_t code_blob_size) {
  Release();
  bool success = true;
  uint32_t fpu_control_word = 0;
  _controlfp_s(&fpu_control_word, _RC_NEAR, _MCW_RC);
  success = Create(code_blob, code_blob_size);
#ifndef _WIN64
  _controlfp_s(&fpu_control_word, fpu_control_word, 0xffffffff);
#endif
  return success;
}

CCoreDX11ComputeShader::CCoreDX11ComputeShader(CCoreDX11Device* device)
    : CCoreComputeShader(device) {
  d3d_device_ = device->GetDevice();
  d3d_device_context_ = device->GetDeviceContext();
  compute_shader_handle_ = nullptr;
}

CCoreDX11ComputeShader::~CCoreDX11ComputeShader() { Release(); }

bool CCoreDX11ComputeShader::Create(void* binary, int32_t length) {
  if (!binary || length <= 0) return false;
  FetchBinary(binary, length);
  const HRESULT result =
      d3d_device_->CreateComputeShader(binary, length, nullptr, &compute_shader_handle_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] ComputeShader Creation error ({:s})", error.ErrorMessage());
  }
  return result == S_OK;
}

void CCoreDX11ComputeShader::Release() {
  if (compute_shader_handle_) compute_shader_handle_->Release();
  compute_shader_handle_ = nullptr;
}

bool CCoreDX11ComputeShader::Apply() {
  if (!compute_shader_handle_) return false;
  d3d_device_context_->CSSetShader(compute_shader_handle_, nullptr, 0);
  return true;
}

bool CCoreDX11ComputeShader::CompileAndCreate(std::string* error) {
  if (!D3DCompileFunc && !InitShaderCompiler()) return false;

  Release();

  bool success = true;

  ID3D10Blob* ps_blob = nullptr;
  ID3D10Blob* error_blob = nullptr;

  uint32_t fpu_control_word = 0;
  _controlfp_s(&fpu_control_word, _RC_NEAR, _MCW_RC);

  if (D3DCompileFunc(code_.c_str(), code_.size(), nullptr, nullptr, nullptr,
                     entry_function_.c_str(), shader_version_.c_str(), 0, 0,
                     &ps_blob, &error_blob) != S_OK) {
    if (!error) {
      Log_Err("[core] ComputeShader compilation error: {:s}",
              static_cast<char*>(error_blob->GetBufferPointer()));
    }
    success = false;
  }

  if (error) {
    *error = error_blob
                 ? std::string(static_cast<char*>(error_blob->GetBufferPointer()))
                 : "";
  }

  if (error_blob) {
    error_blob->Release();
    error_blob = nullptr;
  }

  if (success) {
    success = Create(ps_blob->GetBufferPointer(),
                     static_cast<int32_t>(ps_blob->GetBufferSize()));
    ps_blob->Release();
  }

#ifndef _WIN64
  _controlfp_s(&fpu_control_word, fpu_control_word, 0xffffffff);
#endif

  return success;
}

bool CCoreDX11ComputeShader::CreateFromBlob(void* code_blob,
                                            int32_t code_blob_size) {
  Release();
  bool success = true;
  uint32_t fpu_control_word = 0;
  _controlfp_s(&fpu_control_word, _RC_NEAR, _MCW_RC);
  success = Create(code_blob, code_blob_size);
#ifndef _WIN64
  _controlfp_s(&fpu_control_word, fpu_control_word, 0xffffffff);
#endif
  return success;
}

}  // namespace renderer
