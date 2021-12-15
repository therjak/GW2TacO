#include "dx11_shader.h"
#ifdef CORE_API_DX11

#include <comdef.h>

#include <array>

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

void* GetFunctionFromD3DXDLL(const std::string& FunctName) {
  HMODULE dll = nullptr;

  constexpr std::array<std::string_view, 34> CompilerDLLs = {
      "d3dx11_47.dll", "d3dx11_46.dll", "d3dx11_45.dll", "d3dx11_44.dll",
      "d3dx11_43.dll", "d3dx11_42.dll", "d3dx11_41.dll", "d3dx11_40.dll",
      "d3dx11_39.dll", "d3dx11_38.dll", "d3dx11_37.dll", "d3dx11_36.dll",
      "d3dx11_35.dll", "d3dx11_34.dll", "d3dx11_33.dll", "d3dx11_32.dll",
      "d3dx11_31.dll", "d3dx10_47.dll", "d3dx10_46.dll", "d3dx10_45.dll",
      "d3dx10_44.dll", "d3dx10_43.dll", "d3dx10_42.dll", "d3dx10_41.dll",
      "d3dx10_40.dll", "d3dx10_39.dll", "d3dx10_38.dll", "d3dx10_37.dll",
      "d3dx10_36.dll", "d3dx10_35.dll", "d3dx10_34.dll", "d3dx10_33.dll",
      "d3dx10_32.dll", "d3dx10_31.dll",
  };

  for (const auto& cd : CompilerDLLs) {
    dll = LoadLibraryA(cd.data());
    if (dll) {
      void* func = GetProcAddress(dll, FunctName.c_str());
      if (func) {
        LOG_NFO("[core] Successfully loaded %s from %s", FunctName.c_str(),
                cd.data());
        return func;
      }
    }
  }

  LOG_ERR("[core] Failed to load %s from d3dx**_xx.dll!", FunctName.c_str());
  return nullptr;
}

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
        LOG_NFO("[core] Successfully loaded %s from %s", FunctName.c_str(),
                cd.data());
        return func;
      }
    }
  }

  LOG_ERR("[core] Failed to load %s from d3dcompile_xx.dll!",
          FunctName.c_str());
  return nullptr;
}

bool InitShaderCompiler() {
  if (D3DCompileFunc || D3DXCompileFunc) return true;
  D3DCompileFunc = static_cast<d3d_compile_func*>(
      GetFunctionFromD3DCompileDLL(_T("D3DCompile")));
  return D3DCompileFunc != nullptr;
}

//////////////////////////////////////////////////////////////////////////
// vertex shader

CCoreDX11VertexShader::CCoreDX11VertexShader(CCoreDX11Device* dev)
    : CCoreVertexShader(dev) {
  Dev = dev->GetDevice();
  DeviceContext = dev->GetDeviceContext();
  VertexShaderHandle = nullptr;
}

CCoreDX11VertexShader::~CCoreDX11VertexShader() { Release(); }

bool CCoreDX11VertexShader::Create(void* Binary, int32_t Length) {
  if (!Binary || Length <= 0) return false;
  FetchBinary(Binary, Length);
  const HRESULT res =
      Dev->CreateVertexShader(Binary, Length, nullptr, &VertexShaderHandle);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR(_T("[core] VertexShader Creation error (%s)"), err.ErrorMessage());
  }
  return res == S_OK;
}

void CCoreDX11VertexShader::Release() {
  if (VertexShaderHandle) VertexShaderHandle->Release();
  VertexShaderHandle = nullptr;
}

bool CCoreDX11VertexShader::Apply() {
  if (!VertexShaderHandle) return false;
  DeviceContext->VSSetShader(VertexShaderHandle, nullptr, 0);
  return true;
}

bool CCoreDX11VertexShader::CompileAndCreate(std::string* Err) {
  if (!D3DCompileFunc && !InitShaderCompiler()) return false;

  Release();

  bool Success = true;

  ID3D10Blob* PS = nullptr;
  ID3D10Blob* Error = nullptr;

  uint32_t tmp;
  _controlfp_s(&tmp, _RC_NEAR, _MCW_RC);

#ifdef CORE_API_D3DX
  if (D3DX11CompileFromMemory(Code.c_str(), Code.size(), NULL, NULL, NULL,
                              EntryFunction.c_str(), ShaderVersion.c_str(),
                              D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, NULL, &PS,
                              &Error, 0) != S_OK)
#else
  if (D3DCompileFunc(Code.c_str(), Code.size(), nullptr, nullptr, nullptr,
                     EntryFunction.c_str(), ShaderVersion.c_str(), 0, 0, &PS,
                     &Error) != S_OK)
#endif
  {
    if (!Err)
      LOG_ERR(_T("[core] VertexShader compilation error: %s"),
              (char*)Error->GetBufferPointer());
    Success = false;
  }

  if (Err)
    *Err = Error ? std::string(static_cast<char*>(Error->GetBufferPointer()))
                 : _T("");

  if (Success) {
    Success = Create(PS->GetBufferPointer(),
                     static_cast<int32_t>(PS->GetBufferSize()));
    PS->Release();
  }

#ifndef _WIN64
  _controlfp_s(&tmp, tmp, 0xffffffff);
#endif

  return Success;
}

bool CCoreDX11VertexShader::CreateFromBlob(void* CodeBlob,
                                           int32_t CodeBlobSize) {
  Release();
  bool Success = true;
  uint32_t tmp;
  _controlfp_s(&tmp, _RC_NEAR, _MCW_RC);
  Success = Create(CodeBlob, CodeBlobSize);
#ifndef _WIN64
  _controlfp_s(&tmp, tmp, 0xffffffff);
#endif
  return Success;
}

//////////////////////////////////////////////////////////////////////////
// Pixel shader

CCoreDX11PixelShader::CCoreDX11PixelShader(CCoreDX11Device* dev)
    : CCorePixelShader(dev) {
  Dev = dev->GetDevice();
  DeviceContext = dev->GetDeviceContext();
  PixelShaderHandle = nullptr;
}

CCoreDX11PixelShader::~CCoreDX11PixelShader() { Release(); }

bool CCoreDX11PixelShader::Create(void* Binary, int32_t Length) {
  if (!Binary || Length <= 0) return false;
  FetchBinary(Binary, Length);
  const HRESULT res =
      Dev->CreatePixelShader(Binary, Length, nullptr, &PixelShaderHandle);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR(_T("[core] PixelShader Creation error (%s)"), err.ErrorMessage());
  }
  return res == S_OK;
}

void CCoreDX11PixelShader::Release() {
  if (PixelShaderHandle) PixelShaderHandle->Release();
  PixelShaderHandle = nullptr;
}

bool CCoreDX11PixelShader::Apply() {
  if (!PixelShaderHandle) return false;
  DeviceContext->PSSetShader(PixelShaderHandle, nullptr, 0);
  return true;
}

bool CCoreDX11PixelShader::CompileAndCreate(std::string* Err) {
  if (!D3DCompileFunc && !InitShaderCompiler()) return false;

  Release();

  bool Success = true;

  ID3D10Blob* PS = nullptr;
  ID3D10Blob* Error = nullptr;

  uint32_t tmp;
  _controlfp_s(&tmp, _RC_NEAR, _MCW_RC);

#ifdef CORE_API_D3DX
  if (D3DX11CompileFromMemory(Code.c_str(), Code.size(), NULL, NULL, NULL,
                              EntryFunction.c_str(), ShaderVersion.c_str(),
                              D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, NULL, &PS,
                              &Error, 0) != S_OK)
#else
  if (D3DCompileFunc(Code.c_str(), Code.size(), nullptr, nullptr, nullptr,
                     EntryFunction.c_str(), ShaderVersion.c_str(), 0, 0, &PS,
                     &Error) != S_OK)
#endif
  {
    if (!Err)
      LOG_ERR(_T("[core] PixelShader compilation error: %s"),
              (char*)Error->GetBufferPointer());
    Success = false;
  }

  if (Err)
    *Err = Error ? std::string(static_cast<char*>(Error->GetBufferPointer()))
                 : _T("");

  if (Success) {
    Success = Create(PS->GetBufferPointer(),
                     static_cast<int32_t>(PS->GetBufferSize()));
    PS->Release();
  }

#ifndef _WIN64
  _controlfp_s(&tmp, tmp, 0xffffffff);
#endif

  return Success;
}

bool CCoreDX11PixelShader::CreateFromBlob(void* CodeBlob,
                                          int32_t CodeBlobSize) {
  Release();
  bool Success = true;
  uint32_t tmp;
  _controlfp_s(&tmp, _RC_NEAR, _MCW_RC);
  Success = Create(CodeBlob, CodeBlobSize);
#ifndef _WIN64
  _controlfp_s(&tmp, tmp, 0xffffffff);
#endif
  return Success;
}

//////////////////////////////////////////////////////////////////////////
// Geometry shader

CCoreDX11GeometryShader::CCoreDX11GeometryShader(CCoreDX11Device* dev)
    : CCoreGeometryShader(dev) {
  Dev = dev->GetDevice();
  DeviceContext = dev->GetDeviceContext();
  GeometryShaderHandle = nullptr;
}

CCoreDX11GeometryShader::~CCoreDX11GeometryShader() { Release(); }

bool CCoreDX11GeometryShader::Create(void* Binary, int32_t Length) {
  if (!Binary || Length <= 0) return false;
  FetchBinary(Binary, Length);
  const HRESULT res =
      Dev->CreateGeometryShader(Binary, Length, nullptr, &GeometryShaderHandle);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR(_T("[core] GeometryShader Creation error (%s)"),
            err.ErrorMessage());
  }
  return res == S_OK;
}

void CCoreDX11GeometryShader::Release() {
  if (GeometryShaderHandle) GeometryShaderHandle->Release();
  GeometryShaderHandle = nullptr;
}

bool CCoreDX11GeometryShader::Apply() {
  if (!GeometryShaderHandle) return false;
  DeviceContext->GSSetShader(GeometryShaderHandle, nullptr, 0);
  return true;
}

bool CCoreDX11GeometryShader::CompileAndCreate(std::string* Err) {
  if (!D3DCompileFunc && !InitShaderCompiler()) return false;

  Release();

  bool Success = true;

  ID3D10Blob* PS = nullptr;
  ID3D10Blob* Error = nullptr;

  uint32_t tmp;
  _controlfp_s(&tmp, _RC_NEAR, _MCW_RC);

#ifdef CORE_API_D3DX
  if (D3DX11CompileFromMemory(Code.c_str(), Code.size(), NULL, NULL, NULL,
                              EntryFunction.c_str(), ShaderVersion.c_str(),
                              D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, NULL, &PS,
                              &Error, 0) != S_OK)
#else
  if (D3DCompileFunc(Code.c_str(), Code.size(), nullptr, nullptr, nullptr,
                     EntryFunction.c_str(), ShaderVersion.c_str(), 0, 0, &PS,
                     &Error) != S_OK)
#endif
  {
    if (!Err)
      LOG_ERR(_T("[core] GeometryShader compilation error: %s"),
              (char*)Error->GetBufferPointer());
    Success = false;
  }

  if (Err)
    *Err = Error ? std::string(static_cast<char*>(Error->GetBufferPointer()))
                 : _T("");

  if (Success) {
    Success = Create(PS->GetBufferPointer(),
                     static_cast<int32_t>(PS->GetBufferSize()));
    PS->Release();
  }

#ifndef _WIN64
  _controlfp_s(&tmp, tmp, 0xffffffff);
#endif

  return Success;
}

bool CCoreDX11GeometryShader::CreateFromBlob(void* CodeBlob,
                                             int32_t CodeBlobSize) {
  Release();
  bool Success = true;
  uint32_t tmp;
  _controlfp_s(&tmp, _RC_NEAR, _MCW_RC);
  Success = Create(CodeBlob, CodeBlobSize);
#ifndef _WIN64
  _controlfp_s(&tmp, tmp, 0xffffffff);
#endif
  return Success;
}

//////////////////////////////////////////////////////////////////////////
// Domain shader

CCoreDX11DomainShader::CCoreDX11DomainShader(CCoreDX11Device* dev)
    : CCoreDomainShader(dev) {
  Dev = dev->GetDevice();
  DeviceContext = dev->GetDeviceContext();
  DomainShaderHandle = nullptr;
}

CCoreDX11DomainShader::~CCoreDX11DomainShader() { Release(); }

bool CCoreDX11DomainShader::Create(void* Binary, int32_t Length) {
  if (!Binary || Length <= 0) return false;
  FetchBinary(Binary, Length);
  const HRESULT res =
      Dev->CreateDomainShader(Binary, Length, nullptr, &DomainShaderHandle);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR(_T("[core] DomainShader Creation error (%s)"), err.ErrorMessage());
  }
  return res == S_OK;
}

void CCoreDX11DomainShader::Release() {
  if (DomainShaderHandle) DomainShaderHandle->Release();
  DomainShaderHandle = nullptr;
}

bool CCoreDX11DomainShader::Apply() {
  if (!DomainShaderHandle) return false;
  DeviceContext->DSSetShader(DomainShaderHandle, nullptr, 0);
  return true;
}

bool CCoreDX11DomainShader::CompileAndCreate(std::string* Err) {
  if (!D3DCompileFunc && !InitShaderCompiler()) return false;

  Release();

  bool Success = true;

  ID3D10Blob* PS = nullptr;
  ID3D10Blob* Error = nullptr;

  uint32_t tmp;
  _controlfp_s(&tmp, _RC_NEAR, _MCW_RC);

#ifdef CORE_API_D3DX
  if (D3DX11CompileFromMemory(Code.c_str(), Code.size(), NULL, NULL, NULL,
                              EntryFunction.c_str(), ShaderVersion.c_str(),
                              D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, NULL, &PS,
                              &Error, 0) != S_OK)
#else
  if (D3DCompileFunc(Code.c_str(), Code.size(), nullptr, nullptr, nullptr,
                     EntryFunction.c_str(), ShaderVersion.c_str(), 0, 0, &PS,
                     &Error) != S_OK)
#endif
  {
    if (!Err)
      LOG_ERR(_T("[core] DomainShader compilation error: %s"),
              (char*)Error->GetBufferPointer());
    Success = false;
  }

  if (Err)
    *Err = Error ? std::string(static_cast<char*>(Error->GetBufferPointer()))
                 : _T("");

  if (Success) {
    Success = Create(PS->GetBufferPointer(),
                     static_cast<int32_t>(PS->GetBufferSize()));
    PS->Release();
  }

#ifndef _WIN64
  _controlfp_s(&tmp, tmp, 0xffffffff);
#endif

  return Success;
}

bool CCoreDX11DomainShader::CreateFromBlob(void* CodeBlob,
                                           int32_t CodeBlobSize) {
  Release();
  bool Success = true;
  uint32_t tmp;
  _controlfp_s(&tmp, _RC_NEAR, _MCW_RC);
  Success = Create(CodeBlob, CodeBlobSize);
#ifndef _WIN64
  _controlfp_s(&tmp, tmp, 0xffffffff);
#endif
  return Success;
}

//////////////////////////////////////////////////////////////////////////
// Hull shader

CCoreDX11HullShader::CCoreDX11HullShader(CCoreDX11Device* dev)
    : CCoreHullShader(dev) {
  Dev = dev->GetDevice();
  DeviceContext = dev->GetDeviceContext();
  HullShaderHandle = nullptr;
}

CCoreDX11HullShader::~CCoreDX11HullShader() { Release(); }

bool CCoreDX11HullShader::Create(void* Binary, int32_t Length) {
  if (!Binary || Length <= 0) return false;
  FetchBinary(Binary, Length);
  const HRESULT res =
      Dev->CreateHullShader(Binary, Length, nullptr, &HullShaderHandle);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR(_T("[core] HullShader Creation error (%s)"), err.ErrorMessage());
  }
  return res == S_OK;
}

void CCoreDX11HullShader::Release() {
  if (HullShaderHandle) HullShaderHandle->Release();
  HullShaderHandle = nullptr;
}

bool CCoreDX11HullShader::Apply() {
  if (!HullShaderHandle) return false;
  DeviceContext->HSSetShader(HullShaderHandle, nullptr, 0);
  return true;
}

bool CCoreDX11HullShader::CompileAndCreate(std::string* Err) {
  if (!D3DCompileFunc && !InitShaderCompiler()) return false;

  Release();

  bool Success = true;

  ID3D10Blob* PS = nullptr;
  ID3D10Blob* Error = nullptr;

  uint32_t tmp;
  _controlfp_s(&tmp, _RC_NEAR, _MCW_RC);

#ifdef CORE_API_D3DX
  if (D3DX11CompileFromMemory(Code.c_str(), Code.size(), NULL, NULL, NULL,
                              EntryFunction.c_str(), ShaderVersion.c_str(),
                              D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, NULL, &PS,
                              &Error, 0) != S_OK)
#else
  if (D3DCompileFunc(Code.c_str(), Code.size(), nullptr, nullptr, nullptr,
                     EntryFunction.c_str(), ShaderVersion.c_str(), 0, 0, &PS,
                     &Error) != S_OK)
#endif

  {
    if (!Err)
      LOG_ERR(_T("[core] HullShader compilation error: %s"),
              (char*)Error->GetBufferPointer());
    Success = false;
  }

  if (Err)
    *Err = Error ? std::string(static_cast<char*>(Error->GetBufferPointer()))
                 : _T("");

  if (Success) {
    Success = Create(PS->GetBufferPointer(),
                     static_cast<int32_t>(PS->GetBufferSize()));
    PS->Release();
  }

#ifndef _WIN64
  _controlfp_s(&tmp, tmp, 0xffffffff);
#endif

  return Success;
}

bool CCoreDX11HullShader::CreateFromBlob(void* CodeBlob, int32_t CodeBlobSize) {
  Release();
  bool Success = true;
  uint32_t tmp;
  _controlfp_s(&tmp, _RC_NEAR, _MCW_RC);
  Success = Create(CodeBlob, CodeBlobSize);
#ifndef _WIN64
  _controlfp_s(&tmp, tmp, 0xffffffff);
#endif
  return Success;
}

//////////////////////////////////////////////////////////////////////////
// Compute shader

CCoreDX11ComputeShader::CCoreDX11ComputeShader(CCoreDX11Device* dev)
    : CCoreComputeShader(dev) {
  Dev = dev->GetDevice();
  DeviceContext = dev->GetDeviceContext();
  ComputeShaderHandle = nullptr;
}

CCoreDX11ComputeShader::~CCoreDX11ComputeShader() { Release(); }

bool CCoreDX11ComputeShader::Create(void* Binary, int32_t Length) {
  if (!Binary || Length <= 0) return false;
  FetchBinary(Binary, Length);
  const HRESULT res =
      Dev->CreateComputeShader(Binary, Length, nullptr, &ComputeShaderHandle);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR(_T("[core] ComputeShader Creation error (%s)"), err.ErrorMessage());
  }
  return res == S_OK;
}

void CCoreDX11ComputeShader::Release() {
  if (ComputeShaderHandle) ComputeShaderHandle->Release();
  ComputeShaderHandle = nullptr;
}

bool CCoreDX11ComputeShader::Apply() {
  if (!ComputeShaderHandle) return false;
  DeviceContext->CSSetShader(ComputeShaderHandle, nullptr, 0);
  return true;
}

bool CCoreDX11ComputeShader::CompileAndCreate(std::string* Err) {
  if (!D3DCompileFunc && !InitShaderCompiler()) return false;

  Release();

  bool Success = true;

  ID3D10Blob* PS = nullptr;
  ID3D10Blob* Error = nullptr;

  uint32_t tmp;
  _controlfp_s(&tmp, _RC_NEAR, _MCW_RC);

#ifdef CORE_API_D3DX
  if (D3DX11CompileFromMemory(Code.c_str(), Code.size(), NULL, NULL, NULL,
                              EntryFunction.c_str(), ShaderVersion.c_str(),
                              D3DCOMPILE_OPTIMIZATION_LEVEL0, 0, NULL, &PS,
                              &Error, 0) != S_OK)
#else
  if (D3DCompileFunc(Code.c_str(), Code.size(), nullptr, nullptr, nullptr,
                     EntryFunction.c_str(), ShaderVersion.c_str(), 0, 0, &PS,
                     &Error) != S_OK)
#endif

  {
    if (!Err)
      LOG_ERR(_T("[core] ComputeShader compilation error: %s"),
              (char*)Error->GetBufferPointer());
    Success = false;
  }

  if (Err)
    *Err = Error ? std::string(static_cast<char*>(Error->GetBufferPointer()))
                 : _T("");

  if (Success) {
    Success = Create(PS->GetBufferPointer(),
                     static_cast<int32_t>(PS->GetBufferSize()));
    PS->Release();
  }

#ifndef _WIN64
  _controlfp_s(&tmp, tmp, 0xffffffff);
#endif

  return Success;
}

bool CCoreDX11ComputeShader::CreateFromBlob(void* CodeBlob,
                                            int32_t CodeBlobSize) {
  Release();
  bool Success = true;
  uint32_t tmp;
  _controlfp_s(&tmp, _RC_NEAR, _MCW_RC);
  Success = Create(CodeBlob, CodeBlobSize);
#ifndef _WIN64
  _controlfp_s(&tmp, tmp, 0xffffffff);
#endif
  return Success;
}

#else
NoEmptyFile();
#endif
