#include "src/core2/dx11_texture.h"

#include <comdef.h>

#include <algorithm>

#include "src/base/assert.h"
#include "src/base/image_decompressor.h"
#include "src/base/logger.h"
#include "src/core2/dss_texture_loader.h"
#include "src/util/png_decompressor.h"

CCoreDX11Texture2D::CCoreDX11Texture2D(CCoreDX11Device* dev)
    : CCoreTexture2D(dev) {
  Dev = dev->GetDevice();
  DeviceContext = dev->GetDeviceContext();
  TextureHandle = nullptr;
  RenderTarget = false;
  View = nullptr;
  RTView = nullptr;
  DepthView = nullptr;
};

CCoreDX11Texture2D::~CCoreDX11Texture2D() { Release(); }

void CCoreDX11Texture2D::Release() {
  if (TextureHandle) TextureHandle->Release();
  TextureHandle = nullptr;
  if (View) View->Release();
  if (DepthView) DepthView->Release();
  if (RTView) RTView->Release();
  View = nullptr;
  RTView = nullptr;
  DepthView = nullptr;
}

bool CCoreDX11Texture2D::SetToSampler(const CORESAMPLER smp) {
  if (smp >= CORESAMPLER::PS0 && smp <= CORESAMPLER::PS15) {
    DeviceContext->PSSetShaderResources(smp - CORESAMPLER::PS0, 1, &View);
  }
  if (smp >= CORESAMPLER::VS0 && smp <= CORESAMPLER::VS3) {
    DeviceContext->VSSetShaderResources(smp - CORESAMPLER::VS0, 1, &View);
  }
  if (smp >= CORESAMPLER::GS0 && smp <= CORESAMPLER::GS3) {
    DeviceContext->GSSetShaderResources(smp - CORESAMPLER::GS0, 1, &View);
  }

  return true;
}

bool CCoreDX11Texture2D::Create(const int32_t xres, const int32_t yres,
                                const uint8_t* Data, const char BytesPerPixel,
                                const COREFORMAT format,
                                const bool rendertarget) {
  if (xres <= 0 || yres <= 0 || format == COREFORMAT::UNKNOWN) return false;
  Release();

  D3D11_TEXTURE2D_DESC tex;
  memset(&tex, 0, sizeof(D3D11_TEXTURE2D_DESC));
  tex.ArraySize = 1;
  tex.Width = xres;
  tex.Height = yres;
  // tex.MipLevels = 1;// rendertarget ? 0 : 1;
  tex.MipLevels = rendertarget ? 0 : 1;
  tex.MiscFlags = rendertarget ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
  tex.Format = DX11FormatsAt(format);
  tex.SampleDesc.Count = 1;
  tex.SampleDesc.Quality = 0;
  tex.BindFlags = D3D11_BIND_SHADER_RESOURCE |
                  (rendertarget ? D3D11_BIND_RENDER_TARGET : 0);
  // if ( rendertarget )
  //  tex.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

  D3D11_SUBRESOURCE_DATA data;
  data.pSysMem = Data;
  data.SysMemPitch = xres * BytesPerPixel;
  data.SysMemSlicePitch = 0;

  HRESULT res =
      Dev->CreateTexture2D(&tex, Data ? &data : nullptr, &TextureHandle);
  if (res != S_OK) {
    _com_error err(res);
    Log_Err("[core] CreateTexture2D failed ({:s})", err.ErrorMessage());
    return false;
  }

  res = Dev->CreateShaderResourceView(TextureHandle, nullptr, &View);
  if (res != S_OK) {
    _com_error err(res);
    Log_Err("[core] CreateShaderResourceView failed ({:s})",
            err.ErrorMessage());
    return false;
  }

  if (rendertarget) {
    D3D11_RENDER_TARGET_VIEW_DESC rt;
    rt.Format = DX11FormatsAt(Format);
    rt.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
    rt.Texture2D.MipSlice = 0;
    res = Dev->CreateRenderTargetView(TextureHandle, &rt, &RTView);
    if (res != S_OK) {
      _com_error err(res);
      Log_Err("[core] Failed to rendertarget view ({:s})", err.ErrorMessage());
      return false;
    }
  }

  XRes = xres;
  YRes = yres;
  Format = format;

  return true;
}

bool CCoreDX11Texture2D::Create(const uint8_t* Data, const int32_t Size) {
  bool ViewCreated = false;

  if (!Data || Size <= 0) return false;
  Release();

  int32_t xr = 0, yr = 0;
  auto Img = DecompressImage(Data, Size, xr, yr);

  if (!Img) {
    if (!DecompressPNG(Data, Size, Img, xr, yr)) {
      if (!CreateDDSTextureFromMemory(
              Dev, Data, Size,
              reinterpret_cast<ID3D11Resource**>(&TextureHandle), &View)) {
        return false;
      } else {
        ViewCreated = true;
      }
    } else {
      if (!Img) return false;
      ARGBtoABGR(Img.get(), xr, yr);
    }
  }

  if (!ViewCreated) {
    Create(xr, yr, Img.get());
    Img.reset();
  }

  if (!ViewCreated) {
    const HRESULT res =
        Dev->CreateShaderResourceView(TextureHandle, nullptr, &View);
    if (res != S_OK) {
      _com_error err(res);
      Log_Err("[core] CreateShaderResourceView failed ({:s})",
              err.ErrorMessage());
      return false;
    }
  }

  D3D11_TEXTURE2D_DESC texturedesc;
  TextureHandle->GetDesc(&texturedesc);

  XRes = texturedesc.Width;
  YRes = texturedesc.Height;
  Format = GetFormat(texturedesc.Format);
  RenderTarget = false;

  return true;
}

bool CCoreDX11Texture2D::Lock(void** Result, int32_t& pitch) { return false; }

bool CCoreDX11Texture2D::UnLock() { return true; }

void CCoreDX11Texture2D::OnDeviceLost() {
  if (RenderTarget) Release();
}

void CCoreDX11Texture2D::OnDeviceReset() {
  if (RenderTarget && XRes > 0 && YRes > 0 && Format != COREFORMAT::UNKNOWN) {
    BASEASSERT(Create(XRes, YRes, nullptr, 4, Format, RenderTarget));
  }
}

bool CCoreDX11Texture2D::Update(const uint8_t* Data, const int32_t XRes,
                                const int32_t YRes, const char BytesPerPixel) {
  if (!TextureHandle) return false;
  if (!View) return false;

  DeviceContext->UpdateSubresource(TextureHandle, 0, nullptr, Data,
                                   XRes * BytesPerPixel, 0);

  return true;
}

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "src/util/png_decompressor.h"

float degammafloat(float f) {
  if (f < 0.0031308f) return 12.92f * f;
  return 1.055f * powf(f, 1 / 2.4f) - 0.055f;
}

uint16_t degammaint16(uint16_t f) {
  const float tf = f / 65535.0f;

  return static_cast<uint16_t>(degammafloat(tf) * 65535);
}

void CCoreDX11Texture2D::ExportToImage(std::string_view Filename,
                                       bool ClearAlpha,
                                       EXPORTIMAGEFORMAT Format, bool degamma) {
  if (!TextureHandle) return;

  CStreamWriterMemory Writer;
  const HRESULT res = SaveDDSTexture(DeviceContext, TextureHandle, Writer);
  if (res != S_OK) {
    _com_error err(res);
    Log_Err("[core] Failed to export texture to '{:s}' ({:x}: {:s})", Filename,
            res, err.ErrorMessage());
    return;
  }

  struct DDSHEAD {
    uint32_t DDS;
    int32_t dwSize;
    int32_t dwFlags;
    int32_t dwHeight;
    int32_t dwWidth;
    int32_t dwPitchOrLinearSize;
    int32_t dwDepth;
    int32_t dwMipMapCount;
    int32_t dwReserved1[11];

    int32_t _dwSize;
    int32_t _dwFlags;
    int32_t dwFourCC;
    int32_t dwRGBBitCount;
    int32_t dwRBitMask;
    int32_t dwGBitMask;
    int32_t dwBBitMask;
    int32_t dwABitMask;

    int32_t dwCaps;
    int32_t dwCaps2;
    int32_t dwCaps3;
    int32_t dwCaps4;
    int32_t dwReserved2;
  };

  struct DDS_HEADER_DXT10 {
    DXGI_FORMAT dxgiFormat;
    D3D10_RESOURCE_DIMENSION resourceDimension;
    UINT miscFlag;
    UINT arraySize;
    UINT miscFlags2;
  };

  uint8_t* Data = Writer.GetData();
  DDSHEAD head{};
  memcpy(&head, Data, sizeof(DDSHEAD));
  Data += head.dwSize + 4;

  auto image = std::make_unique<uint8_t[]>(head.dwWidth * head.dwHeight * 4);

  switch (head.dwFourCC) {
    case 0:
      if (head._dwFlags == 0x041) {
        // rgba
        memcpy(image.get(), Data, head.dwWidth * head.dwHeight * 4);

        if (head.dwRBitMask == 0x00ff0000 && head.dwBBitMask == 0x000000ff) {
          for (int32_t x = 0; x < head.dwWidth * head.dwHeight; x++) {
            const int32_t p = x * 4;
            const int32_t t = image[p];
            image[p] = image[p + 2];
            image[p + 2] = t;
          }
        }

        break;
      } else {
        Log_Err("[core] Failed to export texture: unknown FourCC format ({:x})",
                head.dwFourCC);
        return;
      }
      break;
    case 36: {
      const uint16_t* inimg = reinterpret_cast<uint16_t*>(Data);
      for (int32_t x = 0; x < head.dwWidth * head.dwHeight * 4; x++) {
        image[x] = (!degamma ? inimg[x] : degammaint16(inimg[x])) / 256;
      }
    } break;
    case 113: {
      // D3DXFloat16To32Array()
      auto img2 = std::make_unique<float[]>(head.dwWidth * head.dwHeight * 4);
      DirectX::PackedVector::XMConvertHalfToFloatStream(
          img2.get(), 4,
          reinterpret_cast<const DirectX::PackedVector::HALF*>(Data), 2,
          head.dwWidth * head.dwHeight * 4);

      if (!degamma) {
        for (int32_t x = 0; x < head.dwWidth * head.dwHeight * 4; x++) {
          image[x] = static_cast<uint8_t>(
              std::max(0.f, std::min(255.f, img2[x] * 255.f)));
        }
      } else {
        for (int32_t x = 0; x < head.dwWidth * head.dwHeight * 4;) {
          for (int y = 0; y < 3; y++, x++) {
            image[x] = static_cast<uint8_t>(
                std::max(0.f, std::min(255.f, degammafloat(img2[x]) * 255.f)));
          }

          x++;
          image[x] = static_cast<uint8_t>(
              std::max(0.f, std::min(255.f, img2[x] * 255.f)));
        }
      }
    } break;
    case '01XD':  // DX10
    {
      const DDS_HEADER_DXT10* Head = reinterpret_cast<DDS_HEADER_DXT10*>(Data);
      Data += sizeof(DDS_HEADER_DXT10);
      switch (Head->dxgiFormat) {
        case DXGI_FORMAT_B8G8R8A8_UNORM:
          memcpy(image.get(), Data, head.dwWidth * head.dwHeight * 4);
          for (int32_t x = 0; x < head.dwWidth * head.dwHeight; x++) {
            const int32_t p = x * 4;
            const int32_t t = image[p];
            image[p] = image[p + 2];
            image[p + 2] = t;
          }
          break;
        default:
          Log_Err("[core] Failed to export texture: unknown DXGI format ({:d})",
                  static_cast<int>(Head->dxgiFormat));
          break;
      }
      break;
    }
    default:
      Log_Err("[core] Failed to export texture: unknown FourCC format ({:x})",
              head.dwFourCC);
      return;
      break;
  }

  switch (Format) {
    case EXPORTIMAGEFORMAT::CORE_PNG:
      ExportPNG(image.get(), head.dwWidth, head.dwHeight, ClearAlpha, Filename);
      break;
    case EXPORTIMAGEFORMAT::CORE_TGA:
      ExportTga(image.get(), head.dwWidth, head.dwHeight, ClearAlpha, Filename);
      break;
    case EXPORTIMAGEFORMAT::CORE_BMP:
      ExportBmp(image.get(), head.dwWidth, head.dwHeight, Filename);
      break;
    default:
      break;
  }
}

bool CCoreDX11Texture2D::CreateDepthBuffer(const int32_t xres,
                                           const int32_t yres,
                                           const int32_t MSCount) {
  if (xres <= 0 || yres <= 0) return false;
  Release();

  D3D11_TEXTURE2D_DESC tex;
  memset(&tex, 0, sizeof(D3D11_TEXTURE2D_DESC));
  tex.ArraySize = 1;
  tex.Width = xres;
  tex.Height = yres;
  tex.MipLevels = 1;
  tex.Format = DXGI_FORMAT_R24G8_TYPELESS;
  tex.SampleDesc.Count = MSCount;
  tex.SampleDesc.Quality = MSCount > 1 ? D3D10_STANDARD_MULTISAMPLE_PATTERN : 0;
  tex.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

  HRESULT res = Dev->CreateTexture2D(&tex, nullptr, &TextureHandle);
  if (res != S_OK) {
    _com_error err(res);
    Log_Err("[core] CreateTexture2D failed ({:s})", err.ErrorMessage());
    return false;
  }

  D3D11_DEPTH_STENCIL_VIEW_DESC deptdesc;

  memset(&deptdesc, 0, sizeof(deptdesc));
  deptdesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  deptdesc.ViewDimension = MSCount > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS
                                       : D3D11_DSV_DIMENSION_TEXTURE2D;
  deptdesc.Texture2D.MipSlice = 0;

  // Create the depth stencil view.
  res = Dev->CreateDepthStencilView(TextureHandle, &deptdesc, &DepthView);
  if (res != S_OK) {
    _com_error err(res);
    Log_Err("[core] CreateDepthStencilView failed ({:s})", err.ErrorMessage());
    return false;
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC resdesc;
  memset(&resdesc, 0, sizeof(resdesc));
  resdesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  resdesc.ViewDimension = MSCount > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS
                                      : D3D11_SRV_DIMENSION_TEXTURE2D;
  resdesc.Texture2D.MipLevels = 1;

  res = Dev->CreateShaderResourceView(TextureHandle, &resdesc, &View);
  if (res != S_OK) {
    _com_error err(res);
    Log_Err("[core] CreateShaderResourceView failed ({:s})",
            err.ErrorMessage());
    return false;
  }

  XRes = xres;
  YRes = yres;
  Format = COREFORMAT::UNKNOWN;

  return true;
}

CCoreDX11Texture3D::CCoreDX11Texture3D(CCoreDX11Device* dev)
    : CCoreTexture3D(dev){};

CCoreDX11TextureCube::CCoreDX11TextureCube(CCoreDX11Device* dev)
    : CCoreTextureCube(dev){};

static DXGI_FORMAT EnsureNotTypeless(DXGI_FORMAT fmt) {
  // Assumes UNORM or FLOAT; doesn't use UINT or SINT
  switch (fmt) {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
      return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case DXGI_FORMAT_R32G32B32_TYPELESS:
      return DXGI_FORMAT_R32G32B32_FLOAT;
    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
      return DXGI_FORMAT_R16G16B16A16_UNORM;
    case DXGI_FORMAT_R32G32_TYPELESS:
      return DXGI_FORMAT_R32G32_FLOAT;
    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
      return DXGI_FORMAT_R10G10B10A2_UNORM;
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
      return DXGI_FORMAT_R8G8B8A8_UNORM;
    case DXGI_FORMAT_R16G16_TYPELESS:
      return DXGI_FORMAT_R16G16_UNORM;
    case DXGI_FORMAT_R32_TYPELESS:
      return DXGI_FORMAT_R32_FLOAT;
    case DXGI_FORMAT_R8G8_TYPELESS:
      return DXGI_FORMAT_R8G8_UNORM;
    case DXGI_FORMAT_R16_TYPELESS:
      return DXGI_FORMAT_R16_UNORM;
    case DXGI_FORMAT_R8_TYPELESS:
      return DXGI_FORMAT_R8_UNORM;
    case DXGI_FORMAT_BC1_TYPELESS:
      return DXGI_FORMAT_BC1_UNORM;
    case DXGI_FORMAT_BC2_TYPELESS:
      return DXGI_FORMAT_BC2_UNORM;
    case DXGI_FORMAT_BC3_TYPELESS:
      return DXGI_FORMAT_BC3_UNORM;
    case DXGI_FORMAT_BC4_TYPELESS:
      return DXGI_FORMAT_BC4_UNORM;
    case DXGI_FORMAT_BC5_TYPELESS:
      return DXGI_FORMAT_BC5_UNORM;
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
      return DXGI_FORMAT_B8G8R8A8_UNORM;
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
      return DXGI_FORMAT_B8G8R8X8_UNORM;
    case DXGI_FORMAT_BC7_TYPELESS:
      return DXGI_FORMAT_BC7_UNORM;
    default:
      return fmt;
  }
}

constexpr static size_t BitsPerPixel(_In_ DXGI_FORMAT fmt) noexcept {
  switch (fmt) {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
      return 128;

    case DXGI_FORMAT_R32G32B32_TYPELESS:
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
      return 96;

    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
    case DXGI_FORMAT_R32G32_TYPELESS:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
    case DXGI_FORMAT_Y416:
    case DXGI_FORMAT_Y210:
    case DXGI_FORMAT_Y216:
      return 64;

    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_R16G16_TYPELESS:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
    case DXGI_FORMAT_AYUV:
    case DXGI_FORMAT_Y410:
    case DXGI_FORMAT_YUY2:
      return 32;

    case DXGI_FORMAT_P010:
    case DXGI_FORMAT_P016:
      return 24;

    case DXGI_FORMAT_R8G8_TYPELESS:
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_A8P8:
    case DXGI_FORMAT_B4G4R4A4_UNORM:
      return 16;

    case DXGI_FORMAT_NV12:
    case DXGI_FORMAT_420_OPAQUE:
    case DXGI_FORMAT_NV11:
      return 12;

    case DXGI_FORMAT_R8_TYPELESS:
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
    case DXGI_FORMAT_A8_UNORM:
    case DXGI_FORMAT_AI44:
    case DXGI_FORMAT_IA44:
    case DXGI_FORMAT_P8:
      return 8;

    case DXGI_FORMAT_R1_UNORM:
      return 1;

    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
      return 4;

    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
      return 8;

#if defined(_XBOX_ONE) && defined(_TITLE)

    case DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
    case DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
      return 32;

    case DXGI_FORMAT_D16_UNORM_S8_UINT:
    case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
      return 24;

#endif  // _XBOX_ONE && _TITLE

    default:
      return 0;
  }
}

static void GetSurfaceInfo(_In_ size_t width, _In_ size_t height,
                           _In_ DXGI_FORMAT fmt, _Out_opt_ size_t* outNumBytes,
                           _Out_opt_ size_t* outRowBytes,
                           _Out_opt_ size_t* outNumRows) {
  size_t numBytes = 0;
  size_t rowBytes = 0;
  size_t numRows = 0;

  bool bc = false;
  bool packed = false;
  bool planar = false;
  size_t bpe = 0;
  switch (fmt) {
    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
      bc = true;
      bpe = 8;
      break;

    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
      bc = true;
      bpe = 16;
      break;

    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case DXGI_FORMAT_YUY2:
      packed = true;
      bpe = 4;
      break;

    case DXGI_FORMAT_Y210:
    case DXGI_FORMAT_Y216:
      packed = true;
      bpe = 8;
      break;

    case DXGI_FORMAT_NV12:
    case DXGI_FORMAT_420_OPAQUE:
      planar = true;
      bpe = 2;
      break;

    case DXGI_FORMAT_P010:
    case DXGI_FORMAT_P016:
      planar = true;
      bpe = 4;
      break;

#if defined(_XBOX_ONE) && defined(_TITLE)

    case DXGI_FORMAT_D16_UNORM_S8_UINT:
    case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
      planar = true;
      bpe = 4;
      break;

#endif
  }

  if (bc) {
    size_t numBlocksWide = 0;
    if (width > 0) {
      numBlocksWide = std::max(1u, (width + 3u) / 4u);
    }
    size_t numBlocksHigh = 0;
    if (height > 0) {
      numBlocksHigh = std::max(1u, (height + 3u) / 4u);
    }
    rowBytes = numBlocksWide * bpe;
    numRows = numBlocksHigh;
    numBytes = rowBytes * numBlocksHigh;
  } else if (packed) {
    rowBytes = ((width + 1) >> 1) * bpe;
    numRows = height;
    numBytes = rowBytes * height;
  } else if (fmt == DXGI_FORMAT_NV11) {
    rowBytes = ((width + 3) >> 2) * 4;
    numRows = height * 2;  // Direct3D makes this simplifying assumption,
                           // although it is larger than the 4:1:1 data
    numBytes = rowBytes * numRows;
  } else if (planar) {
    rowBytes = ((width + 1) >> 1) * bpe;
    numBytes = (rowBytes * height) + ((rowBytes * height + 1) >> 1);
    numRows = height + ((height + 1) >> 1);
  } else {
    const size_t bpp = BitsPerPixel(fmt);
    rowBytes = (width * bpp + 7) / 8;  // round up to nearest byte
    numRows = height;
    numBytes = rowBytes * height;
  }

  if (outNumBytes) {
    *outNumBytes = numBytes;
  }
  if (outRowBytes) {
    *outRowBytes = rowBytes;
  }
  if (outNumRows) {
    *outNumRows = numRows;
  }
}

static bool IsCompressed(_In_ DXGI_FORMAT fmt) {
  switch (fmt) {
    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
      return true;

    default:
      return false;
  }
}

static HRESULT CaptureTexture(ID3D11Device* d3dDevice,
                              _In_ ID3D11DeviceContext* pContext,
                              _In_ ID3D11Resource* pSource,
                              _Inout_ D3D11_TEXTURE2D_DESC& desc,
                              _Inout_ ID3D11Texture2D*& pStaging) {
  if (!pContext || !pSource) return E_INVALIDARG;

  D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
  pSource->GetType(&resType);

  if (resType != D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
    return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
  }

  ID3D11Texture2D* pTexture = nullptr;
  HRESULT hr = pSource->QueryInterface(__uuidof(ID3D11Texture2D),
                                       reinterpret_cast<void**>(&pTexture));
  if (FAILED(hr)) return hr;

  pTexture->GetDesc(&desc);

  if (desc.SampleDesc.Count > 1) {
    // MSAA content must be resolved before being copied to a staging texture
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    ID3D11Texture2D* pTemp = nullptr;
    hr = d3dDevice->CreateTexture2D(&desc, nullptr, &pTemp);
    if (FAILED(hr)) {
      if (pTexture) pTexture->Release();
      return hr;
    }

    if (!pTemp) Log_Err("[Core2] Error creating temp texture");

    const DXGI_FORMAT fmt = EnsureNotTypeless(desc.Format);

    UINT support = 0;
    hr = d3dDevice->CheckFormatSupport(fmt, &support);
    if (FAILED(hr)) {
      if (pTemp) pTemp->Release();
      if (pTexture) pTexture->Release();
      return hr;
    }

    if (!(support & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE)) {
      if (pTemp) pTemp->Release();
      if (pTexture) pTexture->Release();
      return E_FAIL;
    }

    for (UINT item = 0; item < desc.ArraySize; ++item) {
      for (UINT level = 0; level < desc.MipLevels; ++level) {
        const UINT index = D3D11CalcSubresource(level, item, desc.MipLevels);
        pContext->ResolveSubresource(pTemp, index, pSource, index, fmt);
      }
    }

    desc.BindFlags = 0;
    desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;

    hr = d3dDevice->CreateTexture2D(&desc, nullptr, &pStaging);
    if (FAILED(hr)) {
      if (pTemp) pTemp->Release();
      if (pTexture) pTexture->Release();
      return hr;
    }

    if (!pStaging) Log_Err("[Core2] Error creating staging texture");

    pContext->CopyResource(pStaging, pTemp);
    if (pTemp) pTemp->Release();
    if (pTexture) pTexture->Release();
  } else if ((desc.Usage == D3D11_USAGE_STAGING) &&
             (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ)) {
    // Handle case where the source is already a staging texture we can use
    // directly
    pStaging = pTexture;
  } else {
    // Otherwise, create a staging texture from the non-MSAA source
    desc.BindFlags = 0;
    desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;

    hr = d3dDevice->CreateTexture2D(&desc, nullptr, &pStaging);
    if (FAILED(hr)) {
      if (pTexture) pTexture->Release();
      return hr;
    }

    if (!pStaging) Log_Err("[Core2] Error creating staging texture");

    pContext->CopyResource(pStaging, pSource);
    if (pTexture) pTexture->Release();
  }

  return S_OK;
}

HRESULT SaveDDSTexture(_In_ ID3D11DeviceContext* pContext,
                       _In_ ID3D11Resource* pSource, CStreamWriter& Writer) {
  constexpr uint32_t DDS_MAGIC = 0x20534444;  // "DDS "

  struct DDS_PIXELFORMAT {
    uint32_t size;
    uint32_t flags;
    uint32_t fourCC;
    uint32_t RGBBitCount;
    uint32_t RBitMask;
    uint32_t GBitMask;
    uint32_t BBitMask;
    uint32_t ABitMask;
  };

#define DDS_FOURCC 0x00000004      // DDPF_FOURCC
#define DDS_RGB 0x00000040         // DDPF_RGB
#define DDS_RGBA 0x00000041        // DDPF_RGB | DDPF_ALPHAPIXELS
#define DDS_LUMINANCE 0x00020000   // DDPF_LUMINANCE
#define DDS_LUMINANCEA 0x00020001  // DDPF_LUMINANCE | DDPF_ALPHAPIXELS
#define DDS_ALPHA 0x00000002       // DDPF_ALPHA
#define DDS_PAL8 0x00000020        // DDPF_PALETTEINDEXED8

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                          \
  ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | \
   ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24))
#endif  // !MAKEFOURCC

#define DDS_HEADER_FLAGS_TEXTURE \
  0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
#define DDS_HEADER_FLAGS_MIPMAP 0x00020000      // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME 0x00800000      // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH 0x00000008       // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE 0x00080000  // DDSD_LINEARSIZE

#define DDS_HEIGHT 0x00000002  // DDSD_HEIGHT
#define DDS_WIDTH 0x00000004   // DDSD_WIDTH

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000  // DDSCAPS_TEXTURE
#define DDS_SURFACE_FLAGS_MIPMAP 0x00400008  // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008  // DDSCAPS_COMPLEX

#define DDS_CUBEMAP_POSITIVEX \
  0x00000600  // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX \
  0x00000a00  // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY \
  0x00001200  // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY \
  0x00002200  // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ \
  0x00004200  // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ \
  0x00008200  // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES                                               \
  (DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX | DDS_CUBEMAP_POSITIVEY | \
   DDS_CUBEMAP_NEGATIVEY | DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ)

#define DDS_CUBEMAP 0x00000200  // DDSCAPS2_CUBEMAP

#define DDS_FLAGS_VOLUME 0x00200000  // DDSCAPS2_VOLUME

  struct DDS_HEADER {
    uint32_t size;
    uint32_t flags;
    uint32_t height;
    uint32_t width;
    uint32_t pitchOrLinearSize;
    uint32_t depth;  // only if DDS_HEADER_FLAGS_VOLUME is set in flags
    uint32_t mipMapCount;
    uint32_t reserved1[11];
    DDS_PIXELFORMAT ddspf;
    uint32_t caps;
    uint32_t caps2;
    uint32_t caps3;
    uint32_t caps4;
    uint32_t reserved2;
  };

  struct DDS_HEADER_DXT10 {
    DXGI_FORMAT dxgiFormat;
    uint32_t resourceDimension;
    uint32_t miscFlag;  // see D3D11_RESOURCE_MISC_FLAG
    uint32_t arraySize;
    uint32_t miscFlags2;  // see DDS_MISC_FLAGS2
  };

  const DDS_PIXELFORMAT DDSPF_DXT1 = {sizeof(DDS_PIXELFORMAT),
                                      DDS_FOURCC,
                                      MAKEFOURCC('D', 'X', 'T', '1'),
                                      0,
                                      0,
                                      0,
                                      0,
                                      0};
  const DDS_PIXELFORMAT DDSPF_DXT2 = {sizeof(DDS_PIXELFORMAT),
                                      DDS_FOURCC,
                                      MAKEFOURCC('D', 'X', 'T', '2'),
                                      0,
                                      0,
                                      0,
                                      0,
                                      0};
  const DDS_PIXELFORMAT DDSPF_DXT3 = {sizeof(DDS_PIXELFORMAT),
                                      DDS_FOURCC,
                                      MAKEFOURCC('D', 'X', 'T', '3'),
                                      0,
                                      0,
                                      0,
                                      0,
                                      0};
  const DDS_PIXELFORMAT DDSPF_DXT4 = {sizeof(DDS_PIXELFORMAT),
                                      DDS_FOURCC,
                                      MAKEFOURCC('D', 'X', 'T', '4'),
                                      0,
                                      0,
                                      0,
                                      0,
                                      0};
  const DDS_PIXELFORMAT DDSPF_DXT5 = {sizeof(DDS_PIXELFORMAT),
                                      DDS_FOURCC,
                                      MAKEFOURCC('D', 'X', 'T', '5'),
                                      0,
                                      0,
                                      0,
                                      0,
                                      0};
  const DDS_PIXELFORMAT DDSPF_BC4_UNORM = {sizeof(DDS_PIXELFORMAT),
                                           DDS_FOURCC,
                                           MAKEFOURCC('B', 'C', '4', 'U'),
                                           0,
                                           0,
                                           0,
                                           0,
                                           0};
  const DDS_PIXELFORMAT DDSPF_BC4_SNORM = {sizeof(DDS_PIXELFORMAT),
                                           DDS_FOURCC,
                                           MAKEFOURCC('B', 'C', '4', 'S'),
                                           0,
                                           0,
                                           0,
                                           0,
                                           0};
  const DDS_PIXELFORMAT DDSPF_BC5_UNORM = {sizeof(DDS_PIXELFORMAT),
                                           DDS_FOURCC,
                                           MAKEFOURCC('B', 'C', '5', 'U'),
                                           0,
                                           0,
                                           0,
                                           0,
                                           0};
  const DDS_PIXELFORMAT DDSPF_BC5_SNORM = {sizeof(DDS_PIXELFORMAT),
                                           DDS_FOURCC,
                                           MAKEFOURCC('B', 'C', '5', 'S'),
                                           0,
                                           0,
                                           0,
                                           0,
                                           0};
  const DDS_PIXELFORMAT DDSPF_R8G8_B8G8 = {sizeof(DDS_PIXELFORMAT),
                                           DDS_FOURCC,
                                           MAKEFOURCC('R', 'G', 'B', 'G'),
                                           0,
                                           0,
                                           0,
                                           0,
                                           0};
  const DDS_PIXELFORMAT DDSPF_G8R8_G8B8 = {sizeof(DDS_PIXELFORMAT),
                                           DDS_FOURCC,
                                           MAKEFOURCC('G', 'R', 'G', 'B'),
                                           0,
                                           0,
                                           0,
                                           0,
                                           0};
  const DDS_PIXELFORMAT DDSPF_YUY2 = {sizeof(DDS_PIXELFORMAT),
                                      DDS_FOURCC,
                                      MAKEFOURCC('Y', 'U', 'Y', '2'),
                                      0,
                                      0,
                                      0,
                                      0,
                                      0};
  const DDS_PIXELFORMAT DDSPF_A8R8G8B8 = {sizeof(DDS_PIXELFORMAT),
                                          DDS_RGBA,
                                          0,
                                          32,
                                          0x00ff0000,
                                          0x0000ff00,
                                          0x000000ff,
                                          0xff000000};
  const DDS_PIXELFORMAT DDSPF_X8R8G8B8 = {sizeof(DDS_PIXELFORMAT),
                                          DDS_RGB,
                                          0,
                                          32,
                                          0x00ff0000,
                                          0x0000ff00,
                                          0x000000ff,
                                          0x00000000};
  const DDS_PIXELFORMAT DDSPF_A8B8G8R8 = {sizeof(DDS_PIXELFORMAT),
                                          DDS_RGBA,
                                          0,
                                          32,
                                          0x000000ff,
                                          0x0000ff00,
                                          0x00ff0000,
                                          0xff000000};
  const DDS_PIXELFORMAT DDSPF_X8B8G8R8 = {sizeof(DDS_PIXELFORMAT),
                                          DDS_RGB,
                                          0,
                                          32,
                                          0x000000ff,
                                          0x0000ff00,
                                          0x00ff0000,
                                          0x00000000};
  const DDS_PIXELFORMAT DDSPF_G16R16 = {sizeof(DDS_PIXELFORMAT),
                                        DDS_RGB,
                                        0,
                                        32,
                                        0x0000ffff,
                                        0xffff0000,
                                        0x00000000,
                                        0x00000000};
  const DDS_PIXELFORMAT DDSPF_R5G6B5 = {sizeof(DDS_PIXELFORMAT),
                                        DDS_RGB,
                                        0,
                                        16,
                                        0x0000f800,
                                        0x000007e0,
                                        0x0000001f,
                                        0x00000000};
  const DDS_PIXELFORMAT DDSPF_A1R5G5B5 = {sizeof(DDS_PIXELFORMAT),
                                          DDS_RGBA,
                                          0,
                                          16,
                                          0x00007c00,
                                          0x000003e0,
                                          0x0000001f,
                                          0x00008000};
  const DDS_PIXELFORMAT DDSPF_A4R4G4B4 = {sizeof(DDS_PIXELFORMAT),
                                          DDS_RGBA,
                                          0,
                                          16,
                                          0x00000f00,
                                          0x000000f0,
                                          0x0000000f,
                                          0x0000f000};
  const DDS_PIXELFORMAT DDSPF_R8G8B8 = {sizeof(DDS_PIXELFORMAT),
                                        DDS_RGB,
                                        0,
                                        24,
                                        0x00ff0000,
                                        0x0000ff00,
                                        0x000000ff,
                                        0x00000000};
  const DDS_PIXELFORMAT DDSPF_L8 = {
      sizeof(DDS_PIXELFORMAT), DDS_LUMINANCE, 0, 8, 0xff, 0x00, 0x00, 0x00};
  const DDS_PIXELFORMAT DDSPF_L16 = {sizeof(DDS_PIXELFORMAT),
                                     DDS_LUMINANCE,
                                     0,
                                     16,
                                     0xffff,
                                     0x0000,
                                     0x0000,
                                     0x0000};
  const DDS_PIXELFORMAT DDSPF_A8L8 = {sizeof(DDS_PIXELFORMAT),
                                      DDS_LUMINANCEA,
                                      0,
                                      16,
                                      0x00ff,
                                      0x0000,
                                      0x0000,
                                      0xff00};
  const DDS_PIXELFORMAT DDSPF_A8 = {
      sizeof(DDS_PIXELFORMAT), DDS_ALPHA, 0, 8, 0x00, 0x00, 0x00, 0xff};

  // D3DFMT_A2R10G10B10/D3DFMT_A2B10G10R10 should be written using DX10
  // extension to avoid D3DX 10:10:10:2 reversal issue This indicates the
  // DDS_HEADER_DXT10 extension is present (the format is in dxgiFormat)
  const DDS_PIXELFORMAT DDSPF_DX10 = {sizeof(DDS_PIXELFORMAT),
                                      DDS_FOURCC,
                                      MAKEFOURCC('D', 'X', '1', '0'),
                                      0,
                                      0,
                                      0,
                                      0,
                                      0};

  D3D11_TEXTURE2D_DESC desc = {0};
  ID3D11Texture2D* pStaging = nullptr;

  ID3D11Device* d3dDevice = nullptr;
  pContext->GetDevice(&d3dDevice);

  HRESULT hr = CaptureTexture(d3dDevice, pContext, pSource, desc, pStaging);
  d3dDevice->Release();

  if (FAILED(hr)) {
    if (pStaging) pStaging->Release();
    return hr;
  }

  // Setup header
  constexpr size_t MAX_HEADER_SIZE =
      sizeof(uint32_t) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10);
  uint8_t fileHeader[MAX_HEADER_SIZE];

  *(reinterpret_cast<uint32_t*>(&fileHeader[0])) = DDS_MAGIC;

  auto header =
      reinterpret_cast<DDS_HEADER*>(&fileHeader[0] + sizeof(uint32_t));
  size_t headerSize = sizeof(uint32_t) + sizeof(DDS_HEADER);
  memset(header, 0, sizeof(DDS_HEADER));
  header->size = sizeof(DDS_HEADER);
  header->flags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_MIPMAP;
  header->height = desc.Height;
  header->width = desc.Width;
  header->mipMapCount = 1;
  header->caps = DDS_SURFACE_FLAGS_TEXTURE;

  // Try to use a legacy .DDS pixel format for better tools support, otherwise
  // fallback to 'DX10' header extension
  DDS_HEADER_DXT10* extHeader = nullptr;
  switch (desc.Format) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_A8B8G8R8,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_R16G16_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_G16R16,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_R8G8_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_A8L8,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_R16_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_L16,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_R8_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_L8,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_A8_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_A8,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_R8G8_B8G8,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_G8R8_G8B8,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_BC1_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_DXT1,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_BC2_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_DXT3,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_BC3_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_DXT5,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_BC4_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_BC4_UNORM,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_BC4_SNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_BC4_SNORM,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_BC5_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_BC5_UNORM,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_BC5_SNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_BC5_SNORM,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_B5G6R5_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_R5G6B5,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_B5G5R5A1_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_A1R5G5B5,
               sizeof(DDS_PIXELFORMAT));
      break;
    case DXGI_FORMAT_B8G8R8A8_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_A8R8G8B8,
               sizeof(DDS_PIXELFORMAT));
      break;  // DXGI 1.1
    case DXGI_FORMAT_B8G8R8X8_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_X8R8G8B8,
               sizeof(DDS_PIXELFORMAT));
      break;  // DXGI 1.1
    case DXGI_FORMAT_YUY2:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_YUY2,
               sizeof(DDS_PIXELFORMAT));
      break;  // DXGI 1.2
    case DXGI_FORMAT_B4G4R4A4_UNORM:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_A4R4G4B4,
               sizeof(DDS_PIXELFORMAT));
      break;  // DXGI 1.2

      // Legacy D3DX formats using D3DFMT enum value as FourCC
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
      header->ddspf.size = sizeof(DDS_PIXELFORMAT);
      header->ddspf.flags = DDS_FOURCC;
      header->ddspf.fourCC = 116;
      break;  // D3DFMT_A32B32G32R32F
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
      header->ddspf.size = sizeof(DDS_PIXELFORMAT);
      header->ddspf.flags = DDS_FOURCC;
      header->ddspf.fourCC = 113;
      break;  // D3DFMT_A16B16G16R16F
    case DXGI_FORMAT_R16G16B16A16_UNORM:
      header->ddspf.size = sizeof(DDS_PIXELFORMAT);
      header->ddspf.flags = DDS_FOURCC;
      header->ddspf.fourCC = 36;
      break;  // D3DFMT_A16B16G16R16
    case DXGI_FORMAT_R16G16B16A16_SNORM:
      header->ddspf.size = sizeof(DDS_PIXELFORMAT);
      header->ddspf.flags = DDS_FOURCC;
      header->ddspf.fourCC = 110;
      break;  // D3DFMT_Q16W16V16U16
    case DXGI_FORMAT_R32G32_FLOAT:
      header->ddspf.size = sizeof(DDS_PIXELFORMAT);
      header->ddspf.flags = DDS_FOURCC;
      header->ddspf.fourCC = 115;
      break;  // D3DFMT_G32R32F
    case DXGI_FORMAT_R16G16_FLOAT:
      header->ddspf.size = sizeof(DDS_PIXELFORMAT);
      header->ddspf.flags = DDS_FOURCC;
      header->ddspf.fourCC = 112;
      break;  // D3DFMT_G16R16F
    case DXGI_FORMAT_R32_FLOAT:
      header->ddspf.size = sizeof(DDS_PIXELFORMAT);
      header->ddspf.flags = DDS_FOURCC;
      header->ddspf.fourCC = 114;
      break;  // D3DFMT_R32F
    case DXGI_FORMAT_R16_FLOAT:
      header->ddspf.size = sizeof(DDS_PIXELFORMAT);
      header->ddspf.flags = DDS_FOURCC;
      header->ddspf.fourCC = 111;
      break;  // D3DFMT_R16F

    case DXGI_FORMAT_AI44:
    case DXGI_FORMAT_IA44:
    case DXGI_FORMAT_P8:
    case DXGI_FORMAT_A8P8: {
      if (pStaging) pStaging->Release();
      return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    }

    default:
      memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_DX10,
               sizeof(DDS_PIXELFORMAT));

      headerSize += sizeof(DDS_HEADER_DXT10);
      extHeader = reinterpret_cast<DDS_HEADER_DXT10*>(
          (&fileHeader[0]) + sizeof(uint32_t) + sizeof(DDS_HEADER));
      memset(extHeader, 0, sizeof(DDS_HEADER_DXT10));
      extHeader->dxgiFormat = desc.Format;
      extHeader->resourceDimension = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
      extHeader->arraySize = 1;
      break;
  }

  size_t rowPitch = 0, slicePitch = 0, rowCount = 0;
  GetSurfaceInfo(desc.Width, desc.Height, desc.Format, &slicePitch, &rowPitch,
                 &rowCount);

  if (IsCompressed(desc.Format)) {
    header->flags |= DDS_HEADER_FLAGS_LINEARSIZE;
    header->pitchOrLinearSize = static_cast<uint32_t>(slicePitch);
  } else {
    header->flags |= DDS_HEADER_FLAGS_PITCH;
    header->pitchOrLinearSize = static_cast<uint32_t>(rowPitch);
  }

  // Setup pixels
  auto pixels = std::make_unique<uint8_t[]>(slicePitch);
  if (!pixels) {
    if (pStaging) pStaging->Release();
    return E_OUTOFMEMORY;
  }

  D3D11_MAPPED_SUBRESOURCE mapped;
  hr = pContext->Map(pStaging, 0, D3D11_MAP_READ, 0, &mapped);
  if (FAILED(hr)) {
    if (pStaging) pStaging->Release();
    return hr;
  }

  auto sptr = static_cast<uint8_t*>(mapped.pData);
  if (!sptr) {
    pContext->Unmap(pStaging, 0);
    if (pStaging) pStaging->Release();
    return E_POINTER;
  }

  uint8_t* dptr = pixels.get();

  const size_t msize = std::min(rowPitch, mapped.RowPitch);
  for (size_t h = 0; h < rowCount; ++h) {
    memcpy_s(dptr, rowPitch, sptr, msize);
    sptr += mapped.RowPitch;
    dptr += rowPitch;
  }

  pContext->Unmap(pStaging, 0);

  // Write header & pixels

  if (!Writer.Write(
          uint8_view(&fileHeader[0], static_cast<DWORD>(headerSize)))) {
    if (pStaging) pStaging->Release();
    return E_FAIL;
  }
  if (!Writer.Write(uint8_view(pixels.get(), static_cast<DWORD>(slicePitch)))) {
    if (pStaging) pStaging->Release();
    return E_FAIL;
  }

  if (pStaging) pStaging->Release();
  return S_OK;
}
