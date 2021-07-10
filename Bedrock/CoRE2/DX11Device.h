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

	virtual void ResetPrivateResources();
	virtual TBOOL InitAPI(const uint32_t hWnd, const TBOOL FullScreen, const int32_t XRes, const int32_t YRes, const int32_t AALevel = 0, const int32_t RefreshRate = 60);
	virtual TBOOL ApplyRenderState(const CORESAMPLER Sampler, const CORERENDERSTATE RenderState, const CORERENDERSTATEVALUE Value);
	virtual TBOOL SetNoVertexBuffer();
	virtual TBOOL CommitRenderStates();

	virtual TBOOL CreateBackBuffer(int32_t XRes, int32_t YRes);
	virtual TBOOL CreateDepthBuffer(int32_t XRes, int32_t YRes);

	TBOOL CreateClassicSwapChain( const uint32_t hWnd, const TBOOL FullScreen, const int32_t XRes, const int32_t YRes, const int32_t AALevel, const int32_t RefreshRate );
	TBOOL CreateDirectCompositionSwapchain( const uint32_t hWnd, const TBOOL FullScreen, const int32_t XRes, const int32_t YRes, const int32_t AALevel, const int32_t RefreshRate );

public:

	CCoreDX11Device();
	virtual ~CCoreDX11Device();
	INLINE ID3D11Device *GetDevice() { return Device; }
	INLINE ID3D11DeviceContext *GetDeviceContext() { return DeviceContext; }
	virtual COREDEVICEAPI GetAPIType() { return COREAPI_DX11; }

	//this initializer will change to accommodate multiple platforms at once once we get to that point:
	TBOOL Initialize(CCoreWindowHandler *Window, const int32_t AALevel = 0);

	virtual TBOOL DeviceOk();
	virtual TBOOL IsWindowed();
	virtual void Resize(const int32_t xr, const int32_t yr);
	virtual void SetFullScreenMode(const TBOOL FullScreen, const int32_t xr, const int32_t yr);

	ID3D11Texture2D *GetBackBuffer();

	//////////////////////////////////////////////////////////////////////////
	// texture functions

	virtual std::unique_ptr<CCoreTexture2D> CreateTexture2D(const int32_t XRes, const int32_t YRes, const uint8_t *Data, const TS8 BytesPerPixel = 4, const COREFORMAT Format = COREFMT_A8R8G8B8, const TBOOL RenderTarget = false);
	virtual std::unique_ptr<CCoreTexture2D> CreateTexture2D(const uint8_t *Data, const int32_t Size);
	virtual CCoreTexture2D *CopyTexture(CCoreTexture2D *Texture);

	//////////////////////////////////////////////////////////////////////////
	// vertexbuffer functions

	virtual std::unique_ptr<CCoreVertexBuffer> CreateVertexBuffer(const uint8_t *Data, const int32_t Size);
	virtual std::unique_ptr<CCoreVertexBuffer> CreateVertexBufferDynamic(const int32_t Size);

	//////////////////////////////////////////////////////////////////////////
	// indexbuffer functions

	virtual std::unique_ptr<CCoreIndexBuffer> CreateIndexBuffer(const int32_t IndexCount, const int32_t IndexSize = 2);

	//////////////////////////////////////////////////////////////////////////
	// vertexformat functions

	virtual std::unique_ptr<CCoreVertexFormat> CreateVertexFormat(const CArray<COREVERTEXATTRIBUTE> &Attributes, CCoreVertexShader *vs = NULL);

	//////////////////////////////////////////////////////////////////////////
	// shader functions

	virtual std::unique_ptr<CCoreVertexShader> CreateVertexShader(
            LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction,
            LPCSTR ShaderVersion, std::string *Err = NULL);
        virtual std::unique_ptr<CCorePixelShader> CreatePixelShader(
            LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction,
            LPCSTR ShaderVersion, std::string *Err = NULL);
  virtual std::unique_ptr<CCoreVertexShader> CreateVertexShaderFromBlob( uint8_t *Code, int32_t CodeSize );
  virtual std::unique_ptr<CCorePixelShader> CreatePixelShaderFromBlob( uint8_t *Code, int32_t CodeSize );
  virtual CCoreGeometryShader *CreateGeometryShader(LPCSTR Code,
                                                    int32_t CodeSize,
                                                    LPCSTR EntryFunction,
                                                    LPCSTR ShaderVersion,
                                                    std::string *Err = NULL);
  virtual CCoreDomainShader *CreateDomainShader(LPCSTR Code, int32_t CodeSize,
                                                LPCSTR EntryFunction,
                                                LPCSTR ShaderVersion,
                                                std::string *Err = NULL);
  virtual CCoreHullShader *CreateHullShader(LPCSTR Code, int32_t CodeSize,
                                            LPCSTR EntryFunction,
                                            LPCSTR ShaderVersion,
                                            std::string *Err = NULL);
  virtual CCoreComputeShader *CreateComputeShader(LPCSTR Code, int32_t CodeSize,
                                                  LPCSTR EntryFunction,
                                                  LPCSTR ShaderVersion,
                                                  std::string *Err = NULL);
	virtual CCoreVertexShader *CreateVertexShader();
	virtual CCorePixelShader *CreatePixelShader();
	virtual CCoreGeometryShader *CreateGeometryShader();
	virtual CCoreDomainShader *CreateDomainShader();
	virtual CCoreHullShader *CreateHullShader();
	virtual CCoreComputeShader *CreateComputeShader();
	virtual void SetShaderConstants(const CCoreConstantBuffer* Buffers);
	virtual std::unique_ptr<CCoreConstantBuffer> CreateConstantBuffer();

	virtual std::unique_ptr<CCoreBlendState> CreateBlendState();
	virtual std::unique_ptr<CCoreDepthStencilState> CreateDepthStencilState();
	virtual std::unique_ptr<CCoreRasterizerState> CreateRasterizerState();
	virtual std::unique_ptr<CCoreSamplerState> CreateSamplerState();

	virtual TBOOL SetRenderTarget(CCoreTexture2D *RT);

	//////////////////////////////////////////////////////////////////////////
	// display functions

	virtual TBOOL BeginScene();
	virtual TBOOL EndScene();
	virtual TBOOL Clear(const TBOOL clearPixels = true, const TBOOL clearDepth = true, const CColor &Color = CColor((uint32_t)0), const float Depth = 1, const int32_t Stencil = 0);
	virtual TBOOL Flip(TBOOL Vsync = true);
	virtual TBOOL DrawIndexedTriangles(int32_t Count, int32_t NumVertices);
	virtual TBOOL DrawTriangles(int32_t Count);
	virtual TBOOL DrawIndexedLines(int32_t Count, int32_t NumVertices);
	virtual TBOOL DrawLines(int32_t Count);

	//////////////////////////////////////////////////////////////////////////
	// renderstate functions

	virtual TBOOL SetViewport(CRect Viewport);
	ID3D11BlendState *GetCurrentBlendState();
	void SetCurrentBlendState(ID3D11BlendState *bs);
	ID3D11RasterizerState *GetCurrentRasterizerState();
	void SetCurrentRasterizerState(ID3D11RasterizerState *bs);
	ID3D11DepthStencilState *GetCurrentDepthStencilState();
	void SetCurrentDepthStencilState(ID3D11DepthStencilState *bs);

	virtual void ForceStateReset();

	virtual void TakeScreenShot(std::string_view Filename);

	virtual void InitializeDebugAPI();
	virtual void CaptureCurrentFrame();

  //////////////////////////////////////////////////////////////////////////
  // queries

  virtual void BeginOcclusionQuery();
  virtual TBOOL EndOcclusionQuery();
  
  //////////////////////////////////////////////////////////////////////////
	// dx11 specific functions

	ID3D11DepthStencilView *GetDepthBufferView() { return DepthBufferView; }
};

#endif
