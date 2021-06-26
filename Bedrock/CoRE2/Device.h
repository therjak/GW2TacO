#pragma once
#include "../BaseLib/BaseLib.h"
#include "../UtilLib/XMLDocument.h"
#include "WindowHandler.h"

#include "Enums.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexFormat.h"
#include "RenderState.h"

#include "RenderLayer.h"
#include "Material.h"

class CCoreDevice
{
	//////////////////////////////////////////////////////////////////////////
	// resource management

	friend class CCoreResource;
	void RemoveResource(CCoreResource *Resource);
	void AddResource(CCoreResource *Resource);
	CArray<CCoreResource*> Resources;

	CArray<CCoreRenderLayerDescriptor*> RenderLayers;
	CArray<CCoreMaterialTechnique*> TechPool;

protected:

	//////////////////////////////////////////////////////////////////////////
	// device management

	CCoreWindowHandler *Window;

	void ResetDevice();
	virtual void ResetPrivateResources() = 0;
	virtual TBOOL InitAPI(const TU32 hWnd, const TBOOL FullScreen, const int32_t XRes, const int32_t YRes, const int32_t AALevel = 0, const int32_t RefreshRate = 60) = 0;

	//////////////////////////////////////////////////////////////////////////
	// renderstate management

	CDictionaryEnumerable<CORERENDERSTATEID, CORERENDERSTATEVALUE> CurrentRenderState;
	CDictionaryEnumerable<CORERENDERSTATEID, CORERENDERSTATEVALUE> RequestedRenderState;

	CCoreVertexBuffer *CurrentVertexBuffer, *RequestedVertexBuffer;
	TU32 CurrentVertexBufferOffset, RequestedVertexBufferOffset;
	int32_t CurrentVertexFormatSize;

	CCoreBlendState *DefaultBlendState;
	CCoreDepthStencilState *DefaultDepthStencilState;
	CCoreRasterizerState *DefaultRasterizerState;

	TBOOL ApplyTextureToSampler(const CORESAMPLER Sampler, CCoreTexture *Texture);
	TBOOL ApplyVertexShader(CCoreVertexShader *Shader);
	TBOOL ApplyGeometryShader(CCoreGeometryShader *Shader);
	TBOOL ApplyHullShader(CCoreHullShader *Shader);
	TBOOL ApplyDomainShader(CCoreDomainShader *Shader);
	TBOOL ApplyComputeShader(CCoreComputeShader *Shader);
	TBOOL ApplyPixelShader(CCorePixelShader *Shader);
	TBOOL ApplyVertexFormat(CCoreVertexFormat *Format);
	TBOOL ApplyIndexBuffer(CCoreIndexBuffer *IdxBuffer);
	TBOOL ApplyVertexBuffer(CCoreVertexBuffer *VxBuffer, TU32 Offset);
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

	void AddRenderLayer(CCoreRenderLayerDescriptor *Desc);
	CCoreRenderLayerDescriptor *GetRenderLayer(CString &Name);
	void AddMaterialTechnique(CCoreMaterialTechnique *Tech);
	CCoreMaterialTechnique *GetMaterialTechnique(CString &Name);

	//////////////////////////////////////////////////////////////////////////
	// texture functions

	virtual CCoreTexture2D *CreateTexture2D(const int32_t XRes, const int32_t YRes, const TU8 *Data, const TS8 BytesPerPixel = 4, const COREFORMAT Format = COREFMT_A8R8G8B8, const TBOOL RenderTarget = false) = 0;
	virtual CCoreTexture2D *CreateTexture2D(const TU8 *Data, const int32_t Size) = 0;
	virtual TBOOL DestroyTexture(CCoreTexture *Texture) const;
	virtual CCoreTexture2D *CopyTexture(CCoreTexture2D *Texture) = 0;

	//////////////////////////////////////////////////////////////////////////
	// vertexbuffer functions

	virtual CCoreVertexBuffer *CreateVertexBuffer(const TU8 *Data, const int32_t Size) = 0;
	virtual CCoreVertexBuffer *CreateVertexBufferDynamic(const int32_t Size) = 0;
	virtual TBOOL DestroyVertexBuffer(CCoreVertexBuffer *VertexBuffer) const;

	//////////////////////////////////////////////////////////////////////////
	// indexbuffer functions

	virtual CCoreIndexBuffer *CreateIndexBuffer(const int32_t IndexCount, const int32_t IndexSize = 2) = 0;
	virtual TBOOL DestroyIndexBuffer(CCoreIndexBuffer *IndexBuffer) const;

	//////////////////////////////////////////////////////////////////////////
	// vertexformat functions

	virtual CCoreVertexFormat *CreateVertexFormat(const CArray<COREVERTEXATTRIBUTE> &Attributes, CCoreVertexShader *vs = NULL) = 0;
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
	TBOOL SetComputeShader(CCoreComputeShader *Shader);
	TBOOL SetSamplerState(CORESAMPLER Sampler, CCoreSamplerState *SamplerState);
	TBOOL SetTexture(CORESAMPLER Sampler, CCoreTexture *Texture);
	TBOOL SetIndexBuffer(CCoreIndexBuffer *IndexBuffer);
	TBOOL SetVertexBuffer(CCoreVertexBuffer *VertexBuffer, TU32 Offset);
	TBOOL SetVertexFormat(CCoreVertexFormat *VertexFormat);
	int32_t GetVertexFormatSize();
	CCoreTexture *GetTexture(CORESAMPLER Sampler);

	virtual TBOOL SetRenderTarget(CCoreTexture2D *RT) = 0;
	virtual TBOOL SetViewport(CRect Viewport) = 0;

	//////////////////////////////////////////////////////////////////////////
	// shader functions

	virtual CCoreVertexShader *CreateVertexShader(LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL) = 0;
	virtual CCorePixelShader *CreatePixelShader(LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL) = 0;
  virtual CCoreVertexShader *CreateVertexShaderFromBlob( TU8 *Code, int32_t CodeSize ) = 0;
  virtual CCorePixelShader *CreatePixelShaderFromBlob( TU8 *Code, int32_t CodeSize ) = 0;
  virtual CCoreGeometryShader *CreateGeometryShader( LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL ) = 0;
	virtual CCoreDomainShader *CreateDomainShader(LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL) = 0;
	virtual CCoreHullShader *CreateHullShader(LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL) = 0;
	virtual CCoreComputeShader *CreateComputeShader(LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction, LPCSTR ShaderVersion, CString *Err = NULL) = 0;
	virtual CCoreVertexShader *CreateVertexShader() = 0;
	virtual CCorePixelShader *CreatePixelShader() = 0;
	virtual CCoreGeometryShader *CreateGeometryShader() = 0;
	virtual CCoreDomainShader *CreateDomainShader() = 0;
	virtual CCoreHullShader *CreateHullShader() = 0;
	virtual CCoreComputeShader *CreateComputeShader() = 0;
	virtual void SetShaderConstants(int32_t Slot, int32_t Count, CCoreConstantBuffer **Buffers) = 0;
	virtual CCoreConstantBuffer *CreateConstantBuffer() = 0;

	virtual CCoreBlendState *CreateBlendState() = 0;
	virtual CCoreDepthStencilState *CreateDepthStencilState() = 0;
	virtual CCoreRasterizerState *CreateRasterizerState() = 0;
	virtual CCoreSamplerState *CreateSamplerState() = 0;

	//////////////////////////////////////////////////////////////////////////
	// display functions

	virtual TBOOL BeginScene() = 0;
	virtual TBOOL EndScene() = 0;
	virtual TBOOL Clear(const TBOOL clearPixels = true, const TBOOL clearDepth = true, const CColor &Color = CColor((TU32)0), const TF32 Depth = 1, const int32_t Stencil = 0) = 0;
	virtual TBOOL Flip(TBOOL Vsync = true) = 0;
	virtual TBOOL DrawIndexedTriangles(int32_t Count, int32_t NumVertices) = 0;
	virtual TBOOL DrawIndexedLines(int32_t Count, int32_t NumVertices) = 0;
	virtual TBOOL DrawTriangles(int32_t Count) = 0;
	virtual TBOOL DrawLines(int32_t Count) = 0;

	//////////////////////////////////////////////////////////////////////////
	// material import functions

	TBOOL ImportMaterialTechnique(CXMLNode *Root);

	virtual void ForceStateReset() = 0;

	virtual void TakeScreenShot(CString Filename) = 0;

	virtual void InitializeDebugAPI() = 0;
	virtual void CaptureCurrentFrame() = 0;


	virtual TF32 GetUVOffset() { return 0; }

  //////////////////////////////////////////////////////////////////////////
  // queries

  virtual void BeginOcclusionQuery() = 0;
  virtual TBOOL EndOcclusionQuery() = 0;

};
