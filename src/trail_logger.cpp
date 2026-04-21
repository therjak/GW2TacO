module;
#include <windows.h>
// windows header are stupid
#include <commdlg.h>

#include <algorithm>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/base/logger.h"
#include "src/base/stream_reader.h"
#include "src/base/stream_writer.h"
#include "src/core2/constant_buffer.h"
#include "src/core2/enums.h"
#include "src/core2/texture.h"
#include "src/gw2_tactical.h"
#include "src/util/miniz.h"

module taco.trail_logger;

import taco.mumble_link;
import taco.overlay_config;
import time;
import whiteboard;

#pragma comment(lib, "Comdlg32.lib")

constexpr int32_t kTrailFileVersion = 0;

using math::Matrix4x4;
using math::Point;
using math::Rect;
using math::Vector2;
using math::Vector3;
using math::Vector4;

extern float globalOpacity;
extern float minimapOpacity;

std::unordered_map<int, TrailSet> trails;

TrailSet& GetMapTrails() { return trails[mumbleLink.map_id]; }

extern std::unique_ptr<gui::CWBApplication> App;
CStreamWriterFile* TrailLog = nullptr;

int32_t lastMap = -1;
Vector3 last_pos = Vector3(0, 0, 0);

void GlobalDoTrailLogging(int32_t map_id, Vector3 charPos) {
  auto* trails = dynamic_cast<GW2TrailDisplay*>(
      App->GetRoot()->FindChildByID("trail", "gw2Trails"));
  if (trails) trails->DoTrailLogging(map_id, charPos);
}

void GW2TrailDisplay::DrawProxy(gui::CWBDrawAPI* API, bool miniMaprender) {
  int32_t fadeoutBubble = GetConfigValue("FadeoutBubble");

  draw_rect_ = GetClientRect();

  cam_.SetLookAtLH(mumbleLink.cam_position,
                   mumbleLink.cam_position + mumbleLink.cam_dir,
                   Vector3(0, 1, 0));
  persp_.SetPerspectiveFovLH(
      mumbleLink.fov,
      draw_rect_.Width() / static_cast<float>(draw_rect_.Height()), 0.01f,
      150.0f);
  asp_ = draw_rect_.Width() / static_cast<float>(draw_rect_.Height());

  // Matrix4x4 m = cam_*persp_;

  API->FlushDrawBuffer();

  App->GetDevice()->SetVertexShader(vx_shader_.get());
  App->GetDevice()->SetPixelShader(px_shader_.get());
  App->GetDevice()->SetVertexFormat(vertex_format_.get());
  trail_sampler_->Apply(renderer::Sampler::kPs0);
  trail_depth_stencil_->Apply();

  if (!HasConfigValue("ShowMinimapTrails")) {
    SetConfigValue("ShowMinimapTrails", 1);
  }
  int showMinimapTrails = GetConfigValue("ShowMinimapTrails");

  if (!HasConfigValue("ShowBigmapTrails")) {
    SetConfigValue("ShowBigmapTrails", 1);
  }
  int showBigmapTrails = GetConfigValue("ShowBigmapTrails");

  if (!HasConfigValue("ShowInGameTrails")) {
    SetConfigValue("ShowInGameTrails", 1);
  }
  int showIngameTrails = GetConfigValue("ShowInGameTrails");

  float one = 1;
  std::array<float, 8> data{};

  if (!miniMaprender && showIngameTrails > 0) {
    for (int x = 0; x < 2; x++) {
      if (x == 0) {
        trail_rasterizer_2_->Apply();
      } else {
        trail_rasterizer_1_->Apply();
      }

      std::lock_guard<std::mutex> lockGuard(mtx_);

      auto& mapTrails = GetMapTrails();
      for (auto& y : mapTrails) {
        auto& trail = *y.second;
        if (!trail.type_data_.bits_.in_game_visible_ && showIngameTrails != 2) {
          continue;
        }

        renderer::Texture* texture = nullptr;
        if (!trail.texture_) {
          const auto& str = trail.type_data_.texture_;

          if (!str.empty()) {
            texture =
                GetTexture(str, trail.zip_file_,
                           trail.category_ ? trail.category_->zip_file : "");
          } else {
            texture = trail_texture_.get();
          }
          trail.texture_ = texture;
        } else {
          texture = trail.texture_;
        }

        float width = GameToWorldCoords(20);

        trail.SetupAndDraw(const_buffer_.get(), texture, cam_, persp_, one,
                           x == 0, fadeoutBubble, data,
                           GetMapFade() * globalOpacity, width, width, 1.0f);
      }

      if (edited_trail_) {
        if (edited_trail_->map_ == mumbleLink.map_id) {
          data[0] = GetTime() / 1000.0f;

          App->GetDevice()->SetTexture(renderer::Sampler::kPs0,
                                       trail_texture_.get());

          const_buffer_->Reset();
          const auto& cam_data = cam_.data();
          const auto cam_size_x = cam_data[0].size() * sizeof(float);
          const_buffer_->AddData(cam_data[0].data(), cam_size_x);
          const_buffer_->AddData(cam_data[1].data(), cam_size_x);
          const_buffer_->AddData(cam_data[2].data(), cam_size_x);
          const_buffer_->AddData(cam_data[3].data(), cam_size_x);
          const auto& persp_data = persp_.data();
          const auto persp_size_x = persp_data[0].size() * sizeof(float);
          const_buffer_->AddData(persp_data[0].data(), persp_size_x);
          const_buffer_->AddData(persp_data[1].data(), persp_size_x);
          const_buffer_->AddData(persp_data[2].data(), persp_size_x);
          const_buffer_->AddData(persp_data[3].data(), persp_size_x);
          const_buffer_->AddData(&mumbleLink.char_position, 12);
          const_buffer_->AddData(&one, 4);
          const_buffer_->AddData(data.data(), 16);
          // color

          data[0] = 0.2f;
          data[1] = 0.7f;
          data[2] = 1.0f;
          data[3] = 0.8f;

          if (x == 0) {
            data[0] *= 0.5f;
            data[1] *= 0.5f;
            data[2] *= 0.5f;
          }

          const_buffer_->AddData(data.data(), 16);
          data[0] = 1000;
          data[1] = 1200;
          data[2] = static_cast<float>(fadeoutBubble);
          data[3] = GameToWorldCoords(20);
          data[4] = GameToWorldCoords(20);
          data[5] = 1.0f;
          const_buffer_->AddData(data.data(), 32);

          const_buffer_->Upload();
          App->GetDevice()->SetShaderConstants(const_buffer_.get());

          edited_trail_->Draw();
        }
      }
    }
  }

  // draw minimap
  if (miniMaprender) {
    trail_rasterizer_3_->Apply();
    Rect miniRect = GetMinimapRectangle();
    Rect clientRect = GetClientRect();

    float mapFade = GetMapFade();

    if (mapFade > 0 && showMinimapTrails > 0) {
      Matrix4x4 camera =
          mumbleLink.mini_map.BuildTransformationMatrix(miniRect, false);
      // camera *= Matrix4x4().Scaling( Vector3( 2.0f / clientRect.Width(),
      // -2.0f / clientRect.Height(), 0.0f ) ); camera *=
      // Matrix4x4().Translation( Vector3( -1.0f, -1.0f, 0.5 ) );

      camera *= Matrix4x4::Translation(-Vector3(
          static_cast<float>(miniRect.x1), static_cast<float>(miniRect.y1), 0));
      camera *= Matrix4x4::Scaling(Vector3(
          clientRect.Width() / static_cast<float>(miniRect.Width()),
          clientRect.Height() / static_cast<float>(miniRect.Height()), 0));
      camera *= Matrix4x4::Scaling(Vector3(2.0f / clientRect.Width(),
                                           -2.0f / clientRect.Height(), 0.0f));
      camera *= Matrix4x4::Translation(Vector3(-1.0f, 1.0f, 0.5));
      Matrix4x4 perspective;
      perspective.SetIdentity();

      API->SetRenderView(miniRect);

      auto& mapTrails = GetMapTrails();
      for (auto& y : mapTrails) {
        auto& trail = *y.second;
        if (!trail.type_data_.bits_.mini_map_visible_ && showMinimapTrails != 2) {
          continue;
        }

        float trailWidth = trail.type_data_.mini_map_size_ * 0.5f;
        if (trail.type_data_.bits_.scale_with_zoom_) {
          trailWidth /= mumbleLink.mini_map.map_scale;
        }

        renderer::Texture* texture = nullptr;
        if (!trail.texture_) {
          const auto& str = trail.type_data_.texture_;

          if (!str.empty()) {
            texture =
                GetTexture(str, trail.zip_file_,
                           trail.category_ ? trail.category_->zip_file : "");
          } else {
            texture = trail_texture_.get();
          }
          trail.texture_ = texture;
        } else {
          texture = trail.texture_;
        }

        float alpha =
            1.0f -
            std::max(0.0f,
                     std::min(1.0f, (mumbleLink.mini_map.map_scale -
                                     trail.type_data_.mini_map_fade_out_level_) /
                                        2.0f));

        trail.SetupAndDraw(const_buffer_.get(), texture, camera, perspective,
                           one, false, 0, data,
                           mapFade * alpha * minimapOpacity, 1.0f,
                           GameToWorldCoords(20) * 0.1f, trailWidth);
      }
    }

    if (mumbleLink.is_map_open && mapFade < 1.0 && showBigmapTrails > 0) {
      miniRect = GetClientRect();
      Matrix4x4 camera =
          mumbleLink.big_map.BuildTransformationMatrix(miniRect, true);

      camera *= Matrix4x4::Scaling(Vector3(2.0f / clientRect.Width(),
                                           -2.0f / clientRect.Height(), 0.0f));
      camera *= Matrix4x4::Translation(Vector3(-1.0f, 1.0f, 0.5));
      // camera *= Matrix4x4().Scaling( Vector3( 10, 10, 1 ) );
      Matrix4x4 perspective;
      perspective.SetIdentity();

      API->SetRenderView(miniRect);

      auto& mapTrails = GetMapTrails();
      for (auto& y : mapTrails) {
        auto& trail = *y.second;
        if (!trail.type_data_.bits_.big_map_visible_ && showBigmapTrails != 2) {
          continue;
        }

        float trailWidth = trail.type_data_.mini_map_size_ * 0.5f;
        if (trail.type_data_.bits_.scale_with_zoom_) {
          trailWidth /= mumbleLink.mini_map.map_scale;
        }

        renderer::Texture* texture = nullptr;
        if (!trail.texture_) {
          const auto& str = trail.type_data_.texture_;

          if (!str.empty()) {
            texture =
                GetTexture(str, trail.zip_file_,
                           trail.category_ ? trail.category_->zip_file : "");
          } else {
            texture = trail_texture_.get();
          }
          trail.texture_ = texture;
        } else {
          texture = trail.texture_;
        }

        float alpha =
            1.0f -
            std::max(0.0f,
                     std::min(1.0f, (mumbleLink.big_map.map_scale -
                                     trail.type_data_.mini_map_fade_out_level_) /
                                        2.0f));
        trail.SetupAndDraw(const_buffer_.get(), texture, camera, perspective,
                           one, false, 0, data,
                           (1.0f - mapFade) * alpha * minimapOpacity, 1.0f,
                           GameToWorldCoords(20) * 0.1f, trailWidth);
      }
    }

    API->SetRenderView(App->GetRoot()->GetWindowRect());
  }

  API->SetUIRenderState();
}

void GW2TrailDisplay::OnDraw(gui::CWBDrawAPI* API) {
  if (!HasConfigValue("TrailLayerVisible")) {
    SetConfigValue("TrailLayerVisible", 1);
  }

  if (!GetConfigValue("TrailLayerVisible")) return;

  if (!HasConfigValue("FadeoutBubble")) SetConfigValue("FadeoutBubble", 1);

  if (!HasConfigValue("TacticalLayerVisible")) {
    SetConfigValue("TacticalLayerVisible", 1);
  }

  if (!GetConfigValue("TacticalLayerVisible")) return;

  if (!mumbleLink.IsValid()) return;

  DrawProxy(API, false);

  if (GetConfigValue("LogTrails")) {
    gui::CWBFont* f = GetFont(GetState());
    int32_t ypos = math::Lerp(GetClientRect().y1, GetClientRect().y2, 0.25f);

    std::string_view s = "TacO is logging your trail.";

    Point pos = f->GetTextPosition(
        s, Rect(GetClientRect().x1, ypos, GetClientRect().x2, ypos),
        gui::WBTEXTALIGNMENTX::WBTA_CENTERX,
        gui::WBTEXTALIGNMENTY::WBTA_CENTERY, gui::WBTEXTTRANSFORM::WBTT_NONE,
        true);
    ypos += f->GetLineHeight();
    f->Write(API, s, pos, CColor{0xffff0000});
  }
}

void GW2TrailDisplay::DoTrailLogging(int32_t map_id, Vector3 charPos) {
  std::lock_guard<std::mutex> lockGuard(mtx_);

  if (!trail_being_recorded_) return;

  if (trail_record_paused_) return;

  if (map_id != lastMap) ClearEditedTrail();

  if (!edited_trail_) {
    edited_trail_ = std::make_unique<GW2Trail>();
    edited_trail_->Reset(map_id);
  }

  float dist = WorldToGameCoords((last_pos - charPos).Length());
  if (dist < 30) return;

  lastMap = map_id;
  edited_trail_->positions_.push_back(charPos);
  last_pos = charPos;

  edited_trail_->Update();
}

void GW2TrailDisplay::ClearEditedTrail() { edited_trail_.reset(); }

mz_zip_archive* OpenZipFile(std::string_view zipFile);

renderer::Texture2D* GW2TrailDisplay::GetTexture(
    const std::string_view& fname, const std::string_view& zipFile,
    const std::string_view& categoryZip) {
  std::string s =
      (zipFile.empty() ? (std::string(zipFile) + "\\") : std::string("")) +
      std::string(fname);
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (texture_cache_.find(s) != texture_cache_.end()) {
    const auto& tc = texture_cache_[s];
    if (tc) return tc.get();
    return trail_texture_.get();
  }

  if (!zipFile.empty() || !categoryZip.empty()) {
    // we didn't find an entry from within the zip file, try to load it

    for (int x = 0; x < 2; x++) {
      if (zipFile.empty() && x == 0) continue;

      if (categoryZip.empty() && x == 1) continue;

      mz_zip_archive* zip =
          x == 0 ? OpenZipFile(zipFile) : OpenZipFile(categoryZip);

      if (zip) {
        int idx = mz_zip_reader_locate_file(zip, s.c_str(), nullptr, 0);
        if (idx >= 0 && !mz_zip_reader_is_file_a_directory(zip, idx)) {
          mz_zip_archive_file_stat stat;
          if (mz_zip_reader_file_stat(zip, idx, &stat) &&
              stat.m_uncomp_size > 0) {
            auto data = std::make_unique<uint8_t[]>(
                static_cast<int32_t>(stat.m_uncomp_size));

            if (mz_zip_reader_extract_to_mem(
                    zip, idx, data.get(),
                    static_cast<int32_t>(stat.m_uncomp_size), 0)) {
              auto tex = App->GetDevice()->CreateTexture2D(
                  data.get(), static_cast<int32_t>(stat.m_uncomp_size));
              if (tex) {
                auto t = tex.get();
                texture_cache_[s] = std::move(tex);
                return t;
              } else {
                Log_Err(
                    "[GW2TacO] Failed to decompress image {:s} from archive "
                    "{:s}",
                    fname, x == 0 ? zipFile : categoryZip);
              }
            }
          }
        }
      }
    }

    // zipfile load failed, fall back to regular load and add it as an alias
    return GetTexture(s, "", "");
  }

  CStreamReaderMemory f;
  if (!f.Open(s) && !f.Open("POIs\\" + s)) {
    texture_cache_[s] = nullptr;
    Log_Err("[GW2TacO] Failed to open image {:s}", s);
    return trail_texture_.get();
  }

  auto texture = App->GetDevice()->CreateTexture2D(
      f.GetData(), static_cast<int32_t>(f.GetLength()));
  if (!texture) Log_Err("[GW2TacO] Failed to decompress image {:s}", s);
  texture_cache_[s] = std::move(texture);
  return texture_cache_[s].get();
}

GW2TrailDisplay::GW2TrailDisplay() : CWBGuiType() {}

bool GW2TrailDisplay::Initialize(gui::CWBItem* Parent,
                                 const math::Rect& Position) {
  if (!gui::CWBItem::Initialize(Parent, Position)) return false;

  const_buffer_ = App->GetDevice()->CreateConstantBuffer();

  CStreamReaderMemory tex;
  if (tex.Open("Data\\trail.png")) {
    trail_texture_ = App->GetDevice()->CreateTexture2D(
        tex.GetData(), static_cast<int32_t>(tex.GetLength()));
    if (!trail_texture_) {
      Log_Err("[GW2TacO] Failed to decompress trail texture image!");
    }
  } else {
    Log_Err("[GW2TacO] Failed to open trail texture!");
  }

  App->GetDevice()->SetShaderConstants(const_buffer_.get());
  trail_sampler_ = App->GetDevice()->CreateSamplerState();
  trail_sampler_->SetAddressU(renderer::TextureAddressMode::kWrap);
  trail_sampler_->SetAddressV(renderer::TextureAddressMode::kWrap);
  trail_sampler_->SetFilter(renderer::Filter::kAnisotropic);
  trail_sampler_->Update();

  trail_rasterizer_1_ = App->GetDevice()->CreateRasterizerState();
  trail_rasterizer_1_->SetCullMode(renderer::CullMode::kCcw);
  trail_rasterizer_1_->Update();

  trail_rasterizer_2_ = App->GetDevice()->CreateRasterizerState();
  trail_rasterizer_2_->SetCullMode(renderer::CullMode::kCw);
  trail_rasterizer_2_->Update();

  trail_rasterizer_3_ = App->GetDevice()->CreateRasterizerState();
  trail_rasterizer_3_->SetCullMode(renderer::CullMode::kNone);
  trail_rasterizer_3_->Update();

  trail_depth_stencil_ = App->GetDevice()->CreateDepthStencilState();
  trail_depth_stencil_->SetDepthEnable(false);
  trail_depth_stencil_->Update();

  LPCSTR code =
      "Texture2D GuiTexture:register(t0);"
      "SamplerState Sampler:register(s0);"
      "cbuffer resdata : register(b0)"
      "{							   "
      "		float4x4 camera;	   "
      "		float4x4 persp;   "
      "   float4   charpos;"
      "   float4   data;"
      "   float4   color;"
      "   float2   nearFarFades;"
      "   float    fadeOutBubble;"
      "   float    width;"
      "   float    uvScale;"
      "   float    width2d;"
      "}"
      "float rayspheredepth(float3 o, float3 d)"
      "{"
      "  if (fadeOutBubble==0) return 1;"
      "  float4 c = mul(camera,charpos + float4(0,1,0,0));"
      "  c/=c.w;"
      "  float r=2.00;"
      "  float3 oc=o-c.xyz;"
      "  float discriminant = dot(oc, d) * dot(oc, d) - ( dot( oc, oc ) - r * "
      "r ); "
      "  if (discriminant < 0) return 1;"
      "  float t1 = -dot(d, oc) - sqrt(discriminant);"
      "  float t2 = -dot(d, oc) + sqrt(discriminant);"
      "  return saturate(1-abs(t2-t1)/r/2.0);"
      "}"
      "struct VSIN { float4 Position : POSITIONT; float2 UV : TEXCOORD0; "
      "float4 Pos2 : TEXCOORD1; float4 Color : COLOR0; };"
      "struct VSOUT { float4 Position : SV_POSITION; float2 UV : TEXCOORD0; "
      "float4 Color : COLOR0; float4 p : TEXCOORD1; };"
      "VSOUT vsmain(VSIN x) { VSOUT k; "
      "k.p=k.Position=mul(camera,(x.Position-x.Pos2)*width+x.Pos2); "
      "k.p/=k.p.w; float4 p2 = mul(camera,x.Pos2); p2/=p2.w; "
      "k.Position=mul(persp,(k.Position-p2)*width2d+p2); "
      "k.UV=float2(x.UV.x,x.UV.y*uvScale); k.Color=x.Color; k.p/=k.p.w; return "
      "k; }"
      "float4 psmain(VSOUT x) : SV_TARGET0 {  float farFade = "
      "saturate(1.0-(x.p.z-nearFarFades.x)/(nearFarFades.y-nearFarFades.x)); "
      "float a=rayspheredepth(0,normalize(x.p.xyz)); a*=farFade; return "
      "x.Color*GuiTexture.Sample(Sampler,x.UV + "
      "float2(0,data.x))*color*float4(1,1,1,a); }";

  vx_shader_ = App->GetDevice()->CreateVertexShader(
      code, static_cast<int32_t>(strlen(code)), "vsmain", "vs_4_0");
  px_shader_ = App->GetDevice()->CreatePixelShader(
      code, static_cast<int32_t>(strlen(code)), "psmain", "ps_4_0");

  std::vector<renderer::VertexAttribute> Att{
      renderer::VertexAttribute::kPositionT4,
      renderer::VertexAttribute::kTexCoord2,
      renderer::VertexAttribute::kTexCoord4,
      renderer::VertexAttribute::kColor4,
  };

  vertex_format_ = App->GetDevice()->CreateVertexFormat(Att, vx_shader_.get());
  if (!vertex_format_) {
    Log_Err("[GW2TacO]  Error creating Trail Vertex Format");
  }
  return true;
}

GW2TrailDisplay::~GW2TrailDisplay() { texture_cache_.clear(); }

gui::CWBItem* GW2TrailDisplay::Factory(gui::CWBItem* Root, const CXMLNode& node,
                                       Rect& Pos) {
  return GW2TrailDisplay::Create(Root, Pos);
}

bool GW2TrailDisplay::IsMouseTransparent(const Point& ClientSpacePoint,
                                         gui::WBMESSAGE MessageType) {
  return true;
}

void GW2TrailDisplay::StartStopTrailRecording(bool start) {
  trail_being_recorded_ = start;
  if (!trail_being_recorded_) ClearEditedTrail();
}

void GW2TrailDisplay::PauseTrail(bool pause, bool newSection) {
  trail_record_paused_ = pause;

  auto* btn = App->GetRoot()->FindChildByID<gui::CWBButton>("pausetrail");
  if (btn) {
    btn->Push(pause);
    btn->SetText(btn->IsPushed() ? "Resume Recording" : "Pause Recording");
  }

  btn = App->GetRoot()->FindChildByID<gui::CWBButton>("startnewsection");
  if (btn) btn->Hide(!pause);

  if (!pause && newSection && edited_trail_) {
    edited_trail_->positions_.emplace_back(Vector3(0, 0, 0));
  }
}

void GW2TrailDisplay::DeleteLastTrailSegment() {
  if (!edited_trail_) return;

  if (!edited_trail_->positions_.empty()) edited_trail_->positions_.pop_back();

  edited_trail_->Update();
}

void GW2TrailDisplay::DeleteTrailSegment() {}

extern bool disableHooks;

void GW2TrailDisplay::ExportTrail() {
  if (!edited_trail_) return;

  disableHooks = true;

  TCHAR dir[1024];
  if (!GetCurrentDirectory(1024, dir)) memset(dir, 0, sizeof(TCHAR) * 1024);
  char Filestring[256] = {};

  OPENFILENAME opf = {};
  opf.hwndOwner = App->GetHandle();
  opf.lpstrFilter = "GW2 Taco Trail Files\0*.trl\0\0";
  opf.lpstrCustomFilter = nullptr;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[0] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = nullptr;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Save Trail";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "trl";
  opf.lpfnHook = nullptr;
  opf.lCustData = 0;
  opf.Flags = (OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NONETWORKBUTTON) &
              ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof(OPENFILENAME);

  opf.hInstance = GetModuleHandle(nullptr);
  opf.pvReserved = nullptr;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  opf.lpstrInitialDir = dir;

  if (GetSaveFileName(&opf)) {
    edited_trail_->SaveToFile(opf.lpstrFile);
  } else {
    DWORD error = CommDlgExtendedError();
  }

  SetCurrentDirectory(dir);

  disableHooks = false;
}

void GW2TrailDisplay::ImportTrail() {
  disableHooks = true;

  TCHAR dir[1024];
  if (!GetCurrentDirectory(1024, dir)) memset(dir, 0, sizeof(TCHAR) * 1024);
  char Filestring[256] = {};

  OPENFILENAME opf = {};
  opf.hwndOwner = App->GetHandle();
  opf.lpstrFilter = "GW2 Taco Trail Files\0*.trl\0\0";
  opf.lpstrCustomFilter = nullptr;
  opf.nMaxCustFilter = 0L;
  opf.nFilterIndex = 1L;
  opf.lpstrFile = Filestring;
  opf.lpstrFile[0] = '\0';
  opf.nMaxFile = 256;
  opf.lpstrFileTitle = nullptr;
  opf.nMaxFileTitle = 50;
  opf.lpstrInitialDir = "Data";
  opf.lpstrTitle = "Import Trail";
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = "trl";
  opf.lpfnHook = nullptr;
  opf.lCustData = 0;
  opf.Flags = (OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON) &
              ~OFN_ALLOWMULTISELECT;
  opf.lStructSize = sizeof(OPENFILENAME);

  opf.hInstance = GetModuleHandle(nullptr);
  opf.pvReserved = nullptr;
  opf.dwReserved = 0;
  opf.FlagsEx = 0;

  if (GetOpenFileName(&opf)) {
    CStreamReaderMemory file;
    if (file.Open(opf.lpstrFile)) {
      StartStopTrailRecording(true);
      ClearEditedTrail();
      PauseTrail(true);

      auto* btn = App->GetRoot()->FindChildByID<gui::CWBButton>("starttrail");
      if (btn) btn->Push(true);
      btn = App->GetRoot()->FindChildByID<gui::CWBButton>("pausetrail");
      if (btn) btn->Push(true);

      if (!edited_trail_) {
        edited_trail_ = std::make_unique<GW2Trail>();
        edited_trail_->Import(file, true);
      }

      lastMap = mumbleLink.map_id;
      last_pos = mumbleLink.char_position;
    }
  }

  SetCurrentDirectory(dir);

  disableHooks = false;
}

void GW2Trail::Reset(int32_t _map_id /*= 0 */) {
  map_ = _map_id;
  positions_.clear();
}

bool GW2Trail::SaveToFile(std::string_view fname) {
  if (positions_.empty()) return false;

  CStreamWriterFile TrailLog;
  if (!TrailLog.Open(fname)) return false;

  TrailLog.WriteDWord(kTrailFileVersion);

  TrailLog.WriteDWord(map_);
  TrailLog.Write(std::string_view(reinterpret_cast<const char*>(&positions_[0]),
                                  sizeof(Vector3) * positions_.size()));

  return true;
}

GW2Trail::~GW2Trail() = default;

void GW2Trail::Build(renderer::Device* d, int32_t map_id, const float* points,
                     int pointCount) {
  dev_ = d;
  map_ = map_id;

  trail_mesh_.reset();
  idx_buf_.reset();

  if (pointCount <= 1) return;

  auto vertices = std::make_unique<GW2TrailVertex[]>(size_t(pointCount) * 2);
  memset(vertices.get(), 0, sizeof(GW2TrailVertex) * pointCount * 2);
  int vertexCount = 0;
  auto indices = std::make_unique<int32_t[]>((size_t(pointCount) - 1) * 6);

  auto last_pos = Vector3(points);
  Vector3 lastOrt = Vector3(0, 0, 0);

  float uvStretch = 0;

  int cnt = 0;
  int icnt = 0;

  float twist = 1;

  for (int32_t x = 0; x < pointCount; x++) {
    auto pos = Vector3(points + x * 3);

    if (pos == Vector3(0, 0, 0)) {
      if (x + 1 >= pointCount) break;
      pos = last_pos = Vector3(points + (x + 1) * 3);
      twist = 1;
      lastOrt = Vector3(0, 0, 0);
    }

    Vector3 nextPos = Vector3(points + std::min(pointCount - 1, x + 1) * 3);

    if (nextPos == Vector3(0, 0, 0)) nextPos = pos;

    uvStretch += (pos - last_pos).Length() * type_data_.trail_scale_ * 2;

    Vector3 dir = nextPos - last_pos;
    dir.y = 0;
    float dirLen = dir.Length();
    dir /= dirLen;
    Vector3 ort = Vector3::Cross(dir, Vector3(0, 1, 0)).Normalized();

    if (lastOrt != Vector3(0, 0, 0) && Vector3::Dot(ort, lastOrt) < 0) {
      twist *= -1;
    }

    Vector3 p1 = pos + ort * twist;
    Vector3 p2 = pos - ort * twist;

    const auto vertPos = size_t(cnt) * 2;
    vertices[vertPos].pos = Vector4(p1.x, p1.y, p1.z, 1);
    vertices[vertPos + 1].pos = Vector4(p2.x, p2.y, p2.z, 1);
    vertices[vertPos].center_pos = Vector4(pos.x, pos.y, pos.z, 1);
    vertices[vertPos + 1].center_pos = Vector4(pos.x, pos.y, pos.z, 1);
    vertices[vertPos].color = CColor{0xffffffff};
    vertices[vertPos + 1].color = CColor{0xffffffff};
    vertices[vertPos].uv = Vector2(0, -uvStretch);
    vertices[vertPos + 1].uv = Vector2(1, -uvStretch);

    if (x < pointCount - 1) {
      indices[icnt++] = x * 2;
      if (twist > 0) {
        indices[icnt++] = x * 2 + 2;
        indices[icnt++] = x * 2 + 1;
      } else {
        indices[icnt++] = x * 2 + 1;
        indices[icnt++] = x * 2 + 2;
      }

      indices[icnt++] = x * 2 + 2;
      if (twist > 0) {
        indices[icnt++] = x * 2 + 3;
        indices[icnt++] = x * 2 + 1;
      } else {
        indices[icnt++] = x * 2 + 1;
        indices[icnt++] = x * 2 + 3;
      }
    }

    cnt++;
    last_pos = pos;

    lastOrt = ort;
    vertexCount += 2;
  }

  trail_mesh_ =
      dev_->CreateVertexBuffer(reinterpret_cast<uint8_t*>(vertices.get()),
                               vertexCount * sizeof(GW2TrailVertex));
  length_ = pointCount * 2;
  idx_buf_ = dev_->CreateIndexBuffer((pointCount - 1) * 6, 4);

  int32_t* idxData = nullptr;

  if (idx_buf_ && idx_buf_->Lock(reinterpret_cast<void**>(&idxData))) {
    memcpy(idxData, indices.get(),
           sizeof(int32_t) * 6 * (size_t(pointCount) - 1));
    // int cnt = 0;
    // for ( int32_t x = 0; x < pointCount - 1; x++ )
    //{
    //  idxData[ cnt++ ] = x * 2;
    //  idxData[ cnt++ ] = x * 2 + 2;
    //  idxData[ cnt++ ] = x * 2 + 1;
    //  idxData[ cnt++ ] = x * 2 + 2;
    //  idxData[ cnt++ ] = x * 2 + 3;
    //  idxData[ cnt++ ] = x * 2 + 1;
    //}
    idx_buf_->UnLock();
  }
}

void GW2Trail::Draw() {
  if (!trail_mesh_ || !idx_buf_) return;

  dev_->SetVertexBuffer(trail_mesh_.get(), 0);
  dev_->SetIndexBuffer(idx_buf_.get());
  dev_->DrawIndexedTriangles(length_ - 2, length_);
}

void GW2Trail::Update() {
  if (!App->GetDevice()) return;

  Build(App->GetDevice(), map_, reinterpret_cast<float*>(&positions_[0]),
        positions_.size());
}

void GW2Trail::SetupAndDraw(renderer::ConstantBuffer* const_buffer_,
                            renderer::Texture* texture, Matrix4x4& cam_,
                            Matrix4x4& persp_, float& one, bool scaleData,
                            int32_t fadeoutBubble, std::array<float, 8>& data,
                            float fadeAlpha, float width, float uvScale,
                            float width2d) {
  if (category_ && !category_->IsVisible()) return;

  if (map_ != mumbleLink.map_id) return;

  App->GetDevice()->SetTexture(renderer::Sampler::kPs0, texture);

  data[0] = GetTime() / 1000.0f;

  data[0] *= type_data_.anim_speed_;

  const_buffer_->Reset();
  const auto& cam_data = cam_.data();
  const auto cam_size_x = cam_data[0].size() * sizeof(float);
  const_buffer_->AddData(cam_data[0].data(), cam_size_x);
  const_buffer_->AddData(cam_data[1].data(), cam_size_x);
  const_buffer_->AddData(cam_data[2].data(), cam_size_x);
  const_buffer_->AddData(cam_data[3].data(), cam_size_x);
  const auto& persp_data = persp_.data();
  const auto persp_size_x = persp_data[0].size() * sizeof(float);
  const_buffer_->AddData(persp_data[0].data(), persp_size_x);
  const_buffer_->AddData(persp_data[1].data(), persp_size_x);
  const_buffer_->AddData(persp_data[2].data(), persp_size_x);
  const_buffer_->AddData(persp_data[3].data(), persp_size_x);
  const_buffer_->AddData(&mumbleLink.char_position, 12);
  const_buffer_->AddData(&one, 4);
  const_buffer_->AddData(data.data(), 16);
  // color

  data[0] = type_data_.color_.R() / 255.0f;
  data[1] = type_data_.color_.G() / 255.0f;
  data[2] = type_data_.color_.B() / 255.0f;
  data[3] = type_data_.alpha_ * fadeAlpha;

  if (scaleData) {
    data[0] *= 0.5;
    data[1] *= 0.5;
    data[2] *= 0.5;
  }

  const_buffer_->AddData(data.data(), 16);

  data[0] = GameToWorldCoords(type_data_.fade_near_);
  data[1] = GameToWorldCoords(type_data_.fade_far_);
  data[2] = static_cast<float>(fadeoutBubble);
  data[3] = width;
  data[4] = uvScale;
  data[5] = width2d;

  const_buffer_->AddData(data.data(), 32);

  const_buffer_->Upload();
  App->GetDevice()->SetShaderConstants(const_buffer_);

  Draw();
}

void GW2Trail::SetCategory(GW2TacticalCategory* t) {
  category_ = t;
  type_data_ = t->data;
  type_ = t->GetFullTypeName();
}

bool GW2Trail::Import(CStreamReaderMemory& f, bool keepPoints) {
  if (keepPoints) {
    positions_.clear();
    for (int32_t x = 0; x < (f.GetLength() - 8) / 12; x++) {
      positions_.emplace_back(
          Vector3(&(reinterpret_cast<const float*>(f.GetData() + 8))[x * 3]));
    }
  }

  Build(App->GetDevice(), *reinterpret_cast<const int32_t*>(f.GetData() + 4),
        reinterpret_cast<const float*>(f.GetData() + 8),
        static_cast<int32_t>((f.GetLength() - 8) / 12));
  return true;
}

bool GW2Trail::Import(std::string_view fileName, std::string_view zipFile,
                      bool keepPoints /*= false */) {
  if (!zipFile.empty()) {
    mz_zip_archive* zip = OpenZipFile(zipFile);

    if (zip) {
      int idx = mz_zip_reader_locate_file(zip, fileName.data(), nullptr, 0);
      if (idx >= 0 && !mz_zip_reader_is_file_a_directory(zip, idx)) {
        mz_zip_archive_file_stat stat;
        if (mz_zip_reader_file_stat(zip, idx, &stat) &&
            stat.m_uncomp_size > 0) {
          auto data = std::make_unique<uint8_t[]>(
              static_cast<int32_t>(stat.m_uncomp_size));

          if (mz_zip_reader_extract_to_mem(
                  zip, idx, data.get(),
                  static_cast<int32_t>(stat.m_uncomp_size), 0)) {
            CStreamReaderMemory f;
            if (f.Open(data.get(), static_cast<int32_t>(stat.m_uncomp_size))) {
              if (Import(f, keepPoints)) return true;
            }
          }
        }
      }
    }
  }

  CStreamReaderMemory f;
  if (!f.Open(fileName) && !f.Open("POIs\\" + std::string(fileName))) {
    Log_Err("[GW2TacO] Failed to open trail data file {:s}", fileName);
    return false;
  }

  return Import(f, keepPoints);
}
