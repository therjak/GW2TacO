module;
#include <array>
#include <cassert>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/base/hasher.h"
#include "src/base/matrix.h"
#include "src/base/stream_reader.h"
#include "src/core2/device.h"
#include "src/gw2_tactical.h"

export module taco.trail_logger;

import whiteboard;
import taco.mumble_link;

export void GlobalDoTrailLogging(int32_t mapID, math::CVector3 charPos);

export struct GW2TrailVertex {
  math::CVector4 Pos;
  math::CVector2 UV;
  math::CVector4 CenterPos;
  CColor Color;
};

export class GW2TrailDisplay;

export class GW2Trail {
  friend class GW2TrailDisplay;

 public:
  virtual ~GW2Trail();
  void Build(renderer::CCoreDevice* dev, int32_t mapID, const float* points,
             int pointCount);
  void Draw();
  void Update();
  void SetupAndDraw(renderer::CCoreConstantBuffer* constBuffer,
                    renderer::CCoreTexture* texture, math::CMatrix4x4& cam,
                    math::CMatrix4x4& persp, float& one, bool scaleData,
                    int32_t fadeoutBubble, std::array<float, 8>& data,
                    float fadeAlpha, float width, float uvScale, float width2d);
  void SetCategory(GW2TacticalCategory* t);

  bool Import(CStreamReaderMemory& file, bool keepPoints = false);
  bool Import(std::string_view fileName, std::string_view zipFile,
              bool keepPoints = false);

  int32_t length = 0;
  std::unique_ptr<renderer::CCoreVertexBuffer> trailMesh;
  renderer::CCoreDevice* dev = nullptr;
  std::unique_ptr<renderer::CCoreIndexBuffer> idxBuf;
  renderer::CCoreTexture* texture = nullptr;

  int32_t map = 0;

  MarkerTypeData typeData;
  std::string Type;
  GUID guid{};
  bool External = false;
  std::string zipFile;

  GW2TacticalCategory* category = nullptr;

 private:
  void Reset(int32_t _mapID = 0);
  bool SaveToFile(std::string_view fname);

  std::vector<math::CVector3> positions;
};

export class GW2TrailDisplay : public CWBGuiType<"gw2Trails", CWBItem> {
 public:
  GW2TrailDisplay();
  ~GW2TrailDisplay() override;
  static inline GW2TrailDisplay* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_unique<GW2TrailDisplay>();
    p->Initialize(Parent, Position);
    GW2TrailDisplay* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  bool Initialize(CWBItem* Parent, const math::CRect& Position) override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  void DoTrailLogging(int32_t mapID, math::CVector3 charPos);

  void StartStopTrailRecording(bool start);
  void PauseTrail(bool pause, bool newSection = false);
  void DeleteLastTrailSegment();
  void DeleteTrailSegment();
  void ExportTrail();
  void ImportTrail();

  void DrawProxy(CWBDrawAPI* API, bool miniMaprender);

 private:
  void OnDraw(CWBDrawAPI* API) override;
  void ClearEditedTrail();
  renderer::CCoreTexture2D* GetTexture(const std::string_view& fname,
                                       const std::string_view& zipFile,
                                       const std::string_view& categoryZip);

  float asp = 0;
  math::CMatrix4x4 cam;
  math::CMatrix4x4 persp;
  math::CRect drawrect;

  std::unique_ptr<renderer::CCoreVertexShader> vxShader;
  std::unique_ptr<renderer::CCoreVertexFormat> vertexFormat;
  std::unique_ptr<renderer::CCorePixelShader> pxShader;
  std::unique_ptr<renderer::CCoreConstantBuffer> constBuffer;
  std::unique_ptr<renderer::CCoreTexture2D> trailTexture;
  std::unique_ptr<renderer::CCoreSamplerState> trailSampler;
  std::unique_ptr<renderer::CCoreRasterizerState> trailRasterizer1;
  std::unique_ptr<renderer::CCoreRasterizerState> trailRasterizer2;
  std::unique_ptr<renderer::CCoreRasterizerState> trailRasterizer3;
  std::unique_ptr<renderer::CCoreDepthStencilState> trailDepthStencil;

  std::unique_ptr<GW2Trail> editedTrail;

  bool trailBeingRecorded = false;
  bool trailRecordPaused = false;

  std::mutex mtx;

  std::unordered_map<std::string, std::unique_ptr<renderer::CCoreTexture2D>>
      textureCache;
};

export typedef std::unordered_map<GUID, std::unique_ptr<GW2Trail>> TrailSet;
export extern std::unordered_map<int, TrailSet> trails;
export TrailSet& GetMapTrails();
