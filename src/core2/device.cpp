#include "src/core2/device.h"

#include "src/core2/resource.h"

INLINE const CORERENDERSTATEID IDFromRenderState(const CORERENDERSTATE State,
                                                 const CORESAMPLER Sampler) {
  return (State << 16) + Sampler;
}

INLINE void RenderStateFromID(const CORERENDERSTATEID ID,
                              CORERENDERSTATE& State, CORESAMPLER& Sampler) {
  State = static_cast<CORERENDERSTATE>(ID >> 16);
  Sampler = static_cast<CORESAMPLER>(ID & 0xffff);
}

void CCoreDevice::AddResource(CCoreResource* Resource) {
  Resources.push_back(Resource);
}

void CCoreDevice::RemoveResource(CCoreResource* Resource) {
  Resources.erase(std::remove(Resources.begin(), Resources.end(), Resource),
                  Resources.end());
}

CCoreDevice::CCoreDevice() = default;

CCoreDevice::~CCoreDevice() {
  DefaultRasterizerState.reset();
  DefaultBlendState.reset();
  DefaultDepthStencilState.reset();

#ifdef ENABLE_STACKTRACKER_CLASS
  if (!Resources.empty()) {
#ifdef _DEBUG
    LOG_ERR("[core] ---Leaked graphical resources start here---");
#endif  // _DEBUG
  }
  int32_t Counter = 0;
#endif

  // remove remaining (leaked) resources:
  for (auto r : Resources) {
#ifdef ENABLE_STACKTRACKER_CLASS
#ifdef _DEBUG
    LOG_ERR("[core] Unfreed graphical resource found. Allocation stack:");
    r->StackInfo.DumpToLog(LOGVERBOSITY::LOG_ERROR);
#endif  // _DEBUG
    Counter++;
#endif
    delete r;
  }
  Resources.clear();

#ifdef ENABLE_STACKTRACKER_CLASS
  if (Counter > 0)
    LOG_NFO(
        _T("[core] %d unfreed graphical resources found on exit. This is ")
        _T("fine."),
        Counter);
#endif
}

void CCoreDevice::ResetDevice() {
  for (auto& r : Resources) r->OnDeviceLost();

  ResetPrivateResources();

  for (auto& r : Resources) r->OnDeviceReset();

  // reload render state
  RequestedRenderState.merge(CurrentRenderState);
  CurrentRenderState.clear();
}

bool CCoreDevice::ApplyRequestedRenderState() {
  for (const auto& x : RequestedRenderState) {
    const CORERENDERSTATEID ID = x.first;
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
      // therjak: WTF? was there some reason for this? was this just a bug?
      //          should this be something with CurrentVertexBuffer*?
      // RequestedVertexBuffer = RequestedVertexBuffer;
      // RequestedVertexBufferOffset = RequestedVertexBufferOffset;
    }
  }

  RequestedRenderState.clear();

  return CommitRenderStates();
}

bool CCoreDevice::ApplyTextureToSampler(const CORESAMPLER Sampler,
                                        CCoreTexture* Texture) {
  if (!Texture) return false;
  return Texture->SetToSampler(Sampler);
}

bool CCoreDevice::ApplyVertexShader(CCoreVertexShader* Shader) {
  if (!Shader) return false;
  return Shader->Apply();
}

bool CCoreDevice::ApplyGeometryShader(CCoreGeometryShader* Shader) {
  if (!Shader) return false;
  return Shader->Apply();
}

bool CCoreDevice::ApplyHullShader(CCoreHullShader* Shader) {
  if (!Shader) return false;
  return Shader->Apply();
}

bool CCoreDevice::ApplyDomainShader(CCoreDomainShader* Shader) {
  if (!Shader) return false;
  return Shader->Apply();
}

bool CCoreDevice::ApplyComputeShader(CCoreComputeShader* Shader) {
  if (!Shader) return false;
  return Shader->Apply();
}

bool CCoreDevice::ApplyPixelShader(CCorePixelShader* Shader) {
  if (!Shader) return false;
  return Shader->Apply();
}

bool CCoreDevice::ApplyVertexFormat(CCoreVertexFormat* Format) {
  if (!Format) return false;
  return Format->Apply();
}

bool CCoreDevice::ApplyIndexBuffer(CCoreIndexBuffer* IdxBuffer) {
  if (!IdxBuffer) return false;
  return IdxBuffer->Apply();
}

bool CCoreDevice::ApplyVertexBuffer(CCoreVertexBuffer* VxBuffer,
                                    uint32_t Offset) {
  if (!VxBuffer) return false;
  return VxBuffer->Apply(Offset);
}

bool CCoreDevice::SetSamplerState(CORESAMPLER Sampler,
                                  CCoreSamplerState* SamplerState) {
  RequestedRenderState[IDFromRenderState(CORERS_SAMPLERSTATE, Sampler)]
      .SamplerState = SamplerState;
  return true;
}

bool CCoreDevice::SetRenderState(CCoreRasterizerState* RasterizerState) {
  RequestedRenderState[IDFromRenderState(CORERS_RASTERIZERSTATE,
                                         static_cast<CORESAMPLER>(0))]
      .RasterizerState = RasterizerState;
  return true;
}

bool CCoreDevice::SetRenderState(CCoreBlendState* BlendState) {
  RequestedRenderState[IDFromRenderState(CORERS_BLENDSTATE,
                                         static_cast<CORESAMPLER>(0))]
      .BlendState = BlendState;
  return true;
}

bool CCoreDevice::SetRenderState(CCoreDepthStencilState* DepthStencilState) {
  RequestedRenderState[IDFromRenderState(CORERS_DEPTHSTENCILSTATE,
                                         static_cast<CORESAMPLER>(0))]
      .DepthStencilState = DepthStencilState;
  return true;
}

bool CCoreDevice::SetTexture(CORESAMPLER Sampler, CCoreTexture* Texture) {
  RequestedRenderState[IDFromRenderState(CORERS_TEXTURE, Sampler)].Texture =
      Texture;
  return true;
}

bool CCoreDevice::SetVertexShader(CCoreVertexShader* Shader) {
  RequestedRenderState[IDFromRenderState(CORERS_VERTEXSHADER,
                                         static_cast<CORESAMPLER>(0))]
      .VertexShader = Shader;
  return true;
}

bool CCoreDevice::SetPixelShader(CCorePixelShader* Shader) {
  RequestedRenderState[IDFromRenderState(CORERS_PIXELSHADER,
                                         static_cast<CORESAMPLER>(0))]
      .PixelShader = Shader;
  return true;
}

bool CCoreDevice::SetGeometryShader(CCoreGeometryShader* Shader) {
  RequestedRenderState[IDFromRenderState(CORERS_GEOMETRYSHADER,
                                         static_cast<CORESAMPLER>(0))]
      .GeometryShader = Shader;
  return true;
}

bool CCoreDevice::SetHullShader(CCoreHullShader* Shader) {
  RequestedRenderState[IDFromRenderState(CORERS_HULLSHADER,
                                         static_cast<CORESAMPLER>(0))]
      .HullShader = Shader;
  return true;
}

bool CCoreDevice::SetDomainShader(CCoreDomainShader* Shader) {
  RequestedRenderState[IDFromRenderState(CORERS_DOMAINSHADER,
                                         static_cast<CORESAMPLER>(0))]
      .DomainShader = Shader;
  return true;
}

bool CCoreDevice::SetVertexBuffer(CCoreVertexBuffer* VertexBuffer,
                                  uint32_t Offset) {
  RequestedVertexBuffer = VertexBuffer;
  RequestedVertexBufferOffset = Offset;
  return true;
}

bool CCoreDevice::SetIndexBuffer(CCoreIndexBuffer* IndexBuffer) {
  RequestedRenderState[IDFromRenderState(CORERS_INDEXBUFFER,
                                         static_cast<CORESAMPLER>(0))]
      .IndexBuffer = IndexBuffer;
  return true;
}

bool CCoreDevice::SetVertexFormat(CCoreVertexFormat* VertexFormat) {
  RequestedRenderState[IDFromRenderState(CORERS_VERTEXFORMAT,
                                         static_cast<CORESAMPLER>(0))]
      .VertexFormat = VertexFormat;
  return true;
}

int32_t CCoreDevice::GetVertexFormatSize() { return CurrentVertexFormatSize; }

CCoreTexture* CCoreDevice::GetTexture(CORESAMPLER Sampler) {
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

bool CCoreDevice::CreateDefaultRenderStates() {
  bool Success = true;

  DefaultBlendState = CreateBlendState();
  DefaultBlendState->SetBlendEnable(0, true);
  DefaultBlendState->SetSrcBlend(0, COREBLENDFACTOR::COREBLEND_SRCALPHA);
  DefaultBlendState->SetDestBlend(0, COREBLENDFACTOR::COREBLEND_INVSRCALPHA);
  Success |= DefaultBlendState->Apply();

  DefaultDepthStencilState = CreateDepthStencilState();
  DefaultDepthStencilState->SetDepthEnable(true);
  DefaultDepthStencilState->SetZWriteEnable(true);
  DefaultDepthStencilState->SetDepthFunc(
      CORECOMPARISONFUNCTION::CORECMP_LEQUAL);
  Success |= DefaultDepthStencilState->Apply();

  DefaultRasterizerState = CreateRasterizerState();
  Success |= DefaultRasterizerState->Apply();

  return Success;
}
