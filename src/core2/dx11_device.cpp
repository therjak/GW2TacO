#include "src/core2/dx11_device.h"

#include <comdef.h>
#include <dcomp.h>

#include <algorithm>
#include <vector>

#include "src/core2/dx11_constant_buffer.h"
#include "src/core2/dx11_index_buffer.h"
#include "src/core2/dx11_render_state.h"
#include "src/core2/dx11_shader.h"
#include "src/core2/dx11_texture.h"
#include "src/core2/dx11_vertex_buffer.h"
#include "src/core2/dx11_vertex_format.h"
#pragma comment(lib, "DXGI.lib")
//#pragma comment(lib,"dcomp.lib")

typedef HRESULT(__stdcall* DCompositionCreateDeviceCallback)(
    _In_opt_ IDXGIDevice* dxgiDevice, _In_ REFIID iid,
    _Outptr_ void** dcompositionDevice);
DCompositionCreateDeviceCallback DCompositionCreateDeviceFunc = nullptr;

#ifdef CORE_API_DX11

CCoreDX11Device::CCoreDX11Device() = default;

CCoreDX11Device::~CCoreDX11Device() {
  if (OcclusionQuery) {
    OcclusionQuery->Release();
  }

  if (BackBufferView) {
    BackBufferView->Release();
  }
  if (DepthBufferView) {
    DepthBufferView->Release();
  }
  if (DepthBuffer) {
    DepthBuffer->Release();
  }
  if (SwapChain) {
    SwapChain->SetFullscreenState(false, nullptr);
    SwapChain->Release();
  }

  if (DeviceContext) {
    DeviceContext->ClearState();
    DeviceContext->Flush();
    DeviceContext->Release();
  }

  if (Device) {
    ID3D11Debug* dbg = nullptr;
    Device->QueryInterface(__uuidof(ID3D11Debug),
                           reinterpret_cast<void**>(&dbg));
    if (dbg) {
      LOG_NFO("[core] Dumping Live objects before freeing device:");
      dbg->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
      dbg->Release();
    }
  }
  if (Device) Device->Release();
}

void CCoreDX11Device::ResetPrivateResources() {}

#define BACKBUFFERFORMAT D3DFMT_A8R8G8B8

bool CCoreDX11Device::CreateBackBuffer(int32_t XRes, int32_t YRes) {
  if (BackBufferView) BackBufferView->Release();

  FORCEDDEBUGLOG("creating backbuffer");

  HRESULT res = S_OK;
  ID3D11Texture2D* bb;

  res = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                             reinterpret_cast<LPVOID*>(&bb));
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[core] DirectX11 Swapchain buffer acquisition failed (%s)",
            err.ErrorMessage());
    return false;
  }

  FORCEDDEBUGLOG("getbuffer called");

  res = Device->CreateRenderTargetView(bb, nullptr, &BackBufferView);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[core] DirectX11 Rendertarget View creation failed (%s)",
            err.ErrorMessage());
    return false;
  }

  FORCEDDEBUGLOG("createrendertargetview called");

  res = bb->Release();
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[core] DirectX11 Swapchain buffer texture release failed (%s)",
            err.ErrorMessage());
    // return false;
  }

  FORCEDDEBUGLOG("previous buffer released");

  return true;
}

bool CCoreDX11Device::CreateDepthBuffer(int32_t XRes, int32_t YRes) {
  if (DepthBufferView) {
    DepthBufferView->Release();
  }
  if (DepthBuffer) {
    DepthBuffer->Release();
  }

  FORCEDDEBUGLOG("creating depthbuffer");

  HRESULT res = S_OK;

  D3D11_TEXTURE2D_DESC depthBufferDesc;
  D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

  memset(&depthBufferDesc, 0, sizeof(depthBufferDesc));

  depthBufferDesc.Width = XRes;
  depthBufferDesc.Height = YRes;
  depthBufferDesc.MipLevels = 1;
  depthBufferDesc.ArraySize = 1;
  depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthBufferDesc.SampleDesc.Count = 1;
  depthBufferDesc.SampleDesc.Quality = 0;
  depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depthBufferDesc.CPUAccessFlags = 0;
  depthBufferDesc.MiscFlags = 0;

  res = Device->CreateTexture2D(&depthBufferDesc, nullptr, &DepthBuffer);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[core] DirectX11 Depth Texture creation failed (%s)",
            err.ErrorMessage());
    return false;
  }

  FORCEDDEBUGLOG("createtexture called");

  ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

  depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depthStencilViewDesc.Texture2D.MipSlice = 0;

  res = Device->CreateDepthStencilView(DepthBuffer, &depthStencilViewDesc,
                                       &DepthBufferView);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[core] DirectX11 DepthStencil View creation failed (%s)",
            err.ErrorMessage());
    return false;
  }

  FORCEDDEBUGLOG("createdepthstencilview called");

  return true;
}

bool CCoreDX11Device::CreateClassicSwapChain(
    const uint32_t hWnd, const bool FullScreen, const int32_t XRes,
    const int32_t YRes, const int32_t AALevel, const int32_t RefreshRate) {
  LOG_NFO("[core] Creating classic swap chain");

  FORCEDDEBUGLOG("Initapi");
  HRESULT res = S_OK;

  DXGI_SWAP_CHAIN_DESC scd;
  memset(&scd, 0, sizeof(DXGI_SWAP_CHAIN_DESC));

  scd.BufferCount = 1;
  scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  scd.OutputWindow = (HWND)hWnd;
  scd.SampleDesc.Count = 1;
  scd.Windowed = !FullScreen;

#ifdef ENABLE_CORE_DEBUG_MODE
  res = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG,
      nullptr, NULL, D3D11_SDK_VERSION, &scd,
      reinterpret_cast<IDXGISwapChain**>(&SwapChain), &Device, nullptr,
      &DeviceContext);
  if (res != S_OK) {
    _com_error err(res);
    LOG_WARN(
        "[core] DirectX11 debug mode device creation failed. (%s) Trying "
        "without debug mode...",
        err.ErrorMessage());
#endif
    FORCEDDEBUGLOG("About to create d3d device");
    res = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, NULL,
        D3D11_SDK_VERSION, &scd, reinterpret_cast<IDXGISwapChain**>(&SwapChain),
        &Device, nullptr, &DeviceContext);
    if (res != S_OK) {
      _com_error error(res);
      LOG_ERR("[core] DirectX11 Device creation failed (%s)",
              error.ErrorMessage());
      return false;
    }
    FORCEDDEBUGLOG("D3D device created");
#ifdef ENABLE_CORE_DEBUG_MODE
  }
#endif

  if (!CreateBackBuffer(XRes, YRes)) {
    return false;
  }
  FORCEDDEBUGLOG("Backbuffer created");
  if (!CreateDepthBuffer(XRes, YRes)) {
    return false;
  }
  FORCEDDEBUGLOG("Depthbuffer created");
  DeviceContext->OMSetRenderTargets(1, &BackBufferView, DepthBufferView);

  FORCEDDEBUGLOG("rendertargets set");

  SetViewport(CRect(0, 0, XRes, YRes));
  FORCEDDEBUGLOG("viewports set");

  if (CreateDefaultRenderStates())
    LOG_NFO("[core] DirectX11 Device initialization successful.");

  FORCEDDEBUGLOG("default renderstates created");

  D3D11_QUERY_DESC queryDesc;
  memset(&queryDesc, 0, sizeof(queryDesc));
  queryDesc.Query = D3D11_QUERY_OCCLUSION;
  queryDesc.MiscFlags = 0;

  Device->CreateQuery(&queryDesc, &OcclusionQuery);

  return true;
}

bool CCoreDX11Device::CreateDirectCompositionSwapchain(
    const uint32_t hWnd, const bool FullScreen, const int32_t XRes,
    const int32_t YRes, const int32_t AALevel, const int32_t RefreshRate) {
  LOG_NFO("[core] Creating DirectComposition swap chain");

  FORCEDDEBUGLOG("Initapi");
  HRESULT res = S_OK;

  IDXGIFactory2* dxgiFactory;
#ifdef _DEBUG
  res = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, __uuidof(IDXGIFactory2),
                           reinterpret_cast<void**>(&dxgiFactory));
#else
  res = CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
#endif
  if (res != S_OK) {
    _com_error error(res);
    LOG_ERR("[core] DXGI factory creation failed (%s)", error.ErrorMessage());
    return false;
  }

#ifdef ENABLE_CORE_DEBUG_MODE
  res = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                          D3D11_CREATE_DEVICE_DEBUG, nullptr, NULL,
                          D3D11_SDK_VERSION, &Device, nullptr, &DeviceContext);
  if (res != S_OK) {
    _com_error err(res);
    LOG_WARN(
        "[core] DirectX11 debug mode device creation failed. (%s) Trying "
        "without debug mode...",
        err.ErrorMessage());
#endif
    FORCEDDEBUGLOG("About to create d3d device");
    res = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                            nullptr, NULL, D3D11_SDK_VERSION, &Device, nullptr,
                            &DeviceContext);
    if (res != S_OK) {
      _com_error error(res);
      LOG_ERR("[core] DirectX11 Device creation failed (%s)",
              error.ErrorMessage());
      return false;
    }
    FORCEDDEBUGLOG("D3D device created");
#ifdef ENABLE_CORE_DEBUG_MODE
  }
#endif

  constexpr unsigned int backBufferCount = 2;
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc{static_cast<UINT>(XRes),
                                      static_cast<UINT>(YRes),
                                      DXGI_FORMAT_R8G8B8A8_UNORM,
                                      false,
                                      {1, 0},
                                      DXGI_USAGE_RENDER_TARGET_OUTPUT,
                                      backBufferCount,
                                      DXGI_SCALING_STRETCH,
                                      DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
                                      DXGI_ALPHA_MODE_PREMULTIPLIED,
                                      0};
  res = dxgiFactory->CreateSwapChainForComposition(Device, &swapChainDesc,
                                                   nullptr, &SwapChain);
  if (res != S_OK) {
    _com_error error(res);
    LOG_ERR("[core] DirectX11 SwapChain creation failed (%s)",
            error.ErrorMessage());
    return false;
  }

  IDCompositionDevice* dcompDevice = nullptr;

  res = DCompositionCreateDeviceFunc(reinterpret_cast<IDXGIDevice*>(Device),
                                     __uuidof(IDCompositionDevice),
                                     reinterpret_cast<void**>(&dcompDevice));

  if (res != S_OK) {
    _com_error error(res);
    LOG_ERR("[core] DirectComposition device creation failed (%s)",
            error.ErrorMessage());
    return false;
  }

  IDCompositionTarget* dcompTarget = nullptr;
  res = dcompDevice->CreateTargetForHwnd(HWND(hWnd), true, &dcompTarget);

  if (res != S_OK) {
    _com_error error(res);
    LOG_ERR("[core] DirectComposition target creation failed (%s)",
            error.ErrorMessage());
    return false;
  }

  IDCompositionVisual* dcompVisual = nullptr;
  res = dcompDevice->CreateVisual(&dcompVisual);

  if (res != S_OK) {
    _com_error error(res);
    LOG_ERR("[core] DirectComposition visual creation failed (%s)",
            error.ErrorMessage());
    return false;
  }

  res = dcompVisual->SetContent(SwapChain);

  if (res != S_OK) {
    _com_error error(res);
    LOG_ERR(
        "[core] DirectComposition visual swapchain content setting failed (%s)",
        error.ErrorMessage());
    return false;
  }

  res = dcompTarget->SetRoot(dcompVisual);

  if (res != S_OK) {
    _com_error error(res);
    LOG_ERR("[core] DirectComposition setting target root visual failed (%s)",
            error.ErrorMessage());
    return false;
  }

  res = dcompDevice->Commit();

  if (res != S_OK) {
    _com_error error(res);
    LOG_ERR("[core] DirectComposition commit failed (%s)",
            error.ErrorMessage());
    return false;
  }

  dxgiFactory->Release();

  if (!CreateBackBuffer(XRes, YRes)) {
    return false;
  }
  FORCEDDEBUGLOG("Backbuffer created");
  if (!CreateDepthBuffer(XRes, YRes)) {
    return false;
  }
  FORCEDDEBUGLOG("Depthbuffer created");
  DeviceContext->OMSetRenderTargets(1, &BackBufferView, DepthBufferView);

  FORCEDDEBUGLOG("rendertargets set");

  SetViewport(CRect(0, 0, XRes, YRes));
  FORCEDDEBUGLOG("viewports set");

  if (CreateDefaultRenderStates())
    LOG_NFO("[core] DirectX11 Device initialization successful.");

  FORCEDDEBUGLOG("default renderstates created");

  D3D11_QUERY_DESC queryDesc;
  memset(&queryDesc, 0, sizeof(queryDesc));
  queryDesc.Query = D3D11_QUERY_OCCLUSION;
  queryDesc.MiscFlags = 0;

  Device->CreateQuery(&queryDesc, &OcclusionQuery);

  return true;
}

bool CCoreDX11Device::InitAPI(const uint32_t hWnd, const bool FullScreen,
                              const int32_t XRes, const int32_t YRes,
                              const int32_t AALevel /* =0 */,
                              const int32_t RefreshRate /* =60 */) {
  auto dcomp = LoadLibrary("dcomp.dll");

  if (dcomp) {
    DCompositionCreateDeviceFunc = DCompositionCreateDeviceCallback(
        GetProcAddress(dcomp, "DCompositionCreateDevice"));
  }

  if (!dcomp || !DCompositionCreateDeviceFunc)
    return CreateClassicSwapChain(hWnd, FullScreen, XRes, YRes, AALevel,
                                  RefreshRate);
  else
    return CreateDirectCompositionSwapchain(hWnd, FullScreen, XRes, YRes,
                                            AALevel, RefreshRate);

  if (dcomp) {
    FreeLibrary(dcomp);
  }
}

bool CCoreDX11Device::Initialize(CCoreWindowHandler* window,
                                 const int32_t AALevel) {
  FORCEDDEBUGLOG("Initializing DX11 device");
  Window = window;

  if (!InitAPI(Window->GetHandle(), Window->GetInitParameters().FullScreen,
               Window->GetXRes(), Window->GetYRes(), AALevel, 60))
    return false;

  FORCEDDEBUGLOG("InitAPI ran");

  ShowWindow((HWND)Window->GetHandle(), Window->GetInitParameters().Maximized
                                            ? SW_SHOWMAXIMIZED
                                            : SW_SHOWNORMAL);
  FORCEDDEBUGLOG("Showwindow ran");
  SetForegroundWindow((HWND)Window->GetHandle());
  FORCEDDEBUGLOG("Setforegroundwindow ran");
  SetFocus((HWND)Window->GetHandle());
  FORCEDDEBUGLOG("Setfocus ran");
  return true;
}

bool CCoreDX11Device::IsWindowed() {
  BOOL fs = false;
  IDXGIOutput* i = nullptr;

  if (SwapChain->GetFullscreenState(&fs, &i) != S_OK) {
    LOG_ERR("[core] Failed to get fullscreen state");
    return false;
  }

  if (i) {
    i->Release();
  }
  return fs;
}

void CCoreDX11Device::Resize(const int32_t xr, const int32_t yr) {
  if (xr <= 0 || yr <= 0) {
    LOG_WARN("[core] Trying to resize swapchain to invalid resolution: %d %d",
             xr, yr);
    return;
  }

  DXGI_SWAP_CHAIN_DESC desc;
  HRESULT res = SwapChain->GetDesc(&desc);
  if (res != S_OK) {
    LOG_ERR("[core] Failed to get swapchain description");
    return;
  }

  if (desc.BufferDesc.Width == xr && desc.BufferDesc.Height == yr) {
    return;
  }

  if (BackBufferView) {
    BackBufferView->Release();
  }
  if (DepthBufferView) {
    DepthBufferView->Release();
  }
  if (DepthBuffer) {
    DepthBuffer->Release();
  }
  BackBufferView = nullptr;
  DepthBufferView = nullptr;
  DepthBuffer = nullptr;

  res = SwapChain->ResizeBuffers(desc.BufferCount, xr, yr, DXGI_FORMAT_UNKNOWN,
                                 desc.Flags);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR(
        "[core] Failed to resize swapchain to %d %d (bufferCount: %d, flags: "
        "%d) (%s)",
        xr, yr, desc.BufferCount, desc.Flags, err.ErrorMessage());
    return;
  }

  if (!CreateBackBuffer(xr, yr)) {
    return;
  }
  if (!CreateDepthBuffer(xr, yr)) {
    return;
  }

  DeviceContext->OMSetRenderTargets(1, &BackBufferView, DepthBufferView);
  SetViewport(CRect(0, 0, xr, yr));
}

void CCoreDX11Device::SetFullScreenMode(const bool FullScreen, const int32_t xr,
                                        const int32_t yr) {
  LOG_NFO("[core] Switching fullscreen mode to %d", FullScreen);

  const HRESULT res = SwapChain->SetFullscreenState(FullScreen, nullptr);
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[core] Failed to set FullScreen mode to %d. (%s)", FullScreen,
            err.ErrorMessage());
    return;
  }
}

bool CCoreDX11Device::DeviceOk() { return true; }

//////////////////////////////////////////////////////////////////////////
// texture functions

std::unique_ptr<CCoreTexture2D> CCoreDX11Device::CreateTexture2D(
    const int32_t XRes, const int32_t YRes, const uint8_t* Data,
    const char BytesPerPixel, const COREFORMAT Format /* =COREFMT_A8R8G8B8 */,
    const bool RenderTarget /* =false */) {
  auto Result = std::make_unique<CCoreDX11Texture2D>(this);
  if (!Result->Create(XRes, YRes, Data, BytesPerPixel, Format, RenderTarget))
    Result.reset();
  return Result;
}

std::unique_ptr<CCoreTexture2D> CCoreDX11Device::CreateTexture2D(
    const uint8_t* Data, const int32_t Size) {
  auto Result = std::make_unique<CCoreDX11Texture2D>(this);
  if (!Result->Create(Data, Size)) {
    Result.reset();
  }
  return Result;
}

//////////////////////////////////////////////////////////////////////////
// vertexbuffer functions

std::unique_ptr<CCoreVertexBuffer> CCoreDX11Device::CreateVertexBuffer(
    const uint8_t* Data, const int32_t Size) {
  auto Result = std::make_unique<CCoreDX11VertexBuffer>(this);
  if (!Result->Create(Data, Size)) {
    Result.reset();
  }
  return Result;
}

std::unique_ptr<CCoreVertexBuffer> CCoreDX11Device::CreateVertexBufferDynamic(
    const int32_t Size) {
  auto Result = std::make_unique<CCoreDX11VertexBuffer>(this);
  if (!Result->CreateDynamic(Size)) {
    Result.reset();
  }
  return Result;
}

//////////////////////////////////////////////////////////////////////////
// indexbuffer functions

std::unique_ptr<CCoreIndexBuffer> CCoreDX11Device::CreateIndexBuffer(
    const int32_t IndexCount, const int32_t IndexSize) {
  auto Result = std::make_unique<CCoreDX11IndexBuffer>(this);
  if (!Result->Create(IndexCount, IndexSize)) {
    Result.reset();
  }
  return Result;
}

//////////////////////////////////////////////////////////////////////////
// vertexformat functions

std::unique_ptr<CCoreVertexFormat> CCoreDX11Device::CreateVertexFormat(
    const std::vector<COREVERTEXATTRIBUTE>& Attributes, CCoreVertexShader* vs) {
  auto Result = std::make_unique<CCoreDX11VertexFormat>(this);
  if (!Result->Create(Attributes, vs)) {
    Result.reset();
  }
  return Result;
}

//////////////////////////////////////////////////////////////////////////
// shader functions

std::unique_ptr<CCoreVertexShader> CCoreDX11Device::CreateVertexShader(
    LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
    std::string* Err) {
  if (Err) *Err = _T( "" );
  if (!Code || !CodeSize || !EntryFunction || !ShaderVersion) {
    return {};
  }

  auto s = std::make_unique<CCoreDX11VertexShader>(this);
  s->SetCode(Code, EntryFunction, ShaderVersion);

  if (!s->CompileAndCreate(Err)) {
    s.reset();
  }
  return s;
}

std::unique_ptr<CCorePixelShader> CCoreDX11Device::CreatePixelShader(
    LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
    std::string* Err) {
  if (Err) {
    *Err = _T( "" );
  }
  if (!Code || !CodeSize || !EntryFunction || !ShaderVersion) {
    return {};
  }

  auto s = std::make_unique<CCoreDX11PixelShader>(this);
  s->SetCode(Code, EntryFunction, ShaderVersion);

  if (!s->CompileAndCreate(Err)) {
    s.reset();
  }
  return s;
}

std::unique_ptr<CCoreVertexShader> CCoreDX11Device::CreateVertexShaderFromBlob(
    uint8_t* Code, int32_t CodeSize) {
  auto s = std::make_unique<CCoreDX11VertexShader>(this);
  if (!s->CreateFromBlob(Code, CodeSize)) {
    s.reset();
  }
  return s;
}

std::unique_ptr<CCorePixelShader> CCoreDX11Device::CreatePixelShaderFromBlob(
    uint8_t* Code, int32_t CodeSize) {
  auto s = std::make_unique<CCoreDX11PixelShader>(this);
  if (!s->CreateFromBlob(Code, CodeSize)) {
    s.reset();
  }
  return s;
}

std::unique_ptr<CCoreGeometryShader> CCoreDX11Device::CreateGeometryShader(
    LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
    std::string* Err) {
  if (Err) {
    *Err = _T( "" );
  }
  if (!Code || !CodeSize || !EntryFunction || !ShaderVersion) {
    return nullptr;
  }

  auto s = std::make_unique<CCoreDX11GeometryShader>(this);
  s->SetCode(Code, EntryFunction, ShaderVersion);

  if (!s->CompileAndCreate(Err)) {
    return nullptr;
  }
  return s;
}

std::unique_ptr<CCoreDomainShader> CCoreDX11Device::CreateDomainShader(
    LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
    std::string* Err) {
  if (Err) {
    *Err = _T( "" );
  }
  if (!Code || !CodeSize || !EntryFunction || !ShaderVersion) {
    return nullptr;
  }

  auto s = std::make_unique<CCoreDX11DomainShader>(this);
  s->SetCode(Code, EntryFunction, ShaderVersion);

  if (!s->CompileAndCreate(Err)) {
    return nullptr;
  }
  return s;
}

std::unique_ptr<CCoreHullShader> CCoreDX11Device::CreateHullShader(
    LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
    std::string* Err) {
  if (Err) {
    *Err = _T( "" );
  }
  if (!Code || !CodeSize || !EntryFunction || !ShaderVersion) {
    return nullptr;
  }

  auto s = std::make_unique<CCoreDX11HullShader>(this);
  s->SetCode(Code, EntryFunction, ShaderVersion);

  if (!s->CompileAndCreate(Err)) {
    return nullptr;
  }
  return s;
}

std::unique_ptr<CCoreComputeShader> CCoreDX11Device::CreateComputeShader(
    LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
    std::string* Err) {
  if (Err) {
    *Err = _T( "" );
  }
  if (!Code || !CodeSize || !EntryFunction || !ShaderVersion) {
    return nullptr;
  }

  auto s = std::make_unique<CCoreDX11ComputeShader>(this);
  s->SetCode(Code, EntryFunction, ShaderVersion);

  if (!s->CompileAndCreate(Err)) {
    return nullptr;
  }
  return s;
}

std::unique_ptr<CCoreVertexShader> CCoreDX11Device::CreateVertexShader() {
  return std::make_unique<CCoreDX11VertexShader>(this);
}

std::unique_ptr<CCorePixelShader> CCoreDX11Device::CreatePixelShader() {
  return std::make_unique<CCoreDX11PixelShader>(this);
}

std::unique_ptr<CCoreGeometryShader> CCoreDX11Device::CreateGeometryShader() {
  return std::make_unique<CCoreDX11GeometryShader>(this);
}

std::unique_ptr<CCoreHullShader> CCoreDX11Device::CreateHullShader() {
  return std::make_unique<CCoreDX11HullShader>(this);
}

std::unique_ptr<CCoreDomainShader> CCoreDX11Device::CreateDomainShader() {
  return std::make_unique<CCoreDX11DomainShader>(this);
}

std::unique_ptr<CCoreComputeShader> CCoreDX11Device::CreateComputeShader() {
  return std::make_unique<CCoreDX11ComputeShader>(this);
}

//////////////////////////////////////////////////////////////////////////
// renderstate

bool CCoreDX11Device::ApplyRenderState(const CORESAMPLER Sampler,
                                       const CORERENDERSTATE RenderState,
                                       const CORERENDERSTATEVALUE Value) {
  switch (RenderState) {
    case CORERS_BLENDSTATE: {
      if (!Value.BlendState) {
        DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
        CurrentBlendState = nullptr;
        return true;
      }
      return Value.BlendState->Apply();
    } break;
    case CORERS_RASTERIZERSTATE: {
      if (!Value.RasterizerState) {
        DeviceContext->RSSetState(nullptr);
        CurrentRasterizerState = nullptr;
        return true;
      }
      return Value.RasterizerState->Apply();
    } break;
    case CORERS_DEPTHSTENCILSTATE: {
      if (!Value.DepthStencilState) {
        DeviceContext->OMSetDepthStencilState(nullptr, 0);
        CurrentDepthStencilState = nullptr;
        return true;
      }
      return Value.DepthStencilState->Apply();
    } break;
    case CORERS_SAMPLERSTATE: {
      if (!Value.SamplerState) {
        return false;
      }
      return Value.SamplerState->Apply(Sampler);
    } break;
    case CORERS_TEXTURE: {
      if (!Value.Texture) {
        ID3D11ShaderResourceView* null[1];
        null[0] = nullptr;

        if (Sampler >= CORESMP_PS0 && Sampler <= CORESMP_PS15)
          DeviceContext->PSSetShaderResources(Sampler, 1, null);
        if (Sampler >= CORESMP_VS0 && Sampler <= CORESMP_VS3)
          DeviceContext->VSSetShaderResources(Sampler - CORESMP_VS0, 1, null);
        if (Sampler >= CORESMP_GS0 && Sampler <= CORESMP_GS3)
          DeviceContext->GSSetShaderResources(Sampler - CORESMP_GS0, 1, null);
        return true;
      }
      return ApplyTextureToSampler(Sampler, Value.Texture);
    } break;
    case CORERS_VERTEXFORMAT: {
      if (!Value.VertexFormat) {
        CurrentVertexFormatSize = 0;
        DeviceContext->IASetInputLayout(nullptr);
        return true;
      }

      CurrentVertexFormatSize = Value.VertexFormat->GetSize();
      return ApplyVertexFormat(Value.VertexFormat);
    } break;
    case CORERS_INDEXBUFFER: {
      if (!Value.IndexBuffer) {
        DeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
        return true;
      }
      return ApplyIndexBuffer(Value.IndexBuffer);
    } break;
    case CORERS_VERTEXSHADER: {
      if (!Value.VertexShader) {
        DeviceContext->VSSetShader(nullptr, nullptr, 0);
        return true;
      }
      return ApplyVertexShader(Value.VertexShader);
    } break;
    case CORERS_GEOMETRYSHADER: {
      if (!Value.GeometryShader) {
        DeviceContext->GSSetShader(nullptr, nullptr, 0);
        return true;
      }
      return ApplyGeometryShader(Value.GeometryShader);
    } break;
    case CORERS_HULLSHADER: {
      if (!Value.GeometryShader) {
        DeviceContext->HSSetShader(nullptr, nullptr, 0);
        return true;
      }
      return ApplyHullShader(Value.HullShader);
    } break;
    case CORERS_DOMAINSHADER: {
      if (!Value.DomainShader) {
        DeviceContext->DSSetShader(nullptr, nullptr, 0);
        return true;
      }
      return ApplyDomainShader(Value.DomainShader);
    } break;
    case CORERS_COMPUTESHADER: {
      if (!Value.ComputeShader) {
        DeviceContext->DSSetShader(nullptr, nullptr, 0);
        return true;
      }
      return ApplyComputeShader(Value.ComputeShader);
    } break;
    case CORERS_PIXELSHADER: {
      if (!Value.PixelShader) {
        DeviceContext->PSSetShader(nullptr, nullptr, 0);
        return true;
      }
      return ApplyPixelShader(Value.PixelShader);
    } break;
    default:
      return true;
  }
}

bool CCoreDX11Device::SetNoVertexBuffer() {
  DeviceContext->IASetVertexBuffers(0, 1, nullptr, nullptr, nullptr);
  return true;
}

bool CCoreDX11Device::CommitRenderStates() { return true; }

//////////////////////////////////////////////////////////////////////////
// display functions

bool CCoreDX11Device::BeginScene() { return true; }

bool CCoreDX11Device::EndScene() { return true; }

bool CCoreDX11Device::Clear(const bool clearPixels, const bool clearDepth,
                            const CColor& Color, const float Depth,
                            const int32_t Stencil) {
  const float col[4] = {Color.R() / 255.0f, Color.G() / 255.0f,
                        Color.B() / 255.0f, Color.A() / 255.0f};

  if (clearPixels) {
    DeviceContext->ClearRenderTargetView(BackBufferView, col);
  }

  if (clearDepth)
    DeviceContext->ClearDepthStencilView(
        DepthBufferView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, Depth,
        Stencil);

  return true;
}

bool CCoreDX11Device::Flip(bool Vsync) {
  HRESULT res;

  if (Vsync)
    res = SwapChain->Present(1, 0);
  else
    res = SwapChain->Present(0, 0);

  return res == S_OK;
}

bool CCoreDX11Device::DrawIndexedTriangles(int32_t Count, int32_t NumVertices) {
  DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  if (!ApplyRequestedRenderState()) {
    return false;
  }
  DeviceContext->DrawIndexed(Count * 3, 0, 0);
  return true;
}

bool CCoreDX11Device::DrawLines(int32_t Count) {
  DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
  if (!ApplyRequestedRenderState()) {
    return false;
  }
  DeviceContext->Draw(Count * 2, 0);
  return true;
}

bool CCoreDX11Device::DrawIndexedLines(int32_t Count, int32_t NumVertices) {
  DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
  if (!ApplyRequestedRenderState()) {
    return false;
  }
  DeviceContext->DrawIndexed(Count * 2, 0, 0);
  return true;
}

bool CCoreDX11Device::DrawTriangles(int32_t Count) {
  DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  if (!ApplyRequestedRenderState()) {
    return false;
  }
  DeviceContext->Draw(Count * 3, 0);
  return true;
}

bool CCoreDX11Device::SetViewport(CRect Viewport) {
  D3D11_VIEWPORT viewport;
  memset(&viewport, 0, sizeof(D3D11_VIEWPORT));

  viewport.TopLeftX = static_cast<float>(Viewport.x1);
  viewport.TopLeftY = static_cast<float>(Viewport.y1);
  viewport.Width = std::max(0.f, static_cast<float>(Viewport.Width()));
  viewport.Height = std::max(0.f, static_cast<float>(Viewport.Height()));
  viewport.MinDepth = 0;
  viewport.MaxDepth = 1;
  DeviceContext->RSSetViewports(1, &viewport);

  return true;
}

void CCoreDX11Device::SetShaderConstants(const CCoreConstantBuffer* Buffers) {
  void* buffers[16];

  if (Buffers) {
    buffers[0] = Buffers->GetBufferPointer();
  } else {
    memset(buffers, 0, 16 * sizeof(void*));
  }

  DeviceContext->VSSetConstantBuffers(
      0, 1, reinterpret_cast<ID3D11Buffer**>(buffers));
  DeviceContext->GSSetConstantBuffers(
      0, 1, reinterpret_cast<ID3D11Buffer**>(buffers));
  DeviceContext->PSSetConstantBuffers(
      0, 1, reinterpret_cast<ID3D11Buffer**>(buffers));
}

std::unique_ptr<CCoreConstantBuffer> CCoreDX11Device::CreateConstantBuffer() {
  return std::make_unique<CCoreDX11ConstantBuffer>(this);
}

std::unique_ptr<CCoreBlendState> CCoreDX11Device::CreateBlendState() {
  return std::make_unique<CCoreDX11BlendState>(this);
}

std::unique_ptr<CCoreDepthStencilState>
CCoreDX11Device::CreateDepthStencilState() {
  return std::make_unique<CCoreDX11DepthStencilState>(this);
}

std::unique_ptr<CCoreRasterizerState> CCoreDX11Device::CreateRasterizerState() {
  return std::make_unique<CCoreDX11RasterizerState>(this);
}

std::unique_ptr<CCoreSamplerState> CCoreDX11Device::CreateSamplerState() {
  return std::make_unique<CCoreDX11SamplerState>(this);
}

void CCoreDX11Device::SetCurrentDepthStencilState(ID3D11DepthStencilState* bs) {
  CurrentDepthStencilState = bs;
}

ID3D11DepthStencilState* CCoreDX11Device::GetCurrentDepthStencilState() {
  return CurrentDepthStencilState;
}

void CCoreDX11Device::SetCurrentRasterizerState(ID3D11RasterizerState* bs) {
  CurrentRasterizerState = bs;
}

ID3D11RasterizerState* CCoreDX11Device::GetCurrentRasterizerState() {
  return CurrentRasterizerState;
}

void CCoreDX11Device::SetCurrentBlendState(ID3D11BlendState* bs) {
  CurrentBlendState = bs;
}

ID3D11BlendState* CCoreDX11Device::GetCurrentBlendState() {
  return CurrentBlendState;
}

bool CCoreDX11Device::SetRenderTarget(CCoreTexture2D* RT) {
  if (!RT) {
    DeviceContext->OMSetRenderTargets(1, &BackBufferView, DepthBufferView);
    return true;
  }

  return false;
}

void CCoreDX11Device::ForceStateReset() {
  CurrentVertexBuffer = nullptr;
  CurrentRenderState.clear();
  CurrentBlendState = nullptr;
  CurrentDepthStencilState = nullptr;
  CurrentRasterizerState = nullptr;
}

void CCoreDX11Device::TakeScreenShot(std::string_view Filename) {
  ID3D11Texture2D* bb;

  const HRESULT res = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                           reinterpret_cast<LPVOID*>(&bb));
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[core] DirectX11 Swapchain buffer acquisition failed (%s)",
            err.ErrorMessage());
    return;
  }

  auto dummy = std::make_unique<CCoreDX11Texture2D>(this);
  dummy->SetTextureHandle(bb);

  dummy->ExportToImage(Filename, true, EXPORTIMAGEFORMAT::CORE_PNG, false);

  dummy->SetTextureHandle(nullptr);
  dummy->SetView(nullptr);
  dummy.reset();

  bb->Release();

  LOG_NFO("[core] Screenshot %s saved", Filename);
}

#ifdef ENABLE_PIX_API
#define DONT_SAVE_VSGLOG_TO_TEMP
#include "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\include\vsgcapture.h"
#endif

void CCoreDX11Device::InitializeDebugAPI() {
#ifdef ENABLE_PIX_API
  // InitVsPix();
#endif
}

void CCoreDX11Device::CaptureCurrentFrame() {
#ifdef ENABLE_PIX_API
  g_pVsgDbg->CaptureCurrentFrame();
#endif
}

void CCoreDX11Device::BeginOcclusionQuery() {
  if (OcclusionQuery) {
    DeviceContext->Begin(OcclusionQuery);
  }
}

bool CCoreDX11Device::EndOcclusionQuery() {
  if (OcclusionQuery) {
    DeviceContext->End(OcclusionQuery);

    UINT64
    queryData;  // This data type is different depending on the query type
    while (S_OK != DeviceContext->GetData(OcclusionQuery, &queryData,
                                          sizeof(UINT64), 0)) {
    }

    return queryData > 0;
  }

  return 0;
}

ID3D11Texture2D* CCoreDX11Device::GetBackBuffer() {
  ID3D11Texture2D* bb;

  const HRESULT res = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                           reinterpret_cast<LPVOID*>(&bb));
  if (res != S_OK) {
    _com_error err(res);
    LOG_ERR("[core] DirectX11 Swapchain buffer acquisition failed (%s)",
            err.ErrorMessage());
    return nullptr;
  }

  return bb;
}

#else
NoEmptyFile();
#endif
