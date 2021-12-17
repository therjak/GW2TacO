#include "src/core2/render_state.h"

#include <tchar.h>

#include <limits>

#include "src/util/xml_document.h"

CCoreRenderStateBatch::CCoreRenderStateBatch(CCoreDevice* Device)
    : CCoreResource(Device) {
  Dirty = true;
}

CCoreRenderStateBatch::~CCoreRenderStateBatch() = default;

CCoreBlendState::CCoreBlendState(CCoreDevice* Device)
    : CCoreRenderStateBatch(Device) {
  AlphaToCoverage = false;
  IndependentBlend = false;

  for (auto& RenderTargetBlendState : RenderTargetBlendStates) {
    RenderTargetBlendState.BlendEnable = false;
    RenderTargetBlendState.SrcBlend = COREBLENDFACTOR::COREBLEND_ONE;
    RenderTargetBlendState.DestBlend = COREBLENDFACTOR::COREBLEND_ZERO;
    RenderTargetBlendState.BlendOp = COREBLENDOP::COREBLENDOP_ADD;
    RenderTargetBlendState.SrcBlendAlpha = COREBLENDFACTOR::COREBLEND_ONE;
    RenderTargetBlendState.DestBlendAlpha = COREBLENDFACTOR::COREBLEND_ZERO;
    RenderTargetBlendState.BlendOpAlpha = COREBLENDOP::COREBLENDOP_ADD;
    RenderTargetBlendState.RenderTargetWriteMask = 0x0f;
  }
}

CCoreBlendState::~CCoreBlendState() = default;

void CCoreBlendState::SetRenderTargetWriteMask(int32_t rt, uint8_t e) {
  if (RenderTargetBlendStates[rt].RenderTargetWriteMask != e) Dirty = true;
  RenderTargetBlendStates[rt].RenderTargetWriteMask = e;
}

void CCoreBlendState::SetBlendOpAlpha(int32_t rt, COREBLENDOP e) {
  if (RenderTargetBlendStates[rt].BlendOpAlpha != e) Dirty = true;
  RenderTargetBlendStates[rt].BlendOpAlpha = e;
}

void CCoreBlendState::SetDestBlendAlpha(int32_t rt, COREBLENDFACTOR e) {
  if (RenderTargetBlendStates[rt].DestBlendAlpha != e) Dirty = true;
  RenderTargetBlendStates[rt].DestBlendAlpha = e;
}

void CCoreBlendState::SetSrcBlendAlpha(int32_t rt, COREBLENDFACTOR e) {
  if (RenderTargetBlendStates[rt].SrcBlendAlpha != e) Dirty = true;
  RenderTargetBlendStates[rt].SrcBlendAlpha = e;
}

void CCoreBlendState::SetBlendOp(int32_t rt, COREBLENDOP e) {
  if (RenderTargetBlendStates[rt].BlendOp != e) Dirty = true;
  RenderTargetBlendStates[rt].BlendOp = e;
}

void CCoreBlendState::SetDestBlend(int32_t rt, COREBLENDFACTOR e) {
  if (RenderTargetBlendStates[rt].DestBlend != e) Dirty = true;
  RenderTargetBlendStates[rt].DestBlend = e;
}

void CCoreBlendState::SetSrcBlend(int32_t rt, COREBLENDFACTOR e) {
  if (RenderTargetBlendStates[rt].SrcBlend != e) Dirty = true;
  RenderTargetBlendStates[rt].SrcBlend = e;
}

void CCoreBlendState::SetBlendEnable(int32_t rt, bool e) {
  if (RenderTargetBlendStates[rt].BlendEnable != e) Dirty = true;
  RenderTargetBlendStates[rt].BlendEnable = e;
}

void CCoreBlendState::SetIndependentBlend(bool e) {
  if (IndependentBlend != e) Dirty = true;
  IndependentBlend = e;
}

void CCoreBlendState::SetAlphaToCoverage(bool e) {
  if (AlphaToCoverage != e) Dirty = true;
  AlphaToCoverage = e;
}

bool CCoreBlendState::Import(CXMLNode* n) {
  std::string s;

  if (n->HasAttribute(_T("AlphaToCoverage"))) {
    n->GetChild(_T("AlphaToCoverage")).GetValue(AlphaToCoverage);
  }
  if (n->HasAttribute(_T("IndependentBlend"))) {
    n->GetChild(_T("IndependentBlend")).GetValue(IndependentBlend);
  }

  for (int32_t x = 0; x < n->GetChildCount(_T("RenderTarget")); x++) {
    CXMLNode c = n->GetChild(_T("RenderTarget"), x);
    int32_t id = 0;
    c.GetAttributeAsInteger(_T("Target"), &id);

    if (c.GetChildCount(_T("BlendEnable"))) {
      c.GetChild(_T("BlendEnable"))
          .GetValue(RenderTargetBlendStates[id].BlendEnable);
    }
    if (c.GetChildCount(_T("SrcBlend"))) {
      s = c.GetChild(_T("SrcBlend")).GetText();
      FindEnumByName(BlendFactorNames, s, RenderTargetBlendStates[id].SrcBlend);
    }
    if (c.GetChildCount(_T("DestBlend"))) {
      s = c.GetChild(_T("DestBlend")).GetText();
      FindEnumByName(BlendFactorNames, s,
                     RenderTargetBlendStates[id].DestBlend);
    }
    if (c.GetChildCount(_T("BlendOp"))) {
      s = c.GetChild(_T("BlendOp")).GetText();
      FindEnumByName(BlendOpNames, s, RenderTargetBlendStates[id].BlendOp);
    }

    if (c.GetChildCount(_T("SrcBlendAlpha"))) {
      s = c.GetChild(_T("SrcBlendAlpha")).GetText();
      FindEnumByName(BlendFactorNames, s,
                     RenderTargetBlendStates[id].SrcBlendAlpha);
    }
    if (c.GetChildCount(_T("DestBlendAlpha"))) {
      s = c.GetChild(_T("DestBlendAlpha")).GetText();
      FindEnumByName(BlendFactorNames, s,
                     RenderTargetBlendStates[id].DestBlendAlpha);
    }
    if (c.GetChildCount(_T("BlendOpAlpha"))) {
      s = c.GetChild(_T("BlendOpAlpha")).GetText();
      FindEnumByName(BlendOpNames, s, RenderTargetBlendStates[id].BlendOpAlpha);
    }

    if (c.GetChildCount(_T("RenderTargetWriteMask"))) {
      c.GetChild(_T("RenderTargetWriteMask"))
          .GetValue(RenderTargetBlendStates[id].RenderTargetWriteMask);
    }
  }

  Dirty = true;
  return true;
}

void CCoreBlendState::Export(CXMLNode* n) {
  n->AddChild(_T("AlphaToCoverage")).SetInt(AlphaToCoverage);
  n->AddChild(_T("IndependentBlend")).SetInt(IndependentBlend);

  for (int32_t x = 0; x < 8; x++) {
    CXMLNode b = n->AddChild(_T("RenderTarget"));
    b.SetAttributeFromInteger(_T("Target"), x);

    b.AddChild(_T("BlendEnable"))
        .SetInt(RenderTargetBlendStates[x].BlendEnable);
    b.AddChild(_T("SrcBlend"))
        .SetText(FindNameByEnum(BlendFactorNames,
                                RenderTargetBlendStates[x].SrcBlend)
                     .data());
    b.AddChild(_T("DestBlend"))
        .SetText(FindNameByEnum(BlendFactorNames,
                                RenderTargetBlendStates[x].DestBlend)
                     .data());
    b.AddChild(_T("BlendOp"))
        .SetText(
            FindNameByEnum(BlendOpNames, RenderTargetBlendStates[x].BlendOp)
                .data());

    b.AddChild(_T("SrcBlendAlpha"))
        .SetText(FindNameByEnum(BlendFactorNames,
                                RenderTargetBlendStates[x].SrcBlendAlpha)
                     .data());
    b.AddChild(_T("DestBlendAlpha"))
        .SetText(FindNameByEnum(BlendFactorNames,
                                RenderTargetBlendStates[x].DestBlendAlpha)
                     .data());
    b.AddChild(_T("BlendOpAlpha"))
        .SetText(FindNameByEnum(BlendOpNames,
                                RenderTargetBlendStates[x].BlendOpAlpha)
                     .data());

    b.AddChild(_T("RenderTargetWriteMask"))
        .SetInt(RenderTargetBlendStates[x].RenderTargetWriteMask);
  }
}

CCoreDepthStencilState::CCoreDepthStencilState(CCoreDevice* Device)
    : CCoreRenderStateBatch(Device) {
  DepthEnable = true;
  ZWriteEnable = true;
  DepthFunc = CORECOMPARISONFUNCTION::CORECMP_LESS;
}

CCoreDepthStencilState::~CCoreDepthStencilState() = default;

void CCoreDepthStencilState::SetDepthFunc(CORECOMPARISONFUNCTION e) {
  if (e != DepthFunc) Dirty = true;
  DepthFunc = e;
}

void CCoreDepthStencilState::SetZWriteEnable(bool e) {
  if (e != ZWriteEnable) Dirty = true;
  ZWriteEnable = e;
}

void CCoreDepthStencilState::SetDepthEnable(bool e) {
  if (e != DepthEnable) Dirty = true;
  DepthEnable = e;
}

bool CCoreDepthStencilState::Import(CXMLNode* n) {
  if (n->GetChildCount(_T("DepthEnable"))) {
    n->GetChild(_T("DepthEnable")).GetValue(DepthEnable);
  }
  if (n->GetChildCount(_T("ZWriteEnable"))) {
    n->GetChild(_T("ZWriteEnable")).GetValue(ZWriteEnable);
  }
  if (n->GetChildCount(_T("DepthFunc"))) {
    auto s = n->GetChild(_T("DepthFunc")).GetText();
    FindEnumByName(ComparisonFunctionNames, s, DepthFunc);
  }

  Dirty = true;
  return true;
}

void CCoreDepthStencilState::Export(CXMLNode* n) {
  n->AddChild(_T("DepthEnable")).SetInt(DepthEnable);
  n->AddChild(_T("ZWriteEnable")).SetInt(ZWriteEnable);
  n->AddChild(_T("DepthFunc"))
      .SetText(FindNameByEnum(ComparisonFunctionNames, DepthFunc).data());
}

CCoreRasterizerState::CCoreRasterizerState(CCoreDevice* Device)
    : CCoreRenderStateBatch(Device) {
  FillMode = COREFILL_SOLID;
  CullMode = CORECULL_CCW;
  FrontCounterClockwise = false;
  DepthBias = 0;
  DepthBiasClamp = 0;
  SlopeScaledDepthBias = 0;
  DepthClipEnable = true;
  ScissorEnable = false;
  MultisampleEnable = false;
  AntialiasedLineEnable = false;
}

CCoreRasterizerState::~CCoreRasterizerState() = default;

void CCoreRasterizerState::SetAntialiasedLineEnable(bool e) {
  if (AntialiasedLineEnable != e) Dirty = true;
  AntialiasedLineEnable = e;
}

void CCoreRasterizerState::SetMultisampleEnable(bool e) {
  if (MultisampleEnable != e) Dirty = true;
  MultisampleEnable = e;
}

void CCoreRasterizerState::SetScissorEnable(bool e) {
  if (ScissorEnable != e) Dirty = true;
  ScissorEnable = e;
}

void CCoreRasterizerState::SetDepthClipEnable(bool e) {
  if (DepthClipEnable != e) Dirty = true;
  DepthClipEnable = e;
}

void CCoreRasterizerState::SetSlopeScaledDepthBias(float e) {
  if (SlopeScaledDepthBias != e) Dirty = true;
  SlopeScaledDepthBias = e;
}

void CCoreRasterizerState::SetDepthBiasClamp(float e) {
  if (DepthBiasClamp != e) Dirty = true;
  DepthBiasClamp = e;
}

void CCoreRasterizerState::SetDepthBias(int32_t e) {
  if (DepthBias != e) Dirty = true;
  DepthBias = e;
}

void CCoreRasterizerState::SetFrontCounterClockwise(bool e) {
  if (FrontCounterClockwise != e) Dirty = true;
  FrontCounterClockwise = e;
}

void CCoreRasterizerState::SetCullMode(CORECULLMODE e) {
  if (CullMode != e) Dirty = true;
  CullMode = e;
}

void CCoreRasterizerState::SetFillMode(COREFILLMODE e) {
  if (FillMode != e) Dirty = true;
  FillMode = e;
}

bool CCoreRasterizerState::Import(CXMLNode* n) {
  if (n->GetChildCount(_T("FillMode"))) {
    auto s = n->GetChild(_T("FillMode")).GetText();
    FindEnumByName(FillModeNames, s, FillMode);
  }
  if (n->GetChildCount(_T("CullMode"))) {
    auto s = n->GetChild(_T("CullMode")).GetText();
    FindEnumByName(CullModeNames, s, CullMode);
  }

  if (n->GetChildCount(_T("DepthBias"))) {
    n->GetChild(_T("DepthBias")).GetValue(DepthBias);
  }
  if (n->GetChildCount(_T("DepthBiasClamp"))) {
    n->GetChild(_T("DepthBiasClamp")).GetValue(DepthBiasClamp);
  }
  if (n->GetChildCount(_T("SlopeScaledDepthBias"))) {
    n->GetChild(_T("SlopeScaledDepthBias")).GetValue(SlopeScaledDepthBias);
  }

  if (n->GetChildCount(_T("FrontCounterClockwise"))) {
    n->GetChild(_T("FrontCounterClockwise")).GetValue(FrontCounterClockwise);
  }
  if (n->GetChildCount(_T("DepthClipEnable"))) {
    n->GetChild(_T("DepthClipEnable")).GetValue(DepthClipEnable);
  }
  if (n->GetChildCount(_T("ScissorEnable"))) {
    n->GetChild(_T("ScissorEnable")).GetValue(ScissorEnable);
  }
  if (n->GetChildCount(_T("MultisampleEnable"))) {
    n->GetChild(_T("MultisampleEnable")).GetValue(MultisampleEnable);
  }
  if (n->GetChildCount(_T("AntialiasedLineEnable"))) {
    n->GetChild(_T("AntialiasedLineEnable")).GetValue(AntialiasedLineEnable);
  }

  Dirty = true;
  return true;
}

void CCoreRasterizerState::Export(CXMLNode* n) {
  n->AddChild(_T("FillMode"))
      .SetText(FindNameByEnum(FillModeNames, FillMode).data());
  n->AddChild(_T("CullMode"))
      .SetText(FindNameByEnum(CullModeNames, CullMode).data());

  n->AddChild(_T("DepthBias")).SetInt(DepthBias);
  n->AddChild(_T("DepthBiasClamp")).SetFloat(DepthBiasClamp);
  n->AddChild(_T("SlopeScaledDepthBias")).SetFloat(SlopeScaledDepthBias);
  n->AddChild(_T("FrontCounterClockwise")).SetInt(FrontCounterClockwise);
  n->AddChild(_T("DepthClipEnable")).SetInt(DepthClipEnable);
  n->AddChild(_T("ScissorEnable")).SetInt(ScissorEnable);
  n->AddChild(_T("MultisampleEnable")).SetInt(MultisampleEnable);
  n->AddChild(_T("AntialiasedLineEnable")).SetInt(AntialiasedLineEnable);
}

CCoreSamplerState::CCoreSamplerState(CCoreDevice* Device)
    : CCoreRenderStateBatch(Device) {
  Filter = COREFILTER_MIN_MAG_MIP_LINEAR;
  AddressU = CORETEXADDRESS_CLAMP;
  AddressV = CORETEXADDRESS_CLAMP;
  AddressW = CORETEXADDRESS_CLAMP;
  MinLOD = std::numeric_limits<float>::lowest();  //-FLT_MAX;
  MaxLOD = std::numeric_limits<float>::max();     // FLT_MAX;
  MipLODBias = 0;
  MaxAnisotropy = 1;
  ComparisonFunc = CORECOMPARISONFUNCTION::CORECMP_NEVER;
  BorderColor[0] = BorderColor[1] = BorderColor[2] = BorderColor[3] = 1;
}

CCoreSamplerState::~CCoreSamplerState() = default;

void CCoreSamplerState::SetBorderColor(float r, float g, float b, float a) {
  if (BorderColor[0] != r || BorderColor[1] != g || BorderColor[2] != b ||
      BorderColor[3] != a)
    Dirty = true;
  BorderColor[0] = r;
  BorderColor[1] = g;
  BorderColor[2] = b;
  BorderColor[3] = a;
}

void CCoreSamplerState::SetMaxLOD(float e) {
  if (MaxLOD != e) Dirty = true;
  MaxLOD = e;
}

void CCoreSamplerState::SetMinLOD(float e) {
  if (MinLOD != e) Dirty = true;
  MinLOD = e;
}

void CCoreSamplerState::SetComparisonFunc(CORECOMPARISONFUNCTION e) {
  if (ComparisonFunc != e) Dirty = true;
  ComparisonFunc = e;
}

void CCoreSamplerState::SetMaxAnisotropy(int32_t e) {
  if (MaxAnisotropy != e) Dirty = true;
  MaxAnisotropy = e;
}

void CCoreSamplerState::SetMipLODBias(float e) {
  if (MipLODBias != e) Dirty = true;
  MipLODBias = e;
}

void CCoreSamplerState::SetAddressW(CORETEXTUREADDRESSMODE e) {
  if (AddressW != e) Dirty = true;
  AddressW = e;
}

void CCoreSamplerState::SetAddressV(CORETEXTUREADDRESSMODE e) {
  if (AddressV != e) Dirty = true;
  AddressV = e;
}

void CCoreSamplerState::SetAddressU(CORETEXTUREADDRESSMODE e) {
  if (AddressU != e) Dirty = true;
  AddressU = e;
}

void CCoreSamplerState::SetFilter(COREFILTER e) {
  if (Filter != e) Dirty = true;
  Filter = e;
}

bool CCoreSamplerState::Import(CXMLNode* n) {
  if (n->GetChildCount(_T("Filter"))) {
    auto s = n->GetChild(_T("Filter")).GetText();
    FindEnumByName(FilterNames, s, Filter);
  }
  if (n->GetChildCount(_T("AddressU"))) {
    auto s = n->GetChild(_T("AddressU")).GetText();
    FindEnumByName(AddressModeNames, s, AddressU);
  }
  if (n->GetChildCount(_T("AddressV"))) {
    auto s = n->GetChild(_T("AddressV")).GetText();
    FindEnumByName(AddressModeNames, s, AddressV);
  }
  if (n->GetChildCount(_T("AddressW"))) {
    auto s = n->GetChild(_T("AddressW")).GetText();
    FindEnumByName(AddressModeNames, s, AddressW);
  }

  if (n->GetChildCount(_T("ComparisonFunc"))) {
    auto s = n->GetChild(_T("ComparisonFunc")).GetText();
    FindEnumByName(ComparisonFunctionNames, s, ComparisonFunc);
  }

  if (n->GetChildCount(_T("MipLODBias"))) {
    n->GetChild(_T("MipLODBias")).GetValue(MipLODBias);
  }
  if (n->GetChildCount(_T("MinLOD"))) {
    n->GetChild(_T("MinLOD")).GetValue(MinLOD);
  }
  if (n->GetChildCount(_T("MaxLOD"))) {
    n->GetChild(_T("MaxLOD")).GetValue(MaxLOD);
  }

  if (n->GetChildCount(_T("MaxAnisotropy"))) {
    n->GetChild(_T("MaxAnisotropy")).GetValue(MaxAnisotropy);
  }

  Dirty = true;
  return true;
}

void CCoreSamplerState::Export(CXMLNode* n) {
  n->AddChild(_T("Filter")).SetText(FindNameByEnum(FilterNames, Filter));
  n->AddChild(_T("AddressU"))
      .SetText(FindNameByEnum(AddressModeNames, AddressU));
  n->AddChild(_T("AddressV"))
      .SetText(FindNameByEnum(AddressModeNames, AddressV));
  n->AddChild(_T("AddressW"))
      .SetText(FindNameByEnum(AddressModeNames, AddressW));
  n->AddChild(_T("ComparisonFunc"))
      .SetText(FindNameByEnum(ComparisonFunctionNames, ComparisonFunc));
  n->AddChild(_T("MipLODBias")).SetFloat(MipLODBias);
  n->AddChild(_T("MinLOD")).SetFloat(MinLOD);
  n->AddChild(_T("MaxLOD")).SetFloat(MaxLOD);
  n->AddChild(_T("MaxAnisotropy")).SetInt(MaxAnisotropy);
}
