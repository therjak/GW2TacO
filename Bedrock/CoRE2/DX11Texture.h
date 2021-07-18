#pragma once
#include "DX11Device.h"
#include "Texture.h"

#ifdef CORE_API_DX11

class CCoreDX11Texture2D : public CCoreTexture2D {
  ID3D11Device* Dev;
  ID3D11DeviceContext* DeviceContext;
  ID3D11Texture2D* TextureHandle;
  ID3D11ShaderResourceView* View;
  ID3D11RenderTargetView* RTView;
  ID3D11DepthStencilView* DepthView;

  bool RenderTarget;

  virtual void Release();
  bool SetToSampler(const CORESAMPLER Sampler) override;

 public:
  CCoreDX11Texture2D(CCoreDX11Device* Device);
  ~CCoreDX11Texture2D() override;

  void OnDeviceLost() override;
  void OnDeviceReset() override;

  bool Create(const int32_t XRes, const int32_t YRes, const uint8_t* Data,
              const char BytesPerPixel = 4,
              const COREFORMAT Format = COREFMT_A8R8G8B8,
              const bool RenderTarget = false) override;
  bool Create(const uint8_t* Data, const int32_t Size) override;
  bool CreateDepthBuffer(const int32_t XRes, const int32_t YRes,
                         const int32_t MSCount = 1) override;
  bool Lock(void** Result, int32_t& pitch) override;
  bool UnLock() override;

  bool Update(const uint8_t* Data, const int32_t XRes, const int32_t YRes,
              const char BytesPerPixel = 4) override;
  void SetTextureHandle(ID3D11Texture2D* Hnd) { TextureHandle = Hnd; }
  void SetView(ID3D11ShaderResourceView* v) { View = v; }

  ID3D11Texture2D* GetTextureHandle() { return TextureHandle; }
  ID3D11DepthStencilView* GetDepthView() { return DepthView; }
  ID3D11RenderTargetView* GetRenderTargetView() { return RTView; }
  ID3D11ShaderResourceView* GetShaderResourceView() { return View; }
  ID3D11DeviceContext* GetDeviceContext() { return DeviceContext; }

  CCoreTexture2D* Copy() override;
  void ExportToImage(std::string_view Filename, bool ClearAlpha,
                     EXPORTIMAGEFORMAT Format, bool degamma) override;
};

class CCoreDX11Texture3D : public CCoreTexture3D {
  // LPDIRECT3DDEVICE9 Dev;

 public:
  CCoreDX11Texture3D(CCoreDX11Device* Device);
};

class CCoreDX11TextureCube : public CCoreTextureCube {
  // LPDIRECT3DDEVICE9 Dev;

 public:
  CCoreDX11TextureCube(CCoreDX11Device* Device);
};

HRESULT SaveDDSTexture(_In_ ID3D11DeviceContext* pContext,
                       _In_ ID3D11Resource* pSource, CStreamWriter& Writer);

#endif
