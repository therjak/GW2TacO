#pragma once
#include <dxgi1_3.h>

#include <vector>

#include "../BaseLib/BaseLib.h"
#include "../BaseLib/Color.h"
#include "Core2_Config.h"
#include "DX11Enums.h"
#include "Device.h"

#ifdef CORE_API_DX11

class CCoreDX11Device : public CCoreDevice {
  IDXGISwapChain1* SwapChain;
  ID3D11Device* Device;
  ID3D11DeviceContext* DeviceContext;

  ID3D11RenderTargetView* BackBufferView;
  ID3D11Texture2D* DepthBuffer;
  ID3D11DepthStencilView* DepthBufferView;

  ID3D11BlendState* CurrentBlendState;
  ID3D11DepthStencilState* CurrentDepthStencilState;
  ID3D11RasterizerState* CurrentRasterizerState;

  ID3D11Query* OcclusionQuery = nullptr;

  void ResetPrivateResources() override;
  bool InitAPI(const uint32_t hWnd, const bool FullScreen, const int32_t XRes,
               const int32_t YRes, const int32_t AALevel = 0,
               const int32_t RefreshRate = 60) override;
  bool ApplyRenderState(const CORESAMPLER Sampler,
                        const CORERENDERSTATE RenderState,
                        const CORERENDERSTATEVALUE Value) override;
  bool SetNoVertexBuffer() override;
  bool CommitRenderStates() override;

  virtual bool CreateBackBuffer(int32_t XRes, int32_t YRes);
  virtual bool CreateDepthBuffer(int32_t XRes, int32_t YRes);

  bool CreateClassicSwapChain(const uint32_t hWnd, const bool FullScreen,
                              const int32_t XRes, const int32_t YRes,
                              const int32_t AALevel, const int32_t RefreshRate);
  bool CreateDirectCompositionSwapchain(const uint32_t hWnd,
                                        const bool FullScreen,
                                        const int32_t XRes, const int32_t YRes,
                                        const int32_t AALevel,
                                        const int32_t RefreshRate);

 public:
  CCoreDX11Device();
  ~CCoreDX11Device() override;
  INLINE ID3D11Device* GetDevice() { return Device; }
  INLINE ID3D11DeviceContext* GetDeviceContext() { return DeviceContext; }
  COREDEVICEAPI GetAPIType() override { return COREAPI_DX11; }

  // this initializer will change to accommodate multiple platforms at once once
  // we get to that point:
  bool Initialize(CCoreWindowHandler* Window,
                  const int32_t AALevel = 0) override;

  bool DeviceOk() override;
  bool IsWindowed() override;
  void Resize(const int32_t xr, const int32_t yr) override;
  void SetFullScreenMode(const bool FullScreen, const int32_t xr,
                         const int32_t yr) override;

  ID3D11Texture2D* GetBackBuffer();

  //////////////////////////////////////////////////////////////////////////
  // texture functions

  std::unique_ptr<CCoreTexture2D> CreateTexture2D(
      const int32_t XRes, const int32_t YRes, const uint8_t* Data,
      const char BytesPerPixel = 4, const COREFORMAT Format = COREFMT_A8R8G8B8,
      const bool RenderTarget = false) override;
  std::unique_ptr<CCoreTexture2D> CreateTexture2D(const uint8_t* Data,
                                                  const int32_t Size) override;

  //////////////////////////////////////////////////////////////////////////
  // vertexbuffer functions

  std::unique_ptr<CCoreVertexBuffer> CreateVertexBuffer(
      const uint8_t* Data, const int32_t Size) override;
  std::unique_ptr<CCoreVertexBuffer> CreateVertexBufferDynamic(
      const int32_t Size) override;

  //////////////////////////////////////////////////////////////////////////
  // indexbuffer functions

  std::unique_ptr<CCoreIndexBuffer> CreateIndexBuffer(
      const int32_t IndexCount, const int32_t IndexSize = 2) override;

  //////////////////////////////////////////////////////////////////////////
  // vertexformat functions

  std::unique_ptr<CCoreVertexFormat> CreateVertexFormat(
      const std::vector<COREVERTEXATTRIBUTE>& Attributes,
      CCoreVertexShader* vs = nullptr) override;

  //////////////////////////////////////////////////////////////////////////
  // shader functions

  std::unique_ptr<CCoreVertexShader> CreateVertexShader(
      LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
      std::string* Err = nullptr) override;
  std::unique_ptr<CCorePixelShader> CreatePixelShader(
      LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
      std::string* Err = nullptr) override;
  std::unique_ptr<CCoreVertexShader> CreateVertexShaderFromBlob(
      uint8_t* Code, int32_t CodeSize) override;
  std::unique_ptr<CCorePixelShader> CreatePixelShaderFromBlob(
      uint8_t* Code, int32_t CodeSize) override;
  std::unique_ptr<CCoreGeometryShader> CreateGeometryShader(
      LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
      std::string* Err = nullptr) override;
  std::unique_ptr<CCoreDomainShader> CreateDomainShader(
      LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
      std::string* Err = nullptr) override;
  std::unique_ptr<CCoreHullShader> CreateHullShader(
      LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
      std::string* Err = nullptr) override;
  std::unique_ptr<CCoreComputeShader> CreateComputeShader(
      LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
      std::string* Err = nullptr) override;
  std::unique_ptr<CCoreVertexShader> CreateVertexShader() override;
  std::unique_ptr<CCorePixelShader> CreatePixelShader() override;
  std::unique_ptr<CCoreGeometryShader> CreateGeometryShader() override;
  std::unique_ptr<CCoreDomainShader> CreateDomainShader() override;
  std::unique_ptr<CCoreHullShader> CreateHullShader() override;
  std::unique_ptr<CCoreComputeShader> CreateComputeShader() override;
  void SetShaderConstants(const CCoreConstantBuffer* Buffers) override;
  std::unique_ptr<CCoreConstantBuffer> CreateConstantBuffer() override;

  std::unique_ptr<CCoreBlendState> CreateBlendState() override;
  std::unique_ptr<CCoreDepthStencilState> CreateDepthStencilState() override;
  std::unique_ptr<CCoreRasterizerState> CreateRasterizerState() override;
  std::unique_ptr<CCoreSamplerState> CreateSamplerState() override;

  bool SetRenderTarget(CCoreTexture2D* RT) override;

  //////////////////////////////////////////////////////////////////////////
  // display functions

  bool BeginScene() override;
  bool EndScene() override;
  bool Clear(const bool clearPixels = true, const bool clearDepth = true,
             const CColor& Color = CColor(), const float Depth = 1,
             const int32_t Stencil = 0) override;
  bool Flip(bool Vsync = true) override;
  bool DrawIndexedTriangles(int32_t Count, int32_t NumVertices) override;
  bool DrawTriangles(int32_t Count) override;
  bool DrawIndexedLines(int32_t Count, int32_t NumVertices) override;
  bool DrawLines(int32_t Count) override;

  //////////////////////////////////////////////////////////////////////////
  // renderstate functions

  bool SetViewport(CRect Viewport) override;
  ID3D11BlendState* GetCurrentBlendState();
  void SetCurrentBlendState(ID3D11BlendState* bs);
  ID3D11RasterizerState* GetCurrentRasterizerState();
  void SetCurrentRasterizerState(ID3D11RasterizerState* bs);
  ID3D11DepthStencilState* GetCurrentDepthStencilState();
  void SetCurrentDepthStencilState(ID3D11DepthStencilState* bs);

  void ForceStateReset() override;

  void TakeScreenShot(std::string_view Filename) override;

  void InitializeDebugAPI() override;
  void CaptureCurrentFrame() override;

  //////////////////////////////////////////////////////////////////////////
  // queries

  void BeginOcclusionQuery() override;
  bool EndOcclusionQuery() override;

  //////////////////////////////////////////////////////////////////////////
  // dx11 specific functions

  ID3D11DepthStencilView* GetDepthBufferView() { return DepthBufferView; }
};

#endif
