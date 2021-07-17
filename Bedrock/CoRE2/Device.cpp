#include "Device.h"

#include "Resource.h"

INLINE const CORERENDERSTATEID IDFromRenderState(const CORERENDERSTATE State,
                                                 const CORESAMPLER Sampler) {
  return (State << 16) + Sampler;
}

INLINE void RenderStateFromID(const CORERENDERSTATEID ID,
                              CORERENDERSTATE &State, CORESAMPLER &Sampler) {
  State = static_cast<CORERENDERSTATE>(ID >> 16);
  Sampler = static_cast<CORESAMPLER>(ID & 0xffff);
}

void CCoreDevice::AddResource(CCoreResource *Resource) {
  Resources.push_back(Resource);
}

void CCoreDevice::RemoveResource(CCoreResource *Resource) {
  Resources.erase(std::remove(Resources.begin(), Resources.end(), Resource),
                  Resources.end());
}

CCoreDevice::CCoreDevice() {
  Window = nullptr;
  CurrentVertexBuffer = RequestedVertexBuffer = nullptr;
  CurrentVertexBufferOffset = RequestedVertexBufferOffset = 0;
  CurrentVertexFormatSize = 0;
}

CCoreDevice::~CCoreDevice() {
  DefaultRasterizerState.reset();
  DefaultBlendState.reset();
  DefaultDepthStencilState.reset();

#ifdef ENABLE_STACKTRACKER_CLASS
  if (!Resources.empty()) {
#ifdef _DEBUG
    LOG(LOG_ERROR, _T("[core] ---Leaked graphical resources start here---"));
#endif  // _DEBUG
  }
  int32_t Counter = 0;
#endif

  // remove remaining (leaked) resources:
  for (auto r : Resources) {
#ifdef ENABLE_STACKTRACKER_CLASS
#ifdef _DEBUG
    LOG(LOG_ERROR,
        _T("[core] Unfreed graphical resource found. Allocation stack:"));
    r->StackInfo.DumpToLog(LOG_ERROR);
#endif  // _DEBUG
    Counter++;
#endif
    delete r;
  }
  Resources.clear();

#ifdef ENABLE_STACKTRACKER_CLASS
  if (Counter > 0)
    LOG(LOG_INFO,
        _T("[core] %d unfreed graphical resources found on exit. This is ")
        _T("fine."),
        Counter);
#endif
}

void CCoreDevice::ResetDevice() {
  for (auto &r : Resources) r->OnDeviceLost();

  ResetPrivateResources();

  for (auto &r : Resources) r->OnDeviceReset();

  // reload render state
  RequestedRenderState.merge(CurrentRenderState);
  CurrentRenderState.clear();
}

TBOOL CCoreDevice::ApplyRequestedRenderState() {
  for (const auto &x : RequestedRenderState) {
    CORERENDERSTATEID ID = x.first;
    CORERENDERSTATEVALUE Value = x.second;

    if (CurrentRenderState.find(ID) == CurrentRenderState.end() ||
        CurrentRenderState[ID] != Value) {
      CORERENDERSTATE RS;
      CORESAMPLER Smp;
      RenderStateFromID(ID, RS, Smp);
      if (!ApplyRenderState(Smp, RS, Value)) return false;
      CurrentRenderState[ID] = Value;
    }
  }

  if (CurrentVertexBuffer != RequestedVertexBuffer ||
      CurrentVertexBufferOffset != RequestedVertexBufferOffset) {
    if (!RequestedVertexBuffer) {
      if (!SetNoVertexBuffer()) return false;
      RequestedVertexBuffer = nullptr;
      RequestedVertexBufferOffset = 0;
    } else {
      if (!ApplyVertexBuffer(RequestedVertexBuffer,
                             RequestedVertexBufferOffset))
        return false;
      RequestedVertexBuffer = RequestedVertexBuffer;
      RequestedVertexBufferOffset = RequestedVertexBufferOffset;
    }
  }

  RequestedRenderState.clear();

  return CommitRenderStates();
}

TBOOL CCoreDevice::ApplyTextureToSampler(const CORESAMPLER Sampler,
                                         CCoreTexture *Texture) {
  if (!Texture) return false;
  return Texture->SetToSampler(Sampler);
}

TBOOL CCoreDevice::ApplyVertexShader(CCoreVertexShader *Shader) {
  if (!Shader) return false;
  return Shader->Apply();
}

TBOOL CCoreDevice::ApplyGeometryShader(CCoreGeometryShader *Shader) {
  if (!Shader) return false;
  return Shader->Apply();
}

TBOOL CCoreDevice::ApplyHullShader(CCoreHullShader *Shader) {
  if (!Shader) return false;
  return Shader->Apply();
}

TBOOL CCoreDevice::ApplyDomainShader(CCoreDomainShader *Shader) {
  if (!Shader) return false;
  return Shader->Apply();
}

TBOOL CCoreDevice::ApplyComputeShader(CCoreComputeShader *Shader) {
  if (!Shader) return false;
  return Shader->Apply();
}

TBOOL CCoreDevice::ApplyPixelShader(CCorePixelShader *Shader) {
  if (!Shader) return false;
  return Shader->Apply();
}

TBOOL CCoreDevice::ApplyVertexFormat(CCoreVertexFormat *Format) {
  if (!Format) return false;
  return Format->Apply();
}

TBOOL CCoreDevice::ApplyIndexBuffer(CCoreIndexBuffer *IdxBuffer) {
  if (!IdxBuffer) return false;
  return IdxBuffer->Apply();
}

TBOOL CCoreDevice::ApplyVertexBuffer(CCoreVertexBuffer *VxBuffer,
                                     uint32_t Offset) {
  if (!VxBuffer) return false;
  return VxBuffer->Apply(Offset);
}

TBOOL CCoreDevice::SetSamplerState(CORESAMPLER Sampler,
                                   CCoreSamplerState *SamplerState) {
  RequestedRenderState[IDFromRenderState(CORERS_SAMPLERSTATE, Sampler)]
      .SamplerState = SamplerState;
  return true;
}

TBOOL CCoreDevice::SetRenderState(CCoreRasterizerState *RasterizerState) {
  RequestedRenderState[IDFromRenderState(CORERS_RASTERIZERSTATE,
                                         static_cast<CORESAMPLER>(0))]
      .RasterizerState = RasterizerState;
  return true;
}

TBOOL CCoreDevice::SetRenderState(CCoreBlendState *BlendState) {
  RequestedRenderState[IDFromRenderState(CORERS_BLENDSTATE,
                                         static_cast<CORESAMPLER>(0))]
      .BlendState = BlendState;
  return true;
}

TBOOL CCoreDevice::SetRenderState(CCoreDepthStencilState *DepthStencilState) {
  RequestedRenderState[IDFromRenderState(CORERS_DEPTHSTENCILSTATE,
                                         static_cast<CORESAMPLER>(0))]
      .DepthStencilState = DepthStencilState;
  return true;
}

TBOOL CCoreDevice::SetTexture(CORESAMPLER Sampler, CCoreTexture *Texture) {
  RequestedRenderState[IDFromRenderState(CORERS_TEXTURE, Sampler)].Texture =
      Texture;
  return true;
}

TBOOL CCoreDevice::SetVertexShader(CCoreVertexShader *Shader) {
  RequestedRenderState[IDFromRenderState(CORERS_VERTEXSHADER,
                                         static_cast<CORESAMPLER>(0))]
      .VertexShader = Shader;
  return true;
}

TBOOL CCoreDevice::SetPixelShader(CCorePixelShader *Shader) {
  RequestedRenderState[IDFromRenderState(CORERS_PIXELSHADER,
                                         static_cast<CORESAMPLER>(0))]
      .PixelShader = Shader;
  return true;
}

TBOOL CCoreDevice::SetGeometryShader(CCoreGeometryShader *Shader) {
  RequestedRenderState[IDFromRenderState(CORERS_GEOMETRYSHADER,
                                         static_cast<CORESAMPLER>(0))]
      .GeometryShader = Shader;
  return true;
}

TBOOL CCoreDevice::SetHullShader(CCoreHullShader *Shader) {
  RequestedRenderState[IDFromRenderState(CORERS_HULLSHADER,
                                         static_cast<CORESAMPLER>(0))]
      .HullShader = Shader;
  return true;
}

TBOOL CCoreDevice::SetDomainShader(CCoreDomainShader *Shader) {
  RequestedRenderState[IDFromRenderState(CORERS_DOMAINSHADER,
                                         static_cast<CORESAMPLER>(0))]
      .DomainShader = Shader;
  return true;
}

TBOOL CCoreDevice::SetVertexBuffer(CCoreVertexBuffer *VertexBuffer,
                                   uint32_t Offset) {
  RequestedVertexBuffer = VertexBuffer;
  RequestedVertexBufferOffset = Offset;
  return true;
}

TBOOL CCoreDevice::SetIndexBuffer(CCoreIndexBuffer *IndexBuffer) {
  RequestedRenderState[IDFromRenderState(CORERS_INDEXBUFFER,
                                         static_cast<CORESAMPLER>(0))]
      .IndexBuffer = IndexBuffer;
  return true;
}

TBOOL CCoreDevice::SetVertexFormat(CCoreVertexFormat *VertexFormat) {
  RequestedRenderState[IDFromRenderState(CORERS_VERTEXFORMAT,
                                         static_cast<CORESAMPLER>(0))]
      .VertexFormat = VertexFormat;
  return true;
}

int32_t CCoreDevice::GetVertexFormatSize() { return CurrentVertexFormatSize; }

CCoreTexture *CCoreDevice::GetTexture(CORESAMPLER Sampler) {
  if (RequestedRenderState.find(IDFromRenderState(CORERS_TEXTURE, Sampler)) !=
      RequestedRenderState.end())
    return RequestedRenderState[IDFromRenderState(CORERS_TEXTURE, Sampler)]
        .Texture;
  if (CurrentRenderState.find(IDFromRenderState(CORERS_TEXTURE, Sampler)) !=
      CurrentRenderState.end())
    return CurrentRenderState[IDFromRenderState(CORERS_TEXTURE, Sampler)]
        .Texture;
  return nullptr;
}

TBOOL CCoreDevice::CreateDefaultRenderStates() {
  TBOOL Success = true;

  DefaultBlendState.swap(CreateBlendState());
  DefaultBlendState->SetBlendEnable(0, true);
  DefaultBlendState->SetSrcBlend(0, COREBLEND_SRCALPHA);
  DefaultBlendState->SetDestBlend(0, COREBLEND_INVSRCALPHA);
  Success |= DefaultBlendState->Apply();

  DefaultDepthStencilState.swap(CreateDepthStencilState());
  DefaultDepthStencilState->SetDepthEnable(true);
  DefaultDepthStencilState->SetZWriteEnable(true);
  DefaultDepthStencilState->SetDepthFunc(CORECMP_LEQUAL);
  Success |= DefaultDepthStencilState->Apply();

  DefaultRasterizerState.swap(CreateRasterizerState());
  Success |= DefaultRasterizerState->Apply();

  return Success;
}
