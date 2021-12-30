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

  if (n->HasAttribute("AlphaToCoverage")) {
    n->GetChild("AlphaToCoverage").GetValue(AlphaToCoverage);
  }
  if (n->HasAttribute("IndependentBlend")) {
    n->GetChild("IndependentBlend").GetValue(IndependentBlend);
  }

  for (int32_t x = 0; x < n->GetChildCount("RenderTarget"); x++) {
    CXMLNode c = n->GetChild("RenderTarget", x);
    int32_t id = 0;
    c.GetAttributeAsInteger("Target", &id);

    if (c.GetChildCount("BlendEnable")) {
      c.GetChild("BlendEnable")
          .GetValue(RenderTargetBlendStates[id].BlendEnable);
    }
    if (c.GetChildCount("SrcBlend")) {
      s = c.GetChild("SrcBlend").GetText();
      FindEnumByName(BlendFactorNames, s, RenderTargetBlendStates[id].SrcBlend);
    }
    if (c.GetChildCount("DestBlend")) {
      s = c.GetChild("DestBlend").GetText();
      FindEnumByName(BlendFactorNames, s,
                     RenderTargetBlendStates[id].DestBlend);
    }
    if (c.GetChildCount("BlendOp")) {
      s = c.GetChild("BlendOp").GetText();
      FindEnumByName(BlendOpNames, s, RenderTargetBlendStates[id].BlendOp);
    }

    if (c.GetChildCount("SrcBlendAlpha")) {
      s = c.GetChild("SrcBlendAlpha").GetText();
      FindEnumByName(BlendFactorNames, s,
                     RenderTargetBlendStates[id].SrcBlendAlpha);
    }
    if (c.GetChildCount("DestBlendAlpha")) {
      s = c.GetChild("DestBlendAlpha").GetText();
      FindEnumByName(BlendFactorNames, s,
                     RenderTargetBlendStates[id].DestBlendAlpha);
    }
    if (c.GetChildCount("BlendOpAlpha")) {
      s = c.GetChild("BlendOpAlpha").GetText();
      FindEnumByName(BlendOpNames, s, RenderTargetBlendStates[id].BlendOpAlpha);
    }

    if (c.GetChildCount("RenderTargetWriteMask")) {
      c.GetChild("RenderTargetWriteMask")
          .GetValue(RenderTargetBlendStates[id].RenderTargetWriteMask);
    }
  }

  Dirty = true;
  return true;
}

void CCoreBlendState::Export(CXMLNode* n) {
  n->AddChild("AlphaToCoverage").SetInt(AlphaToCoverage);
  n->AddChild("IndependentBlend").SetInt(IndependentBlend);

  for (int32_t x = 0; x < 8; x++) {
    CXMLNode b = n->AddChild("RenderTarget");
    b.SetAttributeFromInteger("Target", x);

    b.AddChild("BlendEnable").SetInt(RenderTargetBlendStates[x].BlendEnable);
    b.AddChild("SrcBlend")
        .SetText(FindNameByEnum(BlendFactorNames,
                                RenderTargetBlendStates[x].SrcBlend)
                     .data());
    b.AddChild("DestBlend")
        .SetText(FindNameByEnum(BlendFactorNames,
                                RenderTargetBlendStates[x].DestBlend)
                     .data());
    b.AddChild("BlendOp").SetText(
        FindNameByEnum(BlendOpNames, RenderTargetBlendStates[x].BlendOp)
            .data());

    b.AddChild("SrcBlendAlpha")
        .SetText(FindNameByEnum(BlendFactorNames,
                                RenderTargetBlendStates[x].SrcBlendAlpha)
                     .data());
    b.AddChild("DestBlendAlpha")
        .SetText(FindNameByEnum(BlendFactorNames,
                                RenderTargetBlendStates[x].DestBlendAlpha)
                     .data());
    b.AddChild("BlendOpAlpha")
        .SetText(FindNameByEnum(BlendOpNames,
                                RenderTargetBlendStates[x].BlendOpAlpha)
                     .data());

    b.AddChild("RenderTargetWriteMask")
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
  if (n->GetChildCount("DepthEnable")) {
    n->GetChild("DepthEnable").GetValue(DepthEnable);
  }
  if (n->GetChildCount("ZWriteEnable")) {
    n->GetChild("ZWriteEnable").GetValue(ZWriteEnable);
  }
  if (n->GetChildCount("DepthFunc")) {
    auto s = n->GetChild("DepthFunc").GetText();
    FindEnumByName(ComparisonFunctionNames, s, DepthFunc);
  }

  Dirty = true;
  return true;
}

void CCoreDepthStencilState::Export(CXMLNode* n) {
  n->AddChild("DepthEnable").SetInt(DepthEnable);
  n->AddChild("ZWriteEnable").SetInt(ZWriteEnable);
  n->AddChild("DepthFunc")
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
  if (n->GetChildCount("FillMode")) {
    auto s = n->GetChild("FillMode").GetText();
    FindEnumByName(FillModeNames, s, FillMode);
  }
  if (n->GetChildCount("CullMode")) {
    auto s = n->GetChild("CullMode").GetText();
    FindEnumByName(CullModeNames, s, CullMode);
  }

  if (n->GetChildCount("DepthBias")) {
    n->GetChild("DepthBias").GetValue(DepthBias);
  }
  if (n->GetChildCount("DepthBiasClamp")) {
    n->GetChild("DepthBiasClamp").GetValue(DepthBiasClamp);
  }
  if (n->GetChildCount("SlopeScaledDepthBias")) {
    n->GetChild("SlopeScaledDepthBias").GetValue(SlopeScaledDepthBias);
  }

  if (n->GetChildCount("FrontCounterClockwise")) {
    n->GetChild("FrontCounterClockwise").GetValue(FrontCounterClockwise);
  }
  if (n->GetChildCount("DepthClipEnable")) {
    n->GetChild("DepthClipEnable").GetValue(DepthClipEnable);
  }
  if (n->GetChildCount("ScissorEnable")) {
    n->GetChild("ScissorEnable").GetValue(ScissorEnable);
  }
  if (n->GetChildCount("MultisampleEnable")) {
    n->GetChild("MultisampleEnable").GetValue(MultisampleEnable);
  }
  if (n->GetChildCount("AntialiasedLineEnable")) {
    n->GetChild("AntialiasedLineEnable").GetValue(AntialiasedLineEnable);
  }

  Dirty = true;
  return true;
}

void CCoreRasterizerState::Export(CXMLNode* n) {
  n->AddChild("FillMode")
      .SetText(FindNameByEnum(FillModeNames, FillMode).data());
  n->AddChild("CullMode")
      .SetText(FindNameByEnum(CullModeNames, CullMode).data());

  n->AddChild("DepthBias").SetInt(DepthBias);
  n->AddChild("DepthBiasClamp").SetFloat(DepthBiasClamp);
  n->AddChild("SlopeScaledDepthBias").SetFloat(SlopeScaledDepthBias);
  n->AddChild("FrontCounterClockwise").SetInt(FrontCounterClockwise);
  n->AddChild("DepthClipEnable").SetInt(DepthClipEnable);
  n->AddChild("ScissorEnable").SetInt(ScissorEnable);
  n->AddChild("MultisampleEnable").SetInt(MultisampleEnable);
  n->AddChild("AntialiasedLineEnable").SetInt(AntialiasedLineEnable);
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
  if (n->GetChildCount("Filter")) {
    auto s = n->GetChild("Filter").GetText();
    FindEnumByName(FilterNames, s, Filter);
  }
  if (n->GetChildCount("AddressU")) {
    auto s = n->GetChild("AddressU").GetText();
    FindEnumByName(AddressModeNames, s, AddressU);
  }
  if (n->GetChildCount("AddressV")) {
    auto s = n->GetChild("AddressV").GetText();
    FindEnumByName(AddressModeNames, s, AddressV);
  }
  if (n->GetChildCount("AddressW")) {
    auto s = n->GetChild("AddressW").GetText();
    FindEnumByName(AddressModeNames, s, AddressW);
  }

  if (n->GetChildCount("ComparisonFunc")) {
    auto s = n->GetChild("ComparisonFunc").GetText();
    FindEnumByName(ComparisonFunctionNames, s, ComparisonFunc);
  }

  if (n->GetChildCount("MipLODBias")) {
    n->GetChild("MipLODBias").GetValue(MipLODBias);
  }
  if (n->GetChildCount("MinLOD")) {
    n->GetChild("MinLOD").GetValue(MinLOD);
  }
  if (n->GetChildCount("MaxLOD")) {
    n->GetChild("MaxLOD").GetValue(MaxLOD);
  }

  if (n->GetChildCount("MaxAnisotropy")) {
    n->GetChild("MaxAnisotropy").GetValue(MaxAnisotropy);
  }

  Dirty = true;
  return true;
}

void CCoreSamplerState::Export(CXMLNode* n) {
  n->AddChild("Filter").SetText(FindNameByEnum(FilterNames, Filter));
  n->AddChild("AddressU").SetText(FindNameByEnum(AddressModeNames, AddressU));
  n->AddChild("AddressV").SetText(FindNameByEnum(AddressModeNames, AddressV));
  n->AddChild("AddressW").SetText(FindNameByEnum(AddressModeNames, AddressW));
  n->AddChild("ComparisonFunc")
      .SetText(FindNameByEnum(ComparisonFunctionNames, ComparisonFunc));
  n->AddChild("MipLODBias").SetFloat(MipLODBias);
  n->AddChild("MinLOD").SetFloat(MinLOD);
  n->AddChild("MaxLOD").SetFloat(MaxLOD);
  n->AddChild("MaxAnisotropy").SetInt(MaxAnisotropy);
}
