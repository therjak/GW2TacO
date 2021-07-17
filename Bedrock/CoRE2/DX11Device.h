#pragma once
#include "Core2_Config.h"
#include "../BaseLib/BaseLib.h"
#include "Device.h"

#include "DX11Enums.h"
#include <dxgi1_3.h>

#ifdef CORE_API_DX11

class CCoreDX11Device : public CCoreDevice
{
	IDXGISwapChain1 *SwapChain;
	ID3D11Device *Device;
	ID3D11DeviceContext *DeviceContext;

	ID3D11RenderTargetView *BackBufferView;
	ID3D11Texture2D* DepthBuffer;
	ID3D11DepthStencilView *DepthBufferView;

	ID3D11BlendState *CurrentBlendState;
	ID3D11DepthStencilState *CurrentDepthStencilState;
	ID3D11RasterizerState *CurrentRasterizerState;

  ID3D11Query *OcclusionQuery = nullptr;

  void ResetPrivateResources() override;
  TBOOL InitAPI(const uint32_t hWnd, const TBOOL FullScreen, const int32_t XRes,
                const int32_t YRes, const int32_t AALevel = 0,
                const int32_t RefreshRate = 60) override;
  TBOOL ApplyRenderState(const CORESAMPLER Sampler,
                         const CORERENDERSTATE RenderState,
                         const CORERENDERSTATEVALUE Value) override;
  TBOOL SetNoVertexBuffer() override;
  TBOOL CommitRenderStates() override;

  virtual TBOOL CreateBackBuffer(int32_t XRes, int32_t YRes);
  virtual TBOOL CreateDepthBuffer(int32_t XRes, int32_t YRes);

  TBOOL CreateClassicSwapChain(const uint32_t hWnd, const TBOOL FullScreen,
                               const int32_t XRes, const int32_t YRes,
                               const int32_t AALevel,
                               const int32_t RefreshRate);
  TBOOL CreateDirectCompositionSwapchain(const uint32_t hWnd,
                                         const TBOOL FullScreen,
                                         const int32_t XRes, const int32_t YRes,
                                         const int32_t AALevel,
                                         const int32_t RefreshRate);

 public:
  CCoreDX11Device();
  ~CCoreDX11Device() override;
  INLINE ID3D11Device *GetDevice() { return Device; }
  INLINE ID3D11DeviceContext *GetDeviceContext() { return DeviceContext; }
  COREDEVICEAPI GetAPIType() override { return COREAPI_DX11; }

  // this initializer will change to accommodate multiple platforms at once once
  // we get to that point:
  TBOOL Initialize(CCoreWindowHandler *Window,
                   const int32_t AALevel = 0) override;

  TBOOL DeviceOk() override;
  TBOOL IsWindowed() override;
  void Resize(const int32_t xr, const int32_t yr) override;
  void SetFullScreenMode(const TBOOL FullScreen, const int32_t xr,
                         const int32_t yr) override;

  ID3D11Texture2D *GetBackBuffer();

  //////////////////////////////////////////////////////////////////////////
  // texture functions

  std::unique_ptr<CCoreTexture2D> CreateTexture2D(
      const int32_t XRes, const int32_t YRes, const uint8_t *Data,
      const TS8 BytesPerPixel = 4, const COREFORMAT Format = COREFMT_A8R8G8B8,
      const TBOOL RenderTarget = false) override;
  std::unique_ptr<CCoreTexture2D> CreateTexture2D(const uint8_t *Data,
                                                  const int32_t Size) override;
  CCoreTexture2D *CopyTexture(CCoreTexture2D *Texture) override;

  //////////////////////////////////////////////////////////////////////////
  // vertexbuffer functions

  std::unique_ptr<CCoreVertexBuffer> CreateVertexBuffer(
      const uint8_t *Data, const int32_t Size) override;
  std::unique_ptr<CCoreVertexBuffer> CreateVertexBufferDynamic(
      const int32_t Size) override;

  //////////////////////////////////////////////////////////////////////////
  // indexbuffer functions

  std::unique_ptr<CCoreIndexBuffer> CreateIndexBuffer(
      const int32_t IndexCount, const int32_t IndexSize = 2) override;

  //////////////////////////////////////////////////////////////////////////
  // vertexformat functions

  std::unique_ptr<CCoreVertexFormat> CreateVertexFormat(
      const CArray<COREVERTEXATTRIBUTE> &Attributes,
      CCoreVertexShader *vs = NULL) override;

  //////////////////////////////////////////////////////////////////////////
  // shader functions

  std::unique_ptr<CCoreVertexShader> CreateVertexShader(
      LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
      std::string *Err = NULL) override;
  std::unique_ptr<CCorePixelShader> CreatePixelShader(
      LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
      std::string *Err = NULL) override;
  std::unique_ptr<CCoreVertexShader> CreateVertexShaderFromBlob(
      uint8_t *Code, int32_t CodeSize) override;
  std::unique_ptr<CCorePixelShader> CreatePixelShaderFromBlob(
      uint8_t *Code, int32_t CodeSize) override;
  CCoreGeometryShader *CreateGeometryShader(LPCSTR Code, int32_t CodeSize,
                                            LPCSTR EntryFunction,
                                            LPCSTR ShaderVersion,
                                            std::string *Err = NULL) override;
  CCoreDomainShader *CreateDomainShader(LPCSTR Code, int32_t CodeSize,
                                        LPCSTR EntryFunction,
                                        LPCSTR ShaderVersion,
                                        std::string *Err = NULL) override;
  CCoreHullShader *CreateHullShader(LPCSTR Code, int32_t CodeSize,
                                    LPCSTR EntryFunction, LPCSTR ShaderVersion,
                                    std::string *Err = NULL) override;
  CCoreComputeShader *CreateComputeShader(LPCSTR Code, int32_t CodeSize,
                                          LPCSTR EntryFunction,
                                          LPCSTR ShaderVersion,
                                          std::string *Err = NULL) override;
  CCoreVertexShader *CreateVertexShader() override;
  CCorePixelShader *CreatePixelShader() override;
  CCoreGeometryShader *CreateGeometryShader() override;
  CCoreDomainShader *CreateDomainShader() override;
  CCoreHullShader *CreateHullShader() override;
  CCoreComputeShader *CreateComputeShader() override;
  void SetShaderConstants(const CCoreConstantBuffer *Buffers) override;
  std::unique_ptr<CCoreConstantBuffer> CreateConstantBuffer() override;

  std::unique_ptr<CCoreBlendState> CreateBlendState() override;
  std::unique_ptr<CCoreDepthStencilState> CreateDepthStencilState() override;
  std::unique_ptr<CCoreRasterizerState> CreateRasterizerState() override;
  std::unique_ptr<CCoreSamplerState> CreateSamplerState() override;

  TBOOL SetRenderTarget(CCoreTexture2D *RT) override;

  //////////////////////////////////////////////////////////////////////////
  // display functions

  TBOOL BeginScene() override;
  TBOOL EndScene() override;
  TBOOL Clear(const TBOOL clearPixels = true, const TBOOL clearDepth = true,
              const CColor &Color = CColor((uint32_t)0), const float Depth = 1,
              const int32_t Stencil = 0) override;
  TBOOL Flip(TBOOL Vsync = true) override;
  TBOOL DrawIndexedTriangles(int32_t Count, int32_t NumVertices) override;
  TBOOL DrawTriangles(int32_t Count) override;
  TBOOL DrawIndexedLines(int32_t Count, int32_t NumVertices) override;
  TBOOL DrawLines(int32_t Count) override;

  //////////////////////////////////////////////////////////////////////////
  // renderstate functions

  TBOOL SetViewport(CRect Viewport) override;
  ID3D11BlendState *GetCurrentBlendState();
  void SetCurrentBlendState(ID3D11BlendState *bs);
  ID3D11RasterizerState *GetCurrentRasterizerState();
  void SetCurrentRasterizerState(ID3D11RasterizerState *bs);
  ID3D11DepthStencilState *GetCurrentDepthStencilState();
  void SetCurrentDepthStencilState(ID3D11DepthStencilState *bs);

  void ForceStateReset() override;

  void TakeScreenShot(std::string_view Filename) override;

  void InitializeDebugAPI() override;
  void CaptureCurrentFrame() override;

  //////////////////////////////////////////////////////////////////////////
  // queries

  void BeginOcclusionQuery() override;
  TBOOL EndOcclusionQuery() override;

  //////////////////////////////////////////////////////////////////////////
	// dx11 specific functions

	ID3D11DepthStencilView *GetDepthBufferView() { return DepthBufferView; }
};

#endif
