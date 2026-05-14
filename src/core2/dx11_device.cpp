#include "src/core2/dx11_device.h"

#include <comdef.h>
#include <d3d11.h>
#include <dcomp.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "src/base/logger.h"
#include "src/core2/dx11_constant_buffer.h"
#include "src/core2/dx11_index_buffer.h"
#include "src/core2/dx11_render_state.h"
#include "src/core2/dx11_shader.h"
#include "src/core2/dx11_texture.h"
#include "src/core2/dx11_vertex_buffer.h"
#include "src/core2/dx11_vertex_format.h"
#pragma comment(lib, "DXGI.lib")
// #pragma comment(lib,"dcomp.lib")

import math;

namespace renderer {

using math::Rect;

typedef HRESULT(__stdcall* DCompositionCreateDeviceCallback)(
    _In_opt_ IDXGIDevice* dxgiDevice, _In_ REFIID iid,
    _Outptr_ void** dcompositionDevice);
DCompositionCreateDeviceCallback DCompositionCreateDeviceFunc = nullptr;

DX11Device::DX11Device() = default;

DX11Device::~DX11Device() {
  if (swap_chain_retrace_object_) {
    CloseHandle(swap_chain_retrace_object_);
  }

  if (occlusion_query_) {
    occlusion_query_->Release();
  }

  if (back_buffer_view_) {
    back_buffer_view_->Release();
  }
  if (depth_buffer_view_) {
    depth_buffer_view_->Release();
  }
  if (depth_buffer_) {
    depth_buffer_->Release();
  }
  if (dxgi_swap_chain_) {
    dxgi_swap_chain_->SetFullscreenState(false, nullptr);
    dxgi_swap_chain_->Release();
  }

  if (d3d_device_context_) {
    d3d_device_context_->ClearState();
    d3d_device_context_->Flush();
    d3d_device_context_->Release();
  }

  if (d3d_device_) {
    ID3D11Debug* debug_interface = nullptr;
    d3d_device_->QueryInterface(__uuidof(ID3D11Debug),
                                reinterpret_cast<void**>(&debug_interface));
    if (debug_interface) {
      Log_Nfo("[core] Dumping Live objects before freeing device:");
      debug_interface->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
      debug_interface->Release();
    }
  }
  if (d3d_device_) d3d_device_->Release();
}

void DX11Device::ResetPrivateResources() {}

bool DX11Device::CreateBackBuffer(int32_t x_res, int32_t y_res) {
  if (back_buffer_view_) back_buffer_view_->Release();
  back_buffer_view_ = nullptr;

  HRESULT result = S_OK;
  ID3D11Texture2D* back_buffer = nullptr;

  result = dxgi_swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                       reinterpret_cast<LPVOID*>(&back_buffer));
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectX11 Swapchain buffer acquisition failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  result = d3d_device_->CreateRenderTargetView(back_buffer, nullptr,
                                               &back_buffer_view_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectX11 Rendertarget View creation failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  result = back_buffer->Release();
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectX11 Swapchain buffer texture release failed ({:s})",
            error.ErrorMessage());
  }

  return true;
}

bool DX11Device::CreateDepthBuffer(int32_t x_res, int32_t y_res) {
  if (depth_buffer_view_) {
    depth_buffer_view_->Release();
  }
  if (depth_buffer_) {
    depth_buffer_->Release();
  }

  HRESULT result = S_OK;

  D3D11_TEXTURE2D_DESC depth_buffer_desc;
  D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;

  memset(&depth_buffer_desc, 0, sizeof(depth_buffer_desc));

  depth_buffer_desc.Width = x_res;
  depth_buffer_desc.Height = y_res;
  depth_buffer_desc.MipLevels = 1;
  depth_buffer_desc.ArraySize = 1;
  depth_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_buffer_desc.SampleDesc.Count = 1;
  depth_buffer_desc.SampleDesc.Quality = 0;
  depth_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  depth_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depth_buffer_desc.CPUAccessFlags = 0;
  depth_buffer_desc.MiscFlags = 0;

  result =
      d3d_device_->CreateTexture2D(&depth_buffer_desc, nullptr, &depth_buffer_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectX11 Depth Texture creation failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  ZeroMemory(&depth_stencil_view_desc, sizeof(depth_stencil_view_desc));

  depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depth_stencil_view_desc.Texture2D.MipSlice = 0;

  result = d3d_device_->CreateDepthStencilView(
      depth_buffer_, &depth_stencil_view_desc, &depth_buffer_view_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectX11 Depth Stencil View creation failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  return true;
}

bool DX11Device::CreateClassicSwapChain(const HWND window_handle,
                                        const bool full_screen,
                                        const int32_t x_res,
                                        const int32_t y_res,
                                        const int32_t sample_count,
                                        const int32_t refresh_rate) {
  Log_Nfo("[core] Creating classic swap chain");

  HRESULT result = S_OK;

  DXGI_SWAP_CHAIN_DESC swap_chain_desc;
  memset(&swap_chain_desc, 0, sizeof(DXGI_SWAP_CHAIN_DESC));

  swap_chain_desc.BufferCount = 1;
  swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.OutputWindow = window_handle;
  swap_chain_desc.SampleDesc.Count = 1;
  swap_chain_desc.Windowed = !full_screen;

#ifdef ENABLE_CORE_DEBUG_MODE
  result = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG,
      nullptr, NULL, D3D11_SDK_VERSION, &swap_chain_desc,
      reinterpret_cast<IDXGISwapChain**>(&dxgi_swap_chain_), &d3d_device_,
      nullptr, &d3d_device_context_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Warn(
        "[core] DirectX11 debug mode device creation failed. ({:s}) Trying "
        "without debug mode...",
        error.ErrorMessage());
#endif
    result = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, NULL,
        D3D11_SDK_VERSION, &swap_chain_desc,
        reinterpret_cast<IDXGISwapChain**>(&dxgi_swap_chain_), &d3d_device_,
        nullptr, &d3d_device_context_);
    if (result != S_OK) {
      _com_error error(result);
      Log_Err("[core] DirectX11 Device creation failed ({:s})",
              error.ErrorMessage());
      return false;
    }
#ifdef ENABLE_CORE_DEBUG_MODE
  }
#endif

  if (!CreateBackBuffer(x_res, y_res)) {
    return false;
  }
  if (!CreateDepthBuffer(x_res, y_res)) {
    return false;
  }
  d3d_device_context_->OMSetRenderTargets(1, &back_buffer_view_,
                                          depth_buffer_view_);

  SetViewport(Rect(0, 0, x_res, y_res));

  if (CreateDefaultRenderStates()) {
    Log_Nfo("[core] DirectX11 Device initialization successful.");
  }

  D3D11_QUERY_DESC query_desc;
  memset(&query_desc, 0, sizeof(query_desc));
  query_desc.Query = D3D11_QUERY_OCCLUSION;
  query_desc.MiscFlags = 0;

  d3d_device_->CreateQuery(&query_desc, &occlusion_query_);

  return true;
}

bool DX11Device::CreateDirectCompositionSwapchain(const HWND window_handle,
                                                  const bool full_screen,
                                                  const int32_t x_res,
                                                  const int32_t y_res,
                                                  const int32_t sample_count,
                                                  const int32_t refresh_rate) {
  Log_Nfo("[core] Creating DirectComposition swap chain");

  HRESULT result = S_OK;

  IDXGIFactory2* dxgi_factory = nullptr;
#ifdef _DEBUG
  result =
      CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, __uuidof(IDXGIFactory2),
                         reinterpret_cast<void**>(&dxgi_factory));
#else
  result = CreateDXGIFactory1(__uuidof(IDXGIFactory2),
                              reinterpret_cast<void**>(&dxgi_factory));
#endif
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DXGI factory creation failed ({:s})", error.ErrorMessage());
    return false;
  }

#ifdef ENABLE_CORE_DEBUG_MODE
  result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                             D3D11_CREATE_DEVICE_DEBUG, nullptr, NULL,
                             D3D11_SDK_VERSION, &d3d_device_, nullptr,
                             &d3d_device_context_);
  if (result != S_OK) {
    _com_error error(result);
    Log_Warn(
        "[core] DirectX11 debug mode device creation failed. ({:s}) Trying "
        "without debug mode...",
        error.ErrorMessage());
#endif
    result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                               nullptr, 0, D3D11_SDK_VERSION, &d3d_device_,
                               nullptr, &d3d_device_context_);
    if (result != S_OK) {
      _com_error error(result);
      Log_Err("[core] DirectX11 Device creation failed ({:s})",
              error.ErrorMessage());
      return false;
    }
#ifdef ENABLE_CORE_DEBUG_MODE
  }
#endif

  constexpr unsigned int backBufferCount = 2;
  DXGI_SWAP_CHAIN_DESC1 swap_chain_desc{static_cast<UINT>(x_res),
                                        static_cast<UINT>(y_res),
                                        DXGI_FORMAT_R8G8B8A8_UNORM,
                                        false,
                                        {1, 0},
                                        DXGI_USAGE_RENDER_TARGET_OUTPUT,
                                        backBufferCount,
                                        DXGI_SCALING_STRETCH,
                                        DXGI_SWAP_EFFECT_FLIP_DISCARD,
                                        DXGI_ALPHA_MODE_PREMULTIPLIED,
                                        0};
  swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT |
                          DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

  result = dxgi_factory->CreateSwapChainForComposition(
      d3d_device_, &swap_chain_desc, nullptr, &dxgi_swap_chain_);
  if (result != S_OK) {
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swap_chain_desc.Flags = 0;
    result = dxgi_factory->CreateSwapChainForComposition(
        d3d_device_, &swap_chain_desc, nullptr, &dxgi_swap_chain_);

    if (result != S_OK) {
      swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
      result = dxgi_factory->CreateSwapChainForComposition(
          d3d_device_, &swap_chain_desc, nullptr, &dxgi_swap_chain_);

      if (result != S_OK) {
        _com_error error(result);
        Log_Err("[core] DirectX11 SwapChain creation failed (%s)",
                error.ErrorMessage());
        return false;
      }
    }
  }

  IDCompositionDevice* dcomp_device = nullptr;

  result = DCompositionCreateDeviceFunc(
      reinterpret_cast<IDXGIDevice*>(d3d_device_),
      __uuidof(IDCompositionDevice), reinterpret_cast<void**>(&dcomp_device));

  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectComposition device creation failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  IDCompositionTarget* dcomp_target = nullptr;
  result =
      dcomp_device->CreateTargetForHwnd((window_handle), true, &dcomp_target);

  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectComposition target creation failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  IDCompositionVisual* dcomp_visual = nullptr;
  result = dcomp_device->CreateVisual(&dcomp_visual);

  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectComposition visual creation failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  result = dcomp_visual->SetContent(dxgi_swap_chain_);

  if (result != S_OK) {
    _com_error error(result);
    Log_Err(
        "[core] DirectComposition visual swapchain content setting failed "
        "({:s})",
        error.ErrorMessage());
    return false;
  }

  result = dcomp_target->SetRoot(dcomp_visual);

  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectComposition setting target root visual failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  result = dcomp_device->Commit();

  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectComposition commit failed ({:s})",
            error.ErrorMessage());
    return false;
  }

  IDXGISwapChain2* swap_chain2 = nullptr;
  if (SUCCEEDED(dxgi_swap_chain_->QueryInterface(
          __uuidof(IDXGISwapChain2), reinterpret_cast<void**>(&swap_chain2)))) {
    swap_chain_retrace_object_ = swap_chain2->GetFrameLatencyWaitableObject();
    swap_chain2->Release();
  }

  dxgi_factory->Release();

  if (!CreateBackBuffer(x_res, y_res)) {
    return false;
  }
  if (!CreateDepthBuffer(x_res, y_res)) {
    return false;
  }
  d3d_device_context_->OMSetRenderTargets(1, &back_buffer_view_,
                                          depth_buffer_view_);

  SetViewport(Rect(0, 0, x_res, y_res));

  if (CreateDefaultRenderStates()) {
    Log_Nfo("[core] DirectX11 Device initialization successful.");
  }

  D3D11_QUERY_DESC query_desc;
  memset(&query_desc, 0, sizeof(query_desc));
  query_desc.Query = D3D11_QUERY_OCCLUSION;
  query_desc.MiscFlags = 0;

  d3d_device_->CreateQuery(&query_desc, &occlusion_query_);

  return true;
}

bool DX11Device::InitAPI(const HWND window_handle, const bool full_screen,
                         const int32_t x_res, const int32_t y_res,
                         const int32_t sample_count /* =0 */,
                         const int32_t refresh_rate /* =60 */) {
  auto dcomp = LoadLibrary("dcomp.dll");

  if (dcomp) {
    DCompositionCreateDeviceFunc =
        reinterpret_cast<DCompositionCreateDeviceCallback>(
            GetProcAddress(dcomp, "DCompositionCreateDevice"));
  }

  if (!dcomp || !DCompositionCreateDeviceFunc) {
    return CreateClassicSwapChain(window_handle, full_screen, x_res, y_res,
                                  sample_count, refresh_rate);
  } else {
    return CreateDirectCompositionSwapchain(window_handle, full_screen, x_res,
                                            y_res, sample_count, refresh_rate);
  }

  if (dcomp) {
    FreeLibrary(dcomp);
  }
}

bool DX11Device::Initialize(WindowHandler* window, const int32_t sample_count) {
  window_ = window;

  if (!InitAPI(window_->GetHandle(), window_->GetInitParameters()->full_screen_,
               window_->GetXRes(), window_->GetYRes(), sample_count, 60)) {
    return false;
  }

  ::ShowWindow(window_->GetHandle(), window_->GetInitParameters()->maximized_
                                         ? SW_SHOWMAXIMIZED
                                         : SW_SHOWNORMAL);
  SetForegroundWindow(window_->GetHandle());
  SetFocus(window_->GetHandle());
  return true;
}

bool DX11Device::IsWindowed() {
  BOOL full_screen = false;
  IDXGIOutput* output = nullptr;

  if (dxgi_swap_chain_->GetFullscreenState(&full_screen, &output) != S_OK) {
    Log_Err("[core] Failed to get fullscreen state");
    return false;
  }

  if (output) {
    output->Release();
  }
  return full_screen;
}

void DX11Device::Resize(const int32_t x_res, const int32_t y_res) {
  if (x_res <= 0 || y_res <= 0) {
    Log_Warn(
        "[core] Trying to resize swapchain to invalid resolution: {:d} {:d}",
        x_res, y_res);
    return;
  }

  DXGI_SWAP_CHAIN_DESC swap_chain_desc;
  HRESULT result = dxgi_swap_chain_->GetDesc(&swap_chain_desc);
  if (result != S_OK) {
    Log_Err("[core] Failed to get swapchain description");
    return;
  }

  if (swap_chain_desc.BufferDesc.Width == x_res &&
      swap_chain_desc.BufferDesc.Height == y_res) {
    return;
  }

  if (back_buffer_view_) {
    back_buffer_view_->Release();
  }
  if (depth_buffer_view_) {
    depth_buffer_view_->Release();
  }
  if (depth_buffer_) {
    depth_buffer_->Release();
  }
  back_buffer_view_ = nullptr;
  depth_buffer_view_ = nullptr;
  depth_buffer_ = nullptr;

  result = dxgi_swap_chain_->ResizeBuffers(swap_chain_desc.BufferCount, x_res,
                                           y_res, DXGI_FORMAT_UNKNOWN,
                                           swap_chain_desc.Flags);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err(
        "[core] Failed to resize swapchain to {:d} {:d} "
        "(bufferCount: {:d}, flags: {:d}) ({:s})",
        x_res, y_res, swap_chain_desc.BufferCount, swap_chain_desc.Flags,
        error.ErrorMessage());
    return;
  }

  if (!CreateBackBuffer(x_res, y_res)) {
    return;
  }
  if (!CreateDepthBuffer(x_res, y_res)) {
    return;
  }

  d3d_device_context_->OMSetRenderTargets(1, &back_buffer_view_,
                                          depth_buffer_view_);
  SetViewport(Rect(0, 0, x_res, y_res));

  if (swap_chain_retrace_object_) {
    CloseHandle(swap_chain_retrace_object_);

    IDXGISwapChain2* swap_chain2 = nullptr;
    if (SUCCEEDED(dxgi_swap_chain_->QueryInterface(
            __uuidof(IDXGISwapChain2),
            reinterpret_cast<void**>(&swap_chain2)))) {
      swap_chain_retrace_object_ = swap_chain2->GetFrameLatencyWaitableObject();
      swap_chain2->Release();
    }
  }
}

void DX11Device::SetFullScreenMode(const bool full_screen, const int32_t x_res,
                                   const int32_t y_res) {
  Log_Nfo("[core] Switching fullscreen mode to {:d}", full_screen);

  const HRESULT result =
      dxgi_swap_chain_->SetFullscreenState(full_screen, nullptr);
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] Failed to set FullScreen mode to {:d}. ({:s})", full_screen,
            error.ErrorMessage());
    return;
  }
}

bool DX11Device::DeviceOk() { return true; }

//////////////////////////////////////////////////////////////////////////
// texture functions

std::unique_ptr<Texture2D> DX11Device::CreateTexture2D(
    const int32_t x_res, const int32_t y_res, const uint8_t* data,
    const char bytes_per_pixel, const Format format, const bool render_target) {
  auto result = std::make_unique<DX11Texture2D>(this);
  if (!result->Create(x_res, y_res, data, bytes_per_pixel, format,
                      render_target)) {
    result.reset();
  }
  return result;
}

std::unique_ptr<Texture2D> DX11Device::CreateTexture2D(const uint8_t* data,
                                                       const int32_t size) {
  auto result = std::make_unique<DX11Texture2D>(this);
  if (!result->Create(data, size)) {
    result.reset();
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////
// vertexbuffer functions

std::unique_ptr<VertexBuffer> DX11Device::CreateVertexBuffer(
    const uint8_t* data, const int32_t size) {
  auto result = std::make_unique<DX11VertexBuffer>(this);
  if (!result->Create(data, size)) {
    result.reset();
  }
  return result;
}

std::unique_ptr<VertexBuffer> DX11Device::CreateVertexBufferDynamic(
    const int32_t size) {
  auto result = std::make_unique<DX11VertexBuffer>(this);
  if (!result->CreateDynamic(size)) {
    result.reset();
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////
// indexbuffer functions

std::unique_ptr<IndexBuffer> DX11Device::CreateIndexBuffer(
    const int32_t index_count, const int32_t index_size) {
  auto result = std::make_unique<DX11IndexBuffer>(this);
  if (!result->Create(index_count, index_size)) {
    result.reset();
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////
// vertexformat functions

std::unique_ptr<VertexFormat> DX11Device::CreateVertexFormat(
    const std::vector<VertexAttribute>& attributes,
    VertexShader* vertex_shader) {
  auto result = std::make_unique<DX11VertexFormat>(this);
  if (!result->Create(attributes, vertex_shader)) {
    result.reset();
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////
// shader functions

std::unique_ptr<VertexShader> DX11Device::CreateVertexShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) *error = "";
  if (!code || !code_size || !entry_function || !shader_version) {
    return {};
  }

  auto shader = std::make_unique<DX11VertexShader>(this);
  shader->SetCode(code, entry_function, shader_version);

  if (!shader->CompileAndCreate(error)) {
    shader.reset();
  }
  return shader;
}

std::unique_ptr<PixelShader> DX11Device::CreatePixelShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) {
    *error = "";
  }
  if (!code || !code_size || !entry_function || !shader_version) {
    return {};
  }

  auto shader = std::make_unique<DX11PixelShader>(this);
  shader->SetCode(code, entry_function, shader_version);

  if (!shader->CompileAndCreate(error)) {
    shader.reset();
  }
  return shader;
}

std::unique_ptr<VertexShader> DX11Device::CreateVertexShaderFromBlob(
    uint8_t* code, int32_t code_size) {
  auto shader = std::make_unique<DX11VertexShader>(this);
  if (!shader->CreateFromBlob(code, code_size)) {
    shader.reset();
  }
  return shader;
}

std::unique_ptr<PixelShader> DX11Device::CreatePixelShaderFromBlob(
    uint8_t* code, int32_t code_size) {
  auto shader = std::make_unique<DX11PixelShader>(this);
  if (!shader->CreateFromBlob(code, code_size)) {
    shader.reset();
  }
  return shader;
}

std::unique_ptr<GeometryShader> DX11Device::CreateGeometryShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) {
    *error = "";
  }
  if (!code || !code_size || !entry_function || !shader_version) {
    return nullptr;
  }

  auto shader = std::make_unique<DX11GeometryShader>(this);
  shader->SetCode(code, entry_function, shader_version);

  if (!shader->CompileAndCreate(error)) {
    return nullptr;
  }
  return shader;
}

std::unique_ptr<DomainShader> DX11Device::CreateDomainShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) {
    *error = "";
  }
  if (!code || !code_size || !entry_function || !shader_version) {
    return nullptr;
  }

  auto shader = std::make_unique<DX11DomainShader>(this);
  shader->SetCode(code, entry_function, shader_version);

  if (!shader->CompileAndCreate(error)) {
    return nullptr;
  }
  return shader;
}

std::unique_ptr<HullShader> DX11Device::CreateHullShader(LPCSTR code,
                                                         int32_t code_size,
                                                         LPCSTR entry_function,
                                                         LPCSTR shader_version,
                                                         std::string* error) {
  if (error) {
    *error = "";
  }
  if (!code || !code_size || !entry_function || !shader_version) {
    return nullptr;
  }

  auto shader = std::make_unique<DX11HullShader>(this);
  shader->SetCode(code, entry_function, shader_version);

  if (!shader->CompileAndCreate(error)) {
    return nullptr;
  }
  return shader;
}

std::unique_ptr<ComputeShader> DX11Device::CreateComputeShader(
    LPCSTR code, int32_t code_size, LPCSTR entry_function,
    LPCSTR shader_version, std::string* error) {
  if (error) {
    *error = "";
  }
  if (!code || !code_size || !entry_function || !shader_version) {
    return nullptr;
  }

  auto shader = std::make_unique<DX11ComputeShader>(this);
  shader->SetCode(code, entry_function, shader_version);

  if (!shader->CompileAndCreate(error)) {
    return nullptr;
  }
  return shader;
}

std::unique_ptr<VertexShader> DX11Device::CreateVertexShader() {
  return std::make_unique<DX11VertexShader>(this);
}

std::unique_ptr<PixelShader> DX11Device::CreatePixelShader() {
  return std::make_unique<DX11PixelShader>(this);
}

std::unique_ptr<GeometryShader> DX11Device::CreateGeometryShader() {
  return std::make_unique<DX11GeometryShader>(this);
}

std::unique_ptr<HullShader> DX11Device::CreateHullShader() {
  return std::make_unique<DX11HullShader>(this);
}

std::unique_ptr<DomainShader> DX11Device::CreateDomainShader() {
  return std::make_unique<DX11DomainShader>(this);
}

std::unique_ptr<ComputeShader> DX11Device::CreateComputeShader() {
  return std::make_unique<DX11ComputeShader>(this);
}

//////////////////////////////////////////////////////////////////////////
// renderstate

bool DX11Device::ApplyRenderState(const Sampler sampler,
                                  const RenderState render_state,
                                  const RenderStateValue value) {
  switch (render_state) {
    case RenderState::kBlendState: {
      if (!value.blend_state) {
        d3d_device_context_->OMSetBlendState(nullptr, nullptr, 0xffffffff);
        current_blend_state_ = nullptr;
        return true;
      }
      return value.blend_state->Apply();
    } break;
    case RenderState::kRasterizerState: {
      if (!value.rasterizer_state) {
        d3d_device_context_->RSSetState(nullptr);
        current_rasterizer_state_ = nullptr;
        return true;
      }
      return value.rasterizer_state->Apply();
    } break;
    case RenderState::kDepthStencilState: {
      if (!value.depth_stencil_state) {
        d3d_device_context_->OMSetDepthStencilState(nullptr, 0);
        current_depth_stencil_state_ = nullptr;
        return true;
      }
      return value.depth_stencil_state->Apply();
    } break;
    case RenderState::kSamplerState: {
      if (!value.sampler_state) {
        return false;
      }
      return value.sampler_state->Apply(sampler);
    } break;
    case RenderState::kTexture: {
      if (!value.texture) {
        ID3D11ShaderResourceView* null_srv[1];
        null_srv[0] = nullptr;

        if (sampler >= Sampler::kPs0 && sampler <= Sampler::kPs15) {
          d3d_device_context_->PSSetShaderResources(sampler - Sampler::kPs0, 1,
                                                    null_srv);
        }
        if (sampler >= Sampler::kVs0 && sampler <= Sampler::kVs3) {
          d3d_device_context_->VSSetShaderResources(sampler - Sampler::kVs0, 1,
                                                    null_srv);
        }
        if (sampler >= Sampler::kGs0 && sampler <= Sampler::kGs3) {
          d3d_device_context_->GSSetShaderResources(sampler - Sampler::kGs0, 1,
                                                    null_srv);
        }
        return true;
      }
      return ApplyTextureToSampler(sampler, value.texture);
    } break;
    case RenderState::kVertexFormat: {
      if (!value.vertex_format) {
        current_vertex_format_size_ = 0;
        d3d_device_context_->IASetInputLayout(nullptr);
        return true;
      }

      current_vertex_format_size_ = value.vertex_format->GetSize();
      return ApplyVertexFormat(value.vertex_format);
    } break;
    case RenderState::kIndexBuffer: {
      if (!value.index_buffer) {
        d3d_device_context_->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
        return true;
      }
      return ApplyIndexBuffer(value.index_buffer);
    } break;
    case RenderState::kVertexShader: {
      if (!value.vertex_shader) {
        d3d_device_context_->VSSetShader(nullptr, nullptr, 0);
        return true;
      }
      return ApplyVertexShader(value.vertex_shader);
    } break;
    case RenderState::kGeometryShader: {
      if (!value.geometry_shader) {
        d3d_device_context_->GSSetShader(nullptr, nullptr, 0);
        return true;
      }
      return ApplyGeometryShader(value.geometry_shader);
    } break;
    case RenderState::kHullShader: {
      if (!value.hull_shader) {
        d3d_device_context_->HSSetShader(nullptr, nullptr, 0);
        return true;
      }
      return ApplyHullShader(value.hull_shader);
    } break;
    case RenderState::kDomainShader: {
      if (!value.domain_shader) {
        d3d_device_context_->DSSetShader(nullptr, nullptr, 0);
        return true;
      }
      return ApplyDomainShader(value.domain_shader);
    } break;
    case RenderState::kComputeShader: {
      if (!value.compute_shader) {
        d3d_device_context_->DSSetShader(nullptr, nullptr, 0);
        return true;
      }
      return ApplyComputeShader(value.compute_shader);
    } break;
    case RenderState::kPixelShader: {
      if (!value.pixel_shader) {
        d3d_device_context_->PSSetShader(nullptr, nullptr, 0);
        return true;
      }
      return ApplyPixelShader(value.pixel_shader);
    } break;
    default:
      return true;
  }
}

bool DX11Device::SetNoVertexBuffer() {
  d3d_device_context_->IASetVertexBuffers(0, 1, nullptr, nullptr, nullptr);
  return true;
}

bool DX11Device::CommitRenderStates() { return true; }

//////////////////////////////////////////////////////////////////////////
// display functions

bool DX11Device::BeginScene() { return true; }

bool DX11Device::EndScene() { return true; }

bool DX11Device::Clear(const bool clear_pixels, const bool clear_depth,
                       const CColor& color, const float depth,
                       const int32_t stencil) {
  const float clear_color[4] = {color.R() / 255.0f, color.G() / 255.0f,
                                color.B() / 255.0f, color.A() / 255.0f};

  if (clear_pixels) {
    d3d_device_context_->ClearRenderTargetView(back_buffer_view_, clear_color);
  }

  if (clear_depth) {
    d3d_device_context_->ClearDepthStencilView(
        depth_buffer_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth,
        stencil);
  }

  return true;
}

bool DX11Device::Flip(bool vsync) {
  HRESULT result = 0;

  if (vsync) {
    result = dxgi_swap_chain_->Present(1, 0);
  } else {
    result = dxgi_swap_chain_->Present(0, 0);
  }

  return result == S_OK;
}

bool DX11Device::DrawIndexedTriangles(int32_t count, int32_t vertex_count) {
  d3d_device_context_->IASetPrimitiveTopology(
      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  if (!ApplyRequestedRenderState()) {
    return false;
  }
  d3d_device_context_->DrawIndexed(count * 3, 0, 0);
  return true;
}

bool DX11Device::DrawLines(int32_t count) {
  d3d_device_context_->IASetPrimitiveTopology(
      D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
  if (!ApplyRequestedRenderState()) {
    return false;
  }
  d3d_device_context_->Draw(count * 2, 0);
  return true;
}

bool DX11Device::DrawIndexedLines(int32_t count, int32_t vertex_count) {
  d3d_device_context_->IASetPrimitiveTopology(
      D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
  if (!ApplyRequestedRenderState()) {
    return false;
  }
  d3d_device_context_->DrawIndexed(count * 2, 0, 0);
  return true;
}

bool DX11Device::DrawTriangles(int32_t count) {
  d3d_device_context_->IASetPrimitiveTopology(
      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  if (!ApplyRequestedRenderState()) {
    return false;
  }
  d3d_device_context_->Draw(count * 3, 0);
  return true;
}

bool DX11Device::SetViewport(const math::Rect& viewport) {
  D3D11_VIEWPORT d3d_viewport;
  memset(&d3d_viewport, 0, sizeof(D3D11_VIEWPORT));

  d3d_viewport.TopLeftX = static_cast<float>(viewport.x1);
  d3d_viewport.TopLeftY = static_cast<float>(viewport.y1);
  d3d_viewport.Width = std::max(0.f, static_cast<float>(viewport.Width()));
  d3d_viewport.Height = std::max(0.f, static_cast<float>(viewport.Height()));
  d3d_viewport.MinDepth = 0;
  d3d_viewport.MaxDepth = 1;
  d3d_device_context_->RSSetViewports(1, &d3d_viewport);

  return true;
}

void DX11Device::SetShaderConstants(const ConstantBuffer* buffers) {
  void* buffers_table[16];

  if (buffers) {
    buffers_table[0] = buffers->GetBufferPointer();
  } else {
    memset(buffers_table, 0, 16 * sizeof(void*));
  }

  d3d_device_context_->VSSetConstantBuffers(
      0, 1, reinterpret_cast<ID3D11Buffer**>(buffers_table));
  d3d_device_context_->GSSetConstantBuffers(
      0, 1, reinterpret_cast<ID3D11Buffer**>(buffers_table));
  d3d_device_context_->PSSetConstantBuffers(
      0, 1, reinterpret_cast<ID3D11Buffer**>(buffers_table));
}

std::unique_ptr<ConstantBuffer> DX11Device::CreateConstantBuffer() {
  return std::make_unique<DX11ConstantBuffer>(this);
}

std::unique_ptr<BlendState> DX11Device::CreateBlendState() {
  return std::make_unique<DX11BlendState>(this);
}

std::unique_ptr<DepthStencilState> DX11Device::CreateDepthStencilState() {
  return std::make_unique<DX11DepthStencilState>(this);
}

std::unique_ptr<RasterizerState> DX11Device::CreateRasterizerState() {
  return std::make_unique<DX11RasterizerState>(this);
}

std::unique_ptr<SamplerState> DX11Device::CreateSamplerState() {
  return std::make_unique<DX11SamplerState>(this);
}

void DX11Device::SetCurrentDepthStencilState(
    ID3D11DepthStencilState* depth_stencil_state) {
  current_depth_stencil_state_ = depth_stencil_state;
}

ID3D11DepthStencilState* DX11Device::GetCurrentDepthStencilState() {
  return current_depth_stencil_state_;
}

void DX11Device::SetCurrentRasterizerState(
    ID3D11RasterizerState* rasterizer_state) {
  current_rasterizer_state_ = rasterizer_state;
}

ID3D11RasterizerState* DX11Device::GetCurrentRasterizerState() {
  return current_rasterizer_state_;
}

void DX11Device::SetCurrentBlendState(ID3D11BlendState* blend_state) {
  current_blend_state_ = blend_state;
}

ID3D11BlendState* DX11Device::GetCurrentBlendState() {
  return current_blend_state_;
}

bool DX11Device::SetRenderTarget(Texture2D* render_target) {
  if (!render_target) {
    d3d_device_context_->OMSetRenderTargets(1, &back_buffer_view_,
                                            depth_buffer_view_);
    return true;
  }

  return false;
}

void DX11Device::ForceStateReset() {
  current_vertex_buffer_ = nullptr;
  current_render_state_.clear();
  current_blend_state_ = nullptr;
  current_depth_stencil_state_ = nullptr;
  current_rasterizer_state_ = nullptr;
}

void DX11Device::TakeScreenShot(std::string_view filename) {
  ID3D11Texture2D* back_buffer = nullptr;

  const HRESULT result = dxgi_swap_chain_->GetBuffer(
      0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&back_buffer));
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectX11 Swapchain buffer acquisition failed ({:s})",
            error.ErrorMessage());
    return;
  }

  auto dummy = std::make_unique<DX11Texture2D>(this);
  dummy->SetTextureHandle(back_buffer);

  dummy->ExportToImage(filename, true, ExportImageFormat::kPng, false);

  dummy->SetTextureHandle(nullptr);
  dummy->SetView(nullptr);
  dummy.reset();

  back_buffer->Release();

  Log_Nfo("[core] Screenshot {:s} saved", filename);
}

#ifdef ENABLE_PIX_API
#define DONT_SAVE_VSGLOG_TO_TEMP
#include "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\include\vsgcapture.h"
#endif

void DX11Device::InitializeDebugAPI() {
#ifdef ENABLE_PIX_API
  // InitVsPix();
#endif
}

void DX11Device::CaptureCurrentFrame() {
#ifdef ENABLE_PIX_API
  g_pVsgDbg->CaptureCurrentFrame();
#endif
}

void DX11Device::BeginOcclusionQuery() {
  if (occlusion_query_) {
    d3d_device_context_->Begin(occlusion_query_);
  }
}

bool DX11Device::EndOcclusionQuery() {
  if (occlusion_query_) {
    d3d_device_context_->End(occlusion_query_);

    UINT64 query_data = 0;
    while (S_OK != d3d_device_context_->GetData(occlusion_query_, &query_data,
                                                sizeof(UINT64), 0)) {
    }

    return query_data > 0;
  }

  return false;
}

void DX11Device::WaitRetrace() {
  if (swap_chain_retrace_object_) {
    WaitForSingleObjectEx(swap_chain_retrace_object_, 1000, true);
  }
}

ID3D11Texture2D* DX11Device::GetBackBuffer() {
  ID3D11Texture2D* back_buffer = nullptr;

  const HRESULT result = dxgi_swap_chain_->GetBuffer(
      0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&back_buffer));
  if (result != S_OK) {
    _com_error error(result);
    Log_Err("[core] DirectX11 Swapchain buffer acquisition failed ({:s})",
            error.ErrorMessage());
    return nullptr;
  }

  return back_buffer;
}

}  // namespace renderer
