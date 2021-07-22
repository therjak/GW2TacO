#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "../BaseLib/BaseLib.h"
#include "../BaseLib/Color.h"
#include "../UtilLib/XMLDocument.h"
#include "ConstantBuffer.h"
#include "Enums.h"
#include "IndexBuffer.h"
#include "RenderState.h"
#include "VertexBuffer.h"
#include "VertexFormat.h"
#include "WindowHandler.h"

class CCoreDevice {
  //////////////////////////////////////////////////////////////////////////
  // resource management

  friend class CCoreResource;
  void RemoveResource(CCoreResource* Resource);
  void AddResource(CCoreResource* Resource);
  std::vector<CCoreResource*> Resources;

protected:
  //////////////////////////////////////////////////////////////////////////
  // device management

  CCoreWindowHandler* Window;

  void ResetDevice();
  virtual void ResetPrivateResources() = 0;
  virtual bool InitAPI(const uint32_t hWnd, const bool FullScreen,
                       const int32_t XRes, const int32_t YRes,
                       const int32_t AALevel = 0,
                       const int32_t RefreshRate = 60) = 0;

  //////////////////////////////////////////////////////////////////////////
  // renderstate management

  std::unordered_map<CORERENDERSTATEID, CORERENDERSTATEVALUE>
      CurrentRenderState;
  std::unordered_map<CORERENDERSTATEID, CORERENDERSTATEVALUE>
      RequestedRenderState;

  CCoreVertexBuffer *CurrentVertexBuffer, *RequestedVertexBuffer;
  uint32_t CurrentVertexBufferOffset, RequestedVertexBufferOffset;
  int32_t CurrentVertexFormatSize;

  std::unique_ptr<CCoreBlendState> DefaultBlendState;
  std::unique_ptr<CCoreDepthStencilState> DefaultDepthStencilState;
  std::unique_ptr<CCoreRasterizerState> DefaultRasterizerState;

  bool ApplyTextureToSampler(const CORESAMPLER Sampler, CCoreTexture* Texture);
  bool ApplyVertexShader(CCoreVertexShader* Shader);
  bool ApplyGeometryShader(CCoreGeometryShader* Shader);
  bool ApplyHullShader(CCoreHullShader* Shader);
  bool ApplyDomainShader(CCoreDomainShader* Shader);
  bool ApplyComputeShader(CCoreComputeShader* Shader);
  bool ApplyPixelShader(CCorePixelShader* Shader);
  bool ApplyVertexFormat(CCoreVertexFormat* Format);
  bool ApplyIndexBuffer(CCoreIndexBuffer* IdxBuffer);
  bool ApplyVertexBuffer(CCoreVertexBuffer* VxBuffer, uint32_t Offset);
  virtual bool ApplyRenderState(const CORESAMPLER Sampler,
                                const CORERENDERSTATE RenderState,
                                const CORERENDERSTATEVALUE Value) = 0;
  virtual bool SetNoVertexBuffer() = 0;

  virtual bool CommitRenderStates() = 0;
  bool CreateDefaultRenderStates();

public:
  CCoreDevice();
  virtual ~CCoreDevice();
  virtual COREDEVICEAPI GetAPIType() = 0;

  bool ApplyRequestedRenderState();

  // this initializer will change to accommodate multiple platforms at once once
  // we get to that point:
  virtual bool Initialize(CCoreWindowHandler* Window,
                          const int32_t AALevel = 0) = 0;

  virtual bool DeviceOk() = 0;
  virtual bool IsWindowed() = 0;
  virtual void Resize(const int32_t xr, const int32_t yr) = 0;
  virtual void SetFullScreenMode(const bool FullScreen, const int32_t xr,
                                 const int32_t yr) = 0;

  //////////////////////////////////////////////////////////////////////////
  // texture functions

  virtual std::unique_ptr<CCoreTexture2D>
  CreateTexture2D(const int32_t XRes, const int32_t YRes, const uint8_t* Data,
                  const char BytesPerPixel = 4,
                  const COREFORMAT Format = COREFMT_A8R8G8B8,
                  const bool RenderTarget = false) = 0;
  virtual std::unique_ptr<CCoreTexture2D>
  CreateTexture2D(const uint8_t* Data, const int32_t Size) = 0;
  virtual CCoreTexture2D* CopyTexture(CCoreTexture2D* Texture) = 0;

  //////////////////////////////////////////////////////////////////////////
  // vertexbuffer functions

  virtual std::unique_ptr<CCoreVertexBuffer>
  CreateVertexBuffer(const uint8_t* Data, const int32_t Size) = 0;
  virtual std::unique_ptr<CCoreVertexBuffer>
  CreateVertexBufferDynamic(const int32_t Size) = 0;

  //////////////////////////////////////////////////////////////////////////
  // indexbuffer functions

  virtual std::unique_ptr<CCoreIndexBuffer>
  CreateIndexBuffer(const int32_t IndexCount, const int32_t IndexSize = 2) = 0;

  //////////////////////////////////////////////////////////////////////////
  // vertexformat functions

  virtual std::unique_ptr<CCoreVertexFormat>
  CreateVertexFormat(const std::vector<COREVERTEXATTRIBUTE>& Attributes,
                     CCoreVertexShader* vs = nullptr) = 0;

  //////////////////////////////////////////////////////////////////////////
  // renderstate functions

  bool SetRenderState(CCoreRasterizerState* RasterizerState);
  bool SetRenderState(CCoreBlendState* BlendState);
  bool SetRenderState(CCoreDepthStencilState* DepthStencilState);
  bool SetVertexShader(CCoreVertexShader* Shader);
  bool SetPixelShader(CCorePixelShader* Shader);
  bool SetGeometryShader(CCoreGeometryShader* Shader);
  bool SetHullShader(CCoreHullShader* Shader);
  bool SetDomainShader(CCoreDomainShader* Shader);
  bool SetSamplerState(CORESAMPLER Sampler, CCoreSamplerState* SamplerState);
  bool SetTexture(CORESAMPLER Sampler, CCoreTexture* Texture);
  bool SetIndexBuffer(CCoreIndexBuffer* IndexBuffer);
  bool SetVertexBuffer(CCoreVertexBuffer* VertexBuffer, uint32_t Offset);
  bool SetVertexFormat(CCoreVertexFormat* VertexFormat);
  int32_t GetVertexFormatSize();
  CCoreTexture* GetTexture(CORESAMPLER Sampler);

  virtual bool SetRenderTarget(CCoreTexture2D* RT) = 0;
  virtual bool SetViewport(CRect Viewport) = 0;

  //////////////////////////////////////////////////////////////////////////
  // shader functions

  virtual std::unique_ptr<CCoreVertexShader>
  CreateVertexShader(LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction,
                     LPCSTR ShaderVersion, std::string* Err = nullptr) = 0;
  virtual std::unique_ptr<CCorePixelShader>
  CreatePixelShader(LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction,
                    LPCSTR ShaderVersion, std::string* Err = nullptr) = 0;
  virtual std::unique_ptr<CCoreVertexShader>
  CreateVertexShaderFromBlob(uint8_t* Code, int32_t CodeSize) = 0;
  virtual std::unique_ptr<CCorePixelShader>
  CreatePixelShaderFromBlob(uint8_t* Code, int32_t CodeSize) = 0;
  virtual CCoreGeometryShader*
  CreateGeometryShader(LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction,
                       LPCSTR ShaderVersion, std::string* Err = nullptr) = 0;
  virtual CCoreDomainShader* CreateDomainShader(LPCSTR Code, int32_t CodeSize,
                                                LPCSTR EntryFunction,
                                                LPCSTR ShaderVersion,
                                                std::string* Err = nullptr) = 0;
  virtual CCoreHullShader* CreateHullShader(LPCSTR Code, int32_t CodeSize,
                                            LPCSTR EntryFunction,
                                            LPCSTR ShaderVersion,
                                            std::string* Err = nullptr) = 0;
  virtual CCoreComputeShader*
  CreateComputeShader(LPCSTR Code, int32_t CodeSize, LPCSTR EntryFunction,
                      LPCSTR ShaderVersion, std::string* Err = nullptr) = 0;
  virtual CCoreVertexShader* CreateVertexShader() = 0;
  virtual CCorePixelShader* CreatePixelShader() = 0;
  virtual CCoreGeometryShader* CreateGeometryShader() = 0;
  virtual CCoreDomainShader* CreateDomainShader() = 0;
  virtual CCoreHullShader* CreateHullShader() = 0;
  virtual CCoreComputeShader* CreateComputeShader() = 0;
  virtual void SetShaderConstants(const CCoreConstantBuffer* Buffers) = 0;
  virtual std::unique_ptr<CCoreConstantBuffer> CreateConstantBuffer() = 0;

  virtual std::unique_ptr<CCoreBlendState> CreateBlendState() = 0;
  virtual std::unique_ptr<CCoreDepthStencilState> CreateDepthStencilState() = 0;
  virtual std::unique_ptr<CCoreRasterizerState> CreateRasterizerState() = 0;
  virtual std::unique_ptr<CCoreSamplerState> CreateSamplerState() = 0;

  //////////////////////////////////////////////////////////////////////////
  // display functions

  virtual bool BeginScene() = 0;
  virtual bool EndScene() = 0;
  virtual bool Clear(const bool clearPixels = true,
                     const bool clearDepth = true,
                     const CColor& Color = CColor(static_cast<uint32_t>(0)),
                     const float Depth = 1, const int32_t Stencil = 0) = 0;
  virtual bool Flip(bool Vsync = true) = 0;
  virtual bool DrawIndexedTriangles(int32_t Count, int32_t NumVertices) = 0;
  virtual bool DrawIndexedLines(int32_t Count, int32_t NumVertices) = 0;
  virtual bool DrawTriangles(int32_t Count) = 0;
  virtual bool DrawLines(int32_t Count) = 0;

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
  virtual bool EndOcclusionQuery() = 0;
};
