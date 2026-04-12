#pragma once
#include <d3d11.h>

#include <cstdint>
#include <string_view>

#include "src/base/stream_writer.h"
#include "src/core2/dx11_device.h"
#include "src/core2/texture.h"

namespace renderer {

class CCoreDX11Texture2D : public CCoreTexture2D {
 public:
  explicit CCoreDX11Texture2D(CCoreDX11Device* device);
  ~CCoreDX11Texture2D() override;

  void OnDeviceLost() override;
  void OnDeviceReset() override;

  bool Create(const int32_t x_res, const int32_t y_res, const uint8_t* data,
              const char bytes_per_pixel = 4,
              const CoreFormat format = CoreFormat::kA8R8G8B8,
              const bool render_target = false) override;
  bool Create(const uint8_t* data, const int32_t size) override;
  bool CreateDepthBuffer(const int32_t x_res, const int32_t y_res,
                         const int32_t ms_count = 1) override;
  bool Lock(void** result, int32_t& pitch) override;
  bool UnLock() override;

  bool Update(const uint8_t* data, const int32_t x_res, const int32_t y_res,
              const char bytes_per_pixel = 4) override;
  void SetTextureHandle(ID3D11Texture2D* handle) { texture_handle_ = handle; }
  void SetView(ID3D11ShaderResourceView* view) { view_ = view; }

  ID3D11Texture2D* GetTextureHandle() { return texture_handle_; }
  ID3D11DepthStencilView* GetDepthView() { return depth_view_; }
  ID3D11RenderTargetView* GetRenderTargetView() { return rt_view_; }
  ID3D11ShaderResourceView* GetShaderResourceView() { return view_; }
  ID3D11DeviceContext* GetDeviceContext() { return d3d_device_context_; }

  void ExportToImage(std::string_view filename, bool clear_alpha,
                     ExportImageFormat format, bool degamma) override;

 private:
  virtual void Release();
  bool SetToSampler(const CoreSampler sampler) override;

  ID3D11Device* d3d_device_;
  ID3D11DeviceContext* d3d_device_context_;
  ID3D11Texture2D* texture_handle_;
  ID3D11ShaderResourceView* view_;
  ID3D11RenderTargetView* rt_view_;
  ID3D11DepthStencilView* depth_view_;

  bool render_target_;
};

class CCoreDX11Texture3D : public CCoreTexture3D {
 public:
  explicit CCoreDX11Texture3D(CCoreDX11Device* device);
};

class CCoreDX11TextureCube : public CCoreTextureCube {
 public:
  explicit CCoreDX11TextureCube(CCoreDX11Device* device);
};

HRESULT SaveDDSTexture(_In_ ID3D11DeviceContext* d3d_device_context,
                       _In_ ID3D11Resource* source, CStreamWriter& writer);

}  // namespace renderer
