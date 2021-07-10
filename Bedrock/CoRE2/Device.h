#pragma once
#include "../BaseLib/BaseLib.h"
#include "../UtilLib/XMLDocument.h"
#include "WindowHandler.h"

#include "Enums.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexFormat.h"
#include "RenderState.h"
#include "ConstantBuffer.h"

#include <vector>
#include <memory>

class CCoreDevice
{
	//////////////////////////////////////////////////////////////////////////
	// resource management

	friend class CCoreResource;
	void RemoveResource(CCoreResource *Resource);
	void AddResource(CCoreResource *Resource);
	std::vector<CCoreResource*> Resources;

protected:

	//////////////////////////////////////////////////////////////////////////
	// device management

	CCoreWindowHandler *Window;

	void ResetDevice();
	virtual void ResetPrivateResources() = 0;
	virtual TBOOL InitAPI(const uint32_t hWnd, const TBOOL FullScreen, const int32_t XRes, const int32_t YRes, const int32_t AALevel = 0, const int32_t RefreshRate = 60) = 0;

	//////////////////////////////////////////////////////////////////////////
	// renderstate management

	CDictionaryEnumerable<CORERENDERSTATEID, CORERENDERSTATEVALUE> CurrentRenderState;
	CDictionaryEnumerable<CORERENDERSTATEID, CORERENDERSTATEVALUE> RequestedRenderState;

	CCoreVertexBuffer *CurrentVertexBuffer, *RequestedVertexBuffer;
	uint32_t CurrentVertexBufferOffset, RequestedVertexBufferOffset;
	int32_t CurrentVertexFormatSize;

	std::unique_ptr<CCoreBlendState> DefaultBlendState;
	std::unique_ptr<CCoreDepthStencilState> DefaultDepthStencilState;
	std::unique_ptr<CCoreRasterizerState> DefaultRasterizerState;

	TBOOL ApplyTextureToSampler(const CORESAMPLER Sampler, CCoreTexture *Texture);
	TBOOL ApplyVertexShader(CCoreVertexShader *Shader);
	TBOOL ApplyGeometryShader(CCoreGeometryShader *Shader);
	TBOOL ApplyHullShader(CCoreHullShader *Shader);
	TBOOL ApplyDomainShader(CCoreDomainShader *Shader);
	TBOOL ApplyComputeShader(CCoreComputeShader *Shader);
	TBOOL ApplyPixelShader(CCorePixelShader *Shader);
	TBOOL ApplyVertexFormat(CCoreVertexFormat *Format);
	TBOOL ApplyIndexBuffer(CCoreIndexBuffer *IdxBuffer);
	TBOOL ApplyVertexBuffer(CCoreVertexBuffer *VxBuffer, uint32_t Offset);
	virtual TBOOL ApplyRenderState(const CORESAMPLER Sampler, const CORERENDERSTATE RenderState, const CORERENDERSTATEVALUE Value) = 0;
	virtual TBOOL SetNoVertexBuffer() = 0;

	virtual TBOOL CommitRenderStates() = 0;
	TBOOL CreateDefaultRenderStates();

public:

	CCoreDevice();
	virtual ~CCoreDevice();
	virtual COREDEVICEAPI GetAPIType() = 0;

  TBOOL ApplyRequestedRenderState();

	//this initializer will change to accommodate multiple platforms at once once we get to that point:
	virtual TBOOL Initialize(CCoreWindowHandler *Window, const int32_t AALevel = 0) = 0;

	virtual TBOOL DeviceOk() = 0;
	virtual TBOOL IsWindowed() = 0;
	virtual void Resize(const int32_t xr, const int32_t yr) = 0;
	virtual void SetFullScreenMode(const TBOOL FullScreen, const int32_t xr, const int32_t yr) = 0;

	//////////////////////////////////////////////////////////////////////////
	// texture functions

	virtual std::unique_ptr<CCoreTexture2D> CreateTexture2D(const int32_t XRes, const int32_t YRes, const uint8_t *Data, const TS8 BytesPerPixel = 4, const COREFORMAT Format = COREFMT_A8R8G8B8, const TBOOL RenderTarget = false) = 0;
	virtual std::unique_ptr<CCoreTexture2D> CreateTexture2D(const uint8_t *Data, const int32_t Size) = 0;
	virtual TBOOL DestroyTexture(CCoreTexture *Texture) const;
	virtual CCoreTexture2D *CopyTexture(CCoreTexture2D *Texture) = 0;

	//////////////////////////////////////////////////////////////////////////
	// vertexbuffer functions

	virtual std::unique_ptr<CCoreVertexBuffer> CreateVertexBuffer(const uint8_t *Data, const int32_t Size) = 0;
	virtual std::unique_ptr<CCoreVertexBuffer> CreateVertexBufferDynamic(const int32_t Size) = 0;
	virtual TBOOL DestroyVertexBuffer(CCoreVertexBuffer *VertexBuffer) const;

	//////////////////////////////////////////////////////////////////////////
	// indexbuffer functions

	virtual std::unique_ptr<CCoreIndexBuffer> CreateIndexBuffer(const int32_t IndexCount, const int32_t IndexSize = 2) = 0;
	virtual TBOOL DestroyIndexBuffer(CCoreIndexBuffer *IndexBuffer) const;

	//////////////////////////////////////////////////////////////////////////
	// vertexformat functions

	virtual std::unique_ptr<CCoreVertexFormat> CreateVertexFormat(const CArray<COREVERTEXATTRIBUTE> &Attributes, CCoreVertexShader *vs = NULL) = 0;
	virtual TBOOL DestroyVertexFormat(CCoreVertexFormat *VertexFormat) const;

	//////////////////////////////////////////////////////////////////////////
	// renderstate functions

	TBOOL SetRenderState(CCoreRasterizerState *RasterizerState);
	TBOOL SetRenderState(CCoreBlendState *BlendState);
	TBOOL SetRenderState(CCoreDepthStencilState *DepthStencilState);
	TBOOL SetVertexShader(CCoreVertexShader *Shader);
	TBOOL SetPixelShader(CCorePixelShader *Shader);
	TBOOL SetGeometryShader(CCoreGeometryShader *Shader);
	TBOOL SetHullShader(CCoreHullShader *Shader);
	TBOOL SetDomainShader(CCoreDomainShader *Shader);
	TBOOL SetSamplerState(CORESAMPLER Sampler, CCoreSamplerState *SamplerState);
	TBOOL SetTexture(CORESAMPLER Sampler, CCoreTexture *Texture);
	TBOOL SetIndexBuffer(CCoreIndexBuffer *IndexBuffer);
	TBOOL SetVertexBuffer(CCoreVertexBuffer *VertexBuffer, uint32_t Offset);
	TBOOL SetVertexFormat(CCoreVertexFormat *VertexFormat);
	int32_t GetVertexFormatSize();
	CCoreTexture *GetTexture(CORESAMPLER Sampler);

	virtual TBOOL SetRenderTarget(CCoreTexture2D *RT) = 0;
	virtual TBOOL SetViewport(CRect Viewport) = 0;

	//////////////////////////////////////////////////////////////////////////
	// shader functions

	virtual std::unique_ptr<CCoreVertexShader> CreateVertexShader(
            LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction,
            LPCSTR ShaderVersion, std::string *Err = NULL) = 0;
        virtual std::unique_ptr<CCorePixelShader> CreatePixelShader(
            LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction,
            LPCSTR ShaderVersion, std::string *Err = NULL) = 0;
  virtual std::unique_ptr<CCoreVertexShader> CreateVertexShaderFromBlob( uint8_t *Code, int32_t CodeSize ) = 0;
  virtual std::unique_ptr<CCorePixelShader> CreatePixelShaderFromBlob( uint8_t *Code, int32_t CodeSize ) = 0;
  virtual CCoreGeometryShader *CreateGeometryShader(
      LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion,
      std::string *Err = NULL) = 0;
  virtual CCoreDomainShader *CreateDomainShader(LPCSTR Code, int32_t CodeSize,
                                                LPCSTR EntryFunction,
                                                LPCSTR ShaderVersion,
                                                std::string *Err = NULL) = 0;
  virtual CCoreHullShader *CreateHullShader(LPCSTR Code, int32_t CodeSize,
                                            LPCSTR EntryFunction,
                                            LPCSTR ShaderVersion,
                                            std::string *Err = NULL) = 0;
  virtual CCoreComputeShader *CreateComputeShader(LPCSTR Code, int32_t CodeSize,
                                                  LPCSTR EntryFunction,
                                                  LPCSTR ShaderVersion,
                                                  std::string *Err = NULL) = 0;
	virtual CCoreVertexShader *CreateVertexShader() = 0;
	virtual CCorePixelShader *CreatePixelShader() = 0;
	virtual CCoreGeometryShader *CreateGeometryShader() = 0;
	virtual CCoreDomainShader *CreateDomainShader() = 0;
	virtual CCoreHullShader *CreateHullShader() = 0;
	virtual CCoreComputeShader *CreateComputeShader() = 0;
	virtual void SetShaderConstants(const CCoreConstantBuffer* Buffers) = 0;
	virtual std::unique_ptr<CCoreConstantBuffer> CreateConstantBuffer() = 0;

	virtual std::unique_ptr<CCoreBlendState> CreateBlendState() = 0;
	virtual std::unique_ptr<CCoreDepthStencilState> CreateDepthStencilState() = 0;
	virtual std::unique_ptr<CCoreRasterizerState> CreateRasterizerState() = 0;
	virtual std::unique_ptr<CCoreSamplerState> CreateSamplerState() = 0;

	//////////////////////////////////////////////////////////////////////////
	// display functions

	virtual TBOOL BeginScene() = 0;
	virtual TBOOL EndScene() = 0;
	virtual TBOOL Clear(const TBOOL clearPixels = true, const TBOOL clearDepth = true, const CColor &Color = CColor((uint32_t)0), const float Depth = 1, const int32_t Stencil = 0) = 0;
	virtual TBOOL Flip(TBOOL Vsync = true) = 0;
	virtual TBOOL DrawIndexedTriangles(int32_t Count, int32_t NumVertices) = 0;
	virtual TBOOL DrawIndexedLines(int32_t Count, int32_t NumVertices) = 0;
	virtual TBOOL DrawTriangles(int32_t Count) = 0;
	virtual TBOOL DrawLines(int32_t Count) = 0;

	//////////////////////////////////////////////////////////////////////////
	// material import functions

	virtual void ForceStateReset() = 0;

	virtual void TakeScreenShot(std::string_view Filename) = 0;

	virtual void InitializeDebugAPI() = 0;
	virtual void CaptureCurrentFrame() = 0;


	virtual float GetUVOffset() { return 0; }

  //////////////////////////////////////////////////////////////////////////
  // queries

  virtual void BeginOcclusionQuery() = 0;
  virtual TBOOL EndOcclusionQuery() = 0;

};
