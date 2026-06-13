module;
#include <array>
#include <cassert>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
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
  math::Vector4 pos;
  math::Vector2 uv;
  math::Vector4 center_pos;
  Color color;
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
                    const math::Matrix4x4& persp, float one, bool scaleData,
                    int32_t fadeoutBubble, std::array<float, 8>* data,
                    float fadeAlpha, float width, float uvScale, float width2d);
  void SetCategory(GW2TacticalCategory* t);

  bool Import(const StreamReaderMemory& file, bool keepPoints = false);
  bool Import(std::string_view fileName, std::string_view zipFile,
              bool keepPoints = false);

  int32_t length_ = 0;
  std::unique_ptr<renderer::VertexBuffer> trail_mesh_;
  renderer::Device* dev_ = nullptr;
  std::unique_ptr<renderer::IndexBuffer> idx_buf_;
  renderer::Texture* texture_ = nullptr;

  int32_t map_ = 0;

  MarkerTypeData type_data_;
  std::string type_;
  GUID guid_{};
  bool external_ = false;
  std::string zip_file_;

  GW2TacticalCategory* category_ = nullptr;

 private:
  void Reset(int32_t _map_id = 0);
  bool SaveToFile(std::string_view fname);

  std::vector<math::Vector3> positions_;
};

export class GW2TrailDisplay : public gui::GuiType<"gw2Trails", gui::Widget> {
 public:
  GW2TrailDisplay();
  ~GW2TrailDisplay() override;
  static inline GW2TrailDisplay* Create(gui::Widget* Parent,
                                        math::Rect Position) {
    auto p = std::make_unique<GW2TrailDisplay>();
    p->Initialize(Parent, Position);
    GW2TrailDisplay* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  bool Initialize(gui::Widget* Parent, const math::Rect& Position) override;

  static gui::Widget* Factory(gui::Widget* Root, const XMLNode& node,
                              const math::Rect& Pos);

  bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                          gui::MessageType message_type) override;

  void DoTrailLogging(int32_t map_id, math::Vector3 charPos);

  void StartStopTrailRecording(bool start);
  void PauseTrail(bool pause, bool newSection = false);
  void DeleteLastTrailSegment();
  void DeleteTrailSegment();
  void ExportTrail();
  void ImportTrail();

  void DrawProxy(gui::DrawAPI* API, bool miniMaprender);

 private:
  void OnDraw(gui::DrawAPI* API) override;
  void ClearEditedTrail();
  renderer::Texture2D* GetTexture(const std::string_view& fname,
                                  const std::string_view& zipFile,
                                  const std::string_view& categoryZip);

  float asp_ = 0;
  math::Matrix4x4 cam_;
  math::Matrix4x4 persp_;
  math::Rect draw_rect_;

  std::unique_ptr<renderer::VertexShader> vx_shader_;
  std::unique_ptr<renderer::VertexFormat> vertex_format_;
  std::unique_ptr<renderer::PixelShader> px_shader_;
  std::unique_ptr<renderer::ConstantBuffer> const_buffer_;
  std::unique_ptr<renderer::Texture2D> trail_texture_;
  std::unique_ptr<renderer::SamplerState> trail_sampler_;
  std::unique_ptr<renderer::RasterizerState> trail_rasterizer_1_;
  std::unique_ptr<renderer::RasterizerState> trail_rasterizer_2_;
  std::unique_ptr<renderer::RasterizerState> trail_rasterizer_3_;
  std::unique_ptr<renderer::DepthStencilState> trail_depth_stencil_;

  std::unique_ptr<GW2Trail> edited_trail_;

  bool trail_being_recorded_ = false;
  bool trail_record_paused_ = false;

  std::mutex mtx_;

  std::unordered_map<std::string, std::unique_ptr<renderer::Texture2D>>
      texture_cache_;
};

export typedef std::unordered_map<GUID, std::unique_ptr<GW2Trail>> TrailSet;
export std::unordered_map<int, TrailSet> trails;
export TrailSet& GetMapTrails();
export bool disable_hooks = false;
export float global_opacity = 1.0f;
export float minimap_opacity = 1.0f;
