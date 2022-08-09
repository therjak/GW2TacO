#pragma once
#include <array>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/base/hasher.h"
#include "src/base/stream_reader.h"
#include "src/gw2_tactical.h"
#include "src/mumble_link.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

void GlobalDoTrailLogging(int32_t mapID, math::CVector3 charPos);

struct GW2TrailVertex {
  math::CVector4 Pos;
  math::CVector2 UV;
  math::CVector4 CenterPos;
  CColor Color;
};

class GW2Trail {
  friend class GW2TrailDisplay;

  std::vector<math::CVector3> positions;

  void Reset(int32_t _mapID = 0);

  bool SaveToFile(std::string_view fname);

 public:
  virtual ~GW2Trail();

  int32_t length = 0;
  std::unique_ptr<CCoreVertexBuffer> trailMesh;
  CCoreDevice* dev = nullptr;
  std::unique_ptr<CCoreIndexBuffer> idxBuf;
  CCoreTexture* texture = nullptr;

  int32_t map = 0;

  void Build(CCoreDevice* dev, int32_t mapID, float* points, int pointCount);
  void Draw();
  void Update();
  void SetupAndDraw(CCoreConstantBuffer* constBuffer, CCoreTexture* texture,
                    math::CMatrix4x4& cam, math::CMatrix4x4& persp, float& one,
                    bool scaleData, int32_t fadeoutBubble,
                    std::array<float, 8>& data, float fadeAlpha, float width,
                    float uvScale, float width2d);

  MarkerTypeData typeData;
  std::string Type;
  GUID guid;
  bool External = false;
  std::string zipFile;

  GW2TacticalCategory* category = nullptr;
  void SetCategory(CWBApplication* App, GW2TacticalCategory* t);

  bool Import(CStreamReaderMemory& file, bool keepPoints = false);
  bool Import(std::string_view fileName, std::string_view zipFile,
              bool keepPoints = false);
};

class GW2TrailDisplay : public CWBItem {
  float asp = 0;
  math::CMatrix4x4 cam;
  math::CMatrix4x4 persp;
  math::CRect drawrect;

  void OnDraw(CWBDrawAPI* API) override;

  std::unique_ptr<CCoreVertexShader> vxShader;
  std::unique_ptr<CCoreVertexFormat> vertexFormat;
  std::unique_ptr<CCorePixelShader> pxShader;
  std::unique_ptr<CCoreConstantBuffer> constBuffer;
  std::unique_ptr<CCoreTexture2D> trailTexture;
  std::unique_ptr<CCoreSamplerState> trailSampler;
  std::unique_ptr<CCoreRasterizerState> trailRasterizer1;
  std::unique_ptr<CCoreRasterizerState> trailRasterizer2;
  std::unique_ptr<CCoreRasterizerState> trailRasterizer3;
  std::unique_ptr<CCoreDepthStencilState> trailDepthStencil;

  std::unique_ptr<GW2Trail> editedTrail;

  void ClearEditedTrail();
  bool trailBeingRecorded = false;
  bool trailRecordPaused = false;

  std::mutex mtx;

  CCoreTexture2D* GetTexture(const std::string_view& fname,
                             const std::string_view& zipFile,
                             const std::string_view& categoryZip);

  std::unordered_map<std::string, std::unique_ptr<CCoreTexture2D>> textureCache;

 public:
  GW2TrailDisplay(CWBItem* Parent, math::CRect Position);
  static inline std::shared_ptr<GW2TrailDisplay> Create(CWBItem* Parent,
                                                        math::CRect Position) {
    auto p = std::make_shared<GW2TrailDisplay>(Parent, Position);
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~GW2TrailDisplay() override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("gw2Trails", CWBItem);

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
};

typedef std::unordered_map<GUID, std::unique_ptr<GW2Trail>> TrailSet;
extern std::unordered_map<int, TrailSet> trails;
TrailSet& GetMapTrails();
