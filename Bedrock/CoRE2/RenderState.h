#pragma once
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexFormat.h"

class CCoreSamplerState;
class CCoreDepthStencilState;
class CCoreBlendState;
class CCoreRasterizerState;
class CXMLNode;

union CORERENDERSTATEVALUE {
  CCoreSamplerState* SamplerState;
  CCoreDepthStencilState* DepthStencilState;
  CCoreBlendState* BlendState;
  CCoreRasterizerState* RasterizerState;
  CCoreTexture* Texture;
  CCoreIndexBuffer* IndexBuffer;
  CCoreVertexFormat* VertexFormat;
  CCoreVertexShader* VertexShader;
  CCorePixelShader* PixelShader;
  CCoreGeometryShader* GeometryShader;
  CCoreDomainShader* DomainShader;
  CCoreComputeShader* ComputeShader;
  CCoreHullShader* HullShader;

  INLINE const bool operator!=(const CORERENDERSTATEVALUE& v) {
    return SamplerState != v.SamplerState;
  }
};

typedef uint32_t CORERENDERSTATEID;

struct COREBLENDDESCRIPTOR {
  bool BlendEnable;
  COREBLENDFACTOR SrcBlend;
  COREBLENDFACTOR DestBlend;
  COREBLENDOP BlendOp;
  COREBLENDFACTOR SrcBlendAlpha;
  COREBLENDFACTOR DestBlendAlpha;
  COREBLENDOP BlendOpAlpha;
  uint8_t RenderTargetWriteMask;
};

class CCoreRenderStateBatch : public CCoreResource {
 protected:
  bool Dirty;

 public:
  CCoreRenderStateBatch(CCoreDevice* Device);
  ~CCoreRenderStateBatch() override;
  virtual bool Import(CXMLNode* n) = 0;
  virtual void Export(CXMLNode* n) = 0;
};

class CCoreBlendState : public CCoreRenderStateBatch {
 protected:
  bool AlphaToCoverage;
  bool IndependentBlend;
  COREBLENDDESCRIPTOR RenderTargetBlendStates[8];

 public:
  CCoreBlendState(CCoreDevice* Device);
  ~CCoreBlendState() override;

  virtual bool Update() = 0;
  virtual bool Apply() = 0;

  void SetAlphaToCoverage(bool e);
  void SetIndependentBlend(bool e);
  void SetBlendEnable(int32_t rt, bool e);
  void SetSrcBlend(int32_t rt, COREBLENDFACTOR e);
  void SetDestBlend(int32_t rt, COREBLENDFACTOR e);
  void SetBlendOp(int32_t rt, COREBLENDOP e);
  void SetSrcBlendAlpha(int32_t rt, COREBLENDFACTOR e);
  void SetDestBlendAlpha(int32_t rt, COREBLENDFACTOR e);
  void SetBlendOpAlpha(int32_t rt, COREBLENDOP e);
  void SetRenderTargetWriteMask(int32_t rt, uint8_t e);

  bool Import(CXMLNode* n) override;
  void Export(CXMLNode* n) override;
  virtual void* GetHandle() = 0;
};

class CCoreDepthStencilState : public CCoreRenderStateBatch {
 protected:
  bool DepthEnable;
  bool ZWriteEnable;
  CORECOMPARISONFUNCTION DepthFunc;

 public:
  CCoreDepthStencilState(CCoreDevice* Device);
  ~CCoreDepthStencilState() override;

  virtual bool Update() = 0;
  virtual bool Apply() = 0;

  void SetDepthEnable(bool e);
  void SetZWriteEnable(bool e);
  void SetDepthFunc(CORECOMPARISONFUNCTION e);

  bool Import(CXMLNode* n) override;
  void Export(CXMLNode* n) override;

  virtual void* GetHandle() = 0;
};

class CCoreRasterizerState : public CCoreRenderStateBatch {
 protected:
  COREFILLMODE FillMode;
  CORECULLMODE CullMode;
  bool FrontCounterClockwise;
  int32_t DepthBias;
  float DepthBiasClamp;
  float SlopeScaledDepthBias;
  bool DepthClipEnable;
  bool ScissorEnable;
  bool MultisampleEnable;
  bool AntialiasedLineEnable;

 public:
  CCoreRasterizerState(CCoreDevice* Device);
  ~CCoreRasterizerState() override;

  virtual bool Update() = 0;
  virtual bool Apply() = 0;

  void SetFillMode(COREFILLMODE e);
  void SetCullMode(CORECULLMODE e);
  void SetFrontCounterClockwise(bool e);
  void SetDepthBias(int32_t e);
  void SetDepthBiasClamp(float e);
  void SetSlopeScaledDepthBias(float e);
  void SetDepthClipEnable(bool e);
  void SetScissorEnable(bool e);
  void SetMultisampleEnable(bool e);
  void SetAntialiasedLineEnable(bool e);

  bool Import(CXMLNode* n) override;
  void Export(CXMLNode* n) override;
  virtual void* GetHandle() = 0;
};

class CCoreSamplerState : public CCoreRenderStateBatch {
 protected:
  COREFILTER Filter;
  CORETEXTUREADDRESSMODE AddressU;
  CORETEXTUREADDRESSMODE AddressV;
  CORETEXTUREADDRESSMODE AddressW;
  float MipLODBias;
  int32_t MaxAnisotropy;
  CORECOMPARISONFUNCTION ComparisonFunc;
  float BorderColor[4];
  float MinLOD;
  float MaxLOD;

 public:
  CCoreSamplerState(CCoreDevice* Device);
  ~CCoreSamplerState() override;

  virtual bool Update() = 0;
  virtual bool Apply(CORESAMPLER Smp) = 0;

  void SetFilter(COREFILTER e);

  void SetAddressU(CORETEXTUREADDRESSMODE e);
  void SetAddressV(CORETEXTUREADDRESSMODE e);
  void SetAddressW(CORETEXTUREADDRESSMODE e);

  void SetMipLODBias(float e);
  void SetMaxAnisotropy(int32_t e);
  void SetComparisonFunc(CORECOMPARISONFUNCTION e);
  void SetMinLOD(float e);
  void SetMaxLOD(float e);

  void SetBorderColor(float r, float g, float b, float a);

  bool Import(CXMLNode* n) override;
  void Export(CXMLNode* n) override;
  virtual void* GetHandle() = 0;
};
