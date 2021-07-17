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
  CCoreSamplerState *SamplerState;
  CCoreDepthStencilState *DepthStencilState;
  CCoreBlendState *BlendState;
  CCoreRasterizerState *RasterizerState;
  CCoreTexture *Texture;
  CCoreIndexBuffer *IndexBuffer;
  CCoreVertexFormat *VertexFormat;
  CCoreVertexShader *VertexShader;
  CCorePixelShader *PixelShader;
  CCoreGeometryShader *GeometryShader;
  CCoreDomainShader *DomainShader;
  CCoreComputeShader *ComputeShader;
  CCoreHullShader *HullShader;

  INLINE const TBOOL operator!=(const CORERENDERSTATEVALUE &v) {
    return SamplerState != v.SamplerState;
  }
};

typedef uint32_t CORERENDERSTATEID;

struct COREBLENDDESCRIPTOR {
  TBOOL BlendEnable;
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
  TBOOL Dirty;

 public:
  CCoreRenderStateBatch(CCoreDevice *Device);
  ~CCoreRenderStateBatch() override;
  virtual TBOOL Import(CXMLNode *n) = 0;
  virtual void Export(CXMLNode *n) = 0;
};

class CCoreBlendState : public CCoreRenderStateBatch {
 protected:
  TBOOL AlphaToCoverage;
  TBOOL IndependentBlend;
  COREBLENDDESCRIPTOR RenderTargetBlendStates[8];

 public:
  CCoreBlendState(CCoreDevice *Device);
  ~CCoreBlendState() override;

  virtual TBOOL Update() = 0;
  virtual TBOOL Apply() = 0;

  void SetAlphaToCoverage(TBOOL e);
  void SetIndependentBlend(TBOOL e);
  void SetBlendEnable(int32_t rt, TBOOL e);
  void SetSrcBlend(int32_t rt, COREBLENDFACTOR e);
  void SetDestBlend(int32_t rt, COREBLENDFACTOR e);
  void SetBlendOp(int32_t rt, COREBLENDOP e);
  void SetSrcBlendAlpha(int32_t rt, COREBLENDFACTOR e);
  void SetDestBlendAlpha(int32_t rt, COREBLENDFACTOR e);
  void SetBlendOpAlpha(int32_t rt, COREBLENDOP e);
  void SetRenderTargetWriteMask(int32_t rt, uint8_t e);

  TBOOL Import(CXMLNode *n) override;
  void Export(CXMLNode *n) override;
  virtual void *GetHandle() = 0;
};

class CCoreDepthStencilState : public CCoreRenderStateBatch {
 protected:
  TBOOL DepthEnable;
  TBOOL ZWriteEnable;
  CORECOMPARISONFUNCTION DepthFunc;

 public:
  CCoreDepthStencilState(CCoreDevice *Device);
  ~CCoreDepthStencilState() override;

  virtual TBOOL Update() = 0;
  virtual TBOOL Apply() = 0;

  void SetDepthEnable(TBOOL e);
  void SetZWriteEnable(TBOOL e);
  void SetDepthFunc(CORECOMPARISONFUNCTION e);

  TBOOL Import(CXMLNode *n) override;
  void Export(CXMLNode *n) override;

  virtual void *GetHandle() = 0;
};

class CCoreRasterizerState : public CCoreRenderStateBatch {
 protected:
  COREFILLMODE FillMode;
  CORECULLMODE CullMode;
  TBOOL FrontCounterClockwise;
  int32_t DepthBias;
  float DepthBiasClamp;
  float SlopeScaledDepthBias;
  TBOOL DepthClipEnable;
  TBOOL ScissorEnable;
  TBOOL MultisampleEnable;
  TBOOL AntialiasedLineEnable;

 public:
  CCoreRasterizerState(CCoreDevice *Device);
  ~CCoreRasterizerState() override;

  virtual TBOOL Update() = 0;
  virtual TBOOL Apply() = 0;

  void SetFillMode(COREFILLMODE e);
  void SetCullMode(CORECULLMODE e);
  void SetFrontCounterClockwise(TBOOL e);
  void SetDepthBias(int32_t e);
  void SetDepthBiasClamp(float e);
  void SetSlopeScaledDepthBias(float e);
  void SetDepthClipEnable(TBOOL e);
  void SetScissorEnable(TBOOL e);
  void SetMultisampleEnable(TBOOL e);
  void SetAntialiasedLineEnable(TBOOL e);

  TBOOL Import(CXMLNode *n) override;
  void Export(CXMLNode *n) override;
  virtual void *GetHandle() = 0;
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
  CCoreSamplerState(CCoreDevice *Device);
  ~CCoreSamplerState() override;

  virtual TBOOL Update() = 0;
  virtual TBOOL Apply(CORESAMPLER Smp) = 0;

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

  TBOOL Import(CXMLNode *n) override;
  void Export(CXMLNode *n) override;
  virtual void *GetHandle() = 0;
};
