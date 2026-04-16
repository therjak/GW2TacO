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
#include "src/base/stream_reader.h"
#include "src/core2/device.h"
#include "src/gw2_tactical.h"

export module taco.trail_logger;

import math;
import taco.mumble_link;
import whiteboard;

export void GlobalDoTrailLogging(int32_t map_id, math::Vector3 charPos);

export struct GW2TrailVertex {
  math::Vector4 Pos;
  math::Vector2 UV;
  math::Vector4 CenterPos;
  CColor Color;
};

export class GW2TrailDisplay;

export class GW2Trail {
  friend class GW2TrailDisplay;

 public:
  virtual ~GW2Trail();
  void Build(renderer::Device* dev, int32_t map_id, const float* points,
             int pointCount);
  void Draw();
  void Update();
  void SetupAndDraw(renderer::ConstantBuffer* constBuffer,
                    renderer::Texture* texture, math::Matrix4x4& cam,
                    math::Matrix4x4& persp, float& one, bool scaleData,
                    int32_t fadeoutBubble, std::array<float, 8>& data,
                    float fadeAlpha, float width, float uvScale, float width2d);
  void SetCategory(GW2TacticalCategory* t);

  bool Import(CStreamReaderMemory& file, bool keepPoints = false);
  bool Import(std::string_view fileName, std::string_view zipFile,
              bool keepPoints = false);

  int32_t length = 0;
  std::unique_ptr<renderer::VertexBuffer> trailMesh;
  renderer::Device* dev = nullptr;
  std::unique_ptr<renderer::IndexBuffer> idxBuf;
  renderer::Texture* texture = nullptr;

  int32_t map = 0;

  MarkerTypeData typeData;
  std::string Type;
  GUID guid{};
  bool External = false;
  std::string zipFile;

  GW2TacticalCategory* category = nullptr;

 private:
  void Reset(int32_t _map_id = 0);
  bool SaveToFile(std::string_view fname);

  std::vector<math::Vector3> positions;
};

export class GW2TrailDisplay
    : public gui::CWBGuiType<"gw2Trails", gui::CWBItem> {
 public:
  GW2TrailDisplay();
  ~GW2TrailDisplay() override;
  static inline GW2TrailDisplay* Create(gui::CWBItem* Parent,
                                        math::Rect Position) {
    auto p = std::make_unique<GW2TrailDisplay>();
    p->Initialize(Parent, Position);
    GW2TrailDisplay* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  bool Initialize(gui::CWBItem* Parent, const math::Rect& Position) override;

  static gui::CWBItem* Factory(gui::CWBItem* Root, const CXMLNode& node,
                               math::Rect& Pos);

  bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                          gui::WBMESSAGE MessageType) override;

  void DoTrailLogging(int32_t map_id, math::Vector3 charPos);

  void StartStopTrailRecording(bool start);
  void PauseTrail(bool pause, bool newSection = false);
  void DeleteLastTrailSegment();
  void DeleteTrailSegment();
  void ExportTrail();
  void ImportTrail();

  void DrawProxy(gui::CWBDrawAPI* API, bool miniMaprender);

 private:
  void OnDraw(gui::CWBDrawAPI* API) override;
  void ClearEditedTrail();
  renderer::Texture2D* GetTexture(const std::string_view& fname,
                                  const std::string_view& zipFile,
                                  const std::string_view& categoryZip);

  float asp = 0;
  math::Matrix4x4 cam;
  math::Matrix4x4 persp;
  math::Rect drawrect;

  std::unique_ptr<renderer::VertexShader> vxShader;
  std::unique_ptr<renderer::VertexFormat> vertexFormat;
  std::unique_ptr<renderer::PixelShader> pxShader;
  std::unique_ptr<renderer::ConstantBuffer> constBuffer;
  std::unique_ptr<renderer::Texture2D> trailTexture;
  std::unique_ptr<renderer::SamplerState> trailSampler;
  std::unique_ptr<renderer::RasterizerState> trailRasterizer1;
  std::unique_ptr<renderer::RasterizerState> trailRasterizer2;
  std::unique_ptr<renderer::RasterizerState> trailRasterizer3;
  std::unique_ptr<renderer::DepthStencilState> trailDepthStencil;

  std::unique_ptr<GW2Trail> editedTrail;

  bool trailBeingRecorded = false;
  bool trailRecordPaused = false;

  std::mutex mtx;

  std::unordered_map<std::string, std::unique_ptr<renderer::Texture2D>>
      textureCache;
};

export typedef std::unordered_map<GUID, std::unique_ptr<GW2Trail>> TrailSet;
export extern std::unordered_map<int, TrailSet> trails;
export TrailSet& GetMapTrails();
