module;

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "src/base/color.h"
#include "src/base/logger.h"
#include "src/core2/core2.h"
#include "src/core2/enums.h"

module whiteboard;

import :application;
import :draw_api;
import math;

namespace gui {

constexpr int32_t VertexBufferRectCount = 4096;
constexpr int32_t VertexBufferVertexCount = VertexBufferRectCount * 4;

using math::Point;
using math::Rect;
using math::Size;
using math::Vector2;
using math::Vector4;

void DrawAPI::AddDisplayRect(
    const math::Rect& Rect, const float u1, const float v1, const float u2,
    const float v2,
    const CColor
        color /*, const CColor c2, const CColor c3, const CColor c4*/) {
  if (!opacity_) return;

  const math::Rect rect = Rect + offset_;

  if (!crop_rect_.Intersects(rect)) return;

  if (draw_mode_ != WBDRAWMODE::WBD_RECTANGLES) RenderDisplayList();
  draw_mode_ = WBDRAWMODE::WBD_RECTANGLES;

  float u1f = u1;
  float v1f = v1;
  float u2f = u2;
  float v2f = v2;

  const math::Rect Pos = crop_rect_.GetIntersection(rect);

  // need to cull UV
  if (Pos != rect) {
    const auto xs = static_cast<float>(rect.Width());
    const auto ys = static_cast<float>(rect.Height());
    const float us = u2 - u1;
    const float vs = v2 - v1;

    const float x1f = (Pos.x1 - rect.x1) / xs;
    const float y1f = (Pos.y1 - rect.y1) / ys;
    const float x2f = (Pos.x2 - rect.x1) / xs;
    const float y2f = (Pos.y2 - rect.y1) / ys;

    u1f = us * x1f + u1;
    v1f = vs * y1f + v1;
    u2f = us * x2f + u1;
    v2f = vs * y2f + v1;
  }

  CColor Color = color;
  Color.A() = (Color.A() * opacity_) / 255;

  const Vector2 TL(static_cast<float>(Pos.x1), static_cast<float>(Pos.y1));
  const Vector2 BR(static_cast<float>(Pos.x2), static_cast<float>(Pos.y2));

  display_list_.emplace_back(WBGUIVERTEX{TL.x, TL.y, u1f, v1f, Color});
  display_list_.emplace_back(WBGUIVERTEX{BR.x, TL.y, u2f, v1f, Color});
  display_list_.emplace_back(WBGUIVERTEX{BR.x, BR.y, u2f, v2f, Color});
  display_list_.emplace_back(WBGUIVERTEX{TL.x, BR.y, u1f, v2f, Color});
}

void DrawAPI::AddDisplayLine(const Point& _p1, const Point& _p2, const float u1,
                             const float v1, const float u2, const float v2,
                             const CColor a, const CColor b) {
  WBDISPLAYLINE r;

  const Point p1 = _p1 + offset_;
  const Point p2 = _p2 + offset_;

  Rect Area = Rect(p1, p2);
  Area.Normalize();
  const Rect rArea = crop_rect_ | Area;
  if (rArea.Width() < 0 || rArea.Height() < 0) return;
  if (draw_mode_ != WBDRAWMODE::WBD_LINES) RenderDisplayList();

  draw_mode_ = WBDRAWMODE::WBD_LINES;

  float t1 = 0;
  float t2 = 0;

  const auto xs = static_cast<float>(abs(p2.x - p1.x));
  const auto ys = static_cast<float>(abs(p2.y - p1.y));

  if (p1.x < rArea.x1) t1 = std::max(t1, (rArea.x1 - p1.x) / xs);
  if (p1.y < rArea.y1) t1 = std::max(t1, (rArea.y1 - p1.y) / ys);

  if (p1.x > rArea.x2) t1 = std::max(t1, (p1.x - rArea.x2) / xs);
  if (p1.y > rArea.y2) t1 = std::max(t1, (p1.y - rArea.y2) / ys);

  if (p2.x < rArea.x1) t2 = std::max(t2, (rArea.x1 - p2.x) / xs);
  if (p2.y < rArea.y1) t2 = std::max(t2, (rArea.y1 - p2.y) / ys);

  if (p2.x > rArea.x2) t2 = std::max(t2, (p2.x - rArea.x2) / xs);
  if (p2.y > rArea.y2) t2 = std::max(t2, (p2.y - rArea.y2) / ys);

  t2 = 1 - t2;

  if (t1 >= t2) return;

  r.p1 = math::Lerp(p1, p2, t1);
  r.p2 = math::Lerp(p1, p2, t2);

  r.u1 = math::Lerp(u1, u2, t1);
  r.u2 = math::Lerp(u1, u2, t2);
  r.v1 = math::Lerp(v1, v2, t1);
  r.v2 = math::Lerp(v1, v2, t2);

  r.c1 = Lerp(a, b, t1);
  r.c2 = Lerp(a, b, t2);

  display_list_.emplace_back(WBGUIVERTEX(Vector2(r.p1.x + 0.5f, r.p1.y - 0.5f),
                                         Vector2(r.u1, r.v1), r.c1));
  display_list_.emplace_back(WBGUIVERTEX(Vector2(r.p2.x + 0.5f, r.p2.y - 0.5f),
                                         Vector2(r.u2, r.v2), r.c2));
}

void DrawAPI::AddDisplayRectRotated(const math::Rect& Rect, const float u1,
                                    const float v1, const float u2,
                                    const float v2, const CColor color,
                                    float rotation) {
  if (!opacity_) return;

  const math::Rect rect = Rect + offset_;

  if (!crop_rect_.Intersects(rect)) return;

  if (draw_mode_ != WBDRAWMODE::WBD_RECTANGLES) RenderDisplayList();
  draw_mode_ = WBDRAWMODE::WBD_RECTANGLES;

  float u1f = u1;
  float v1f = v1;
  float u2f = u2;
  float v2f = v2;

  const math::Rect Pos = crop_rect_.GetIntersection(rect);

  // need to cull UV
  if (Pos != rect) {
    const auto xs = static_cast<float>(rect.Width());
    const auto ys = static_cast<float>(rect.Height());
    const float us = u2 - u1;
    const float vs = v2 - v1;

    const float x1f = (Pos.x1 - rect.x1) / xs;
    const float y1f = (Pos.y1 - rect.y1) / ys;
    const float x2f = (Pos.x2 - rect.x1) / xs;
    const float y2f = (Pos.y2 - rect.y1) / ys;

    u1f = us * x1f + u1;
    v1f = vs * y1f + v1;
    u2f = us * x2f + u1;
    v2f = vs * y2f + v1;
  }

  CColor Color = color;
  Color.A() = (Color.A() * opacity_) / 255;

  const Vector2 TL(static_cast<float>(Pos.x1), static_cast<float>(Pos.y1));
  const Vector2 BR(static_cast<float>(Pos.x2), static_cast<float>(Pos.y2));

  const Vector2 center = (TL + BR) / 2.0f;
  const auto a = Vector2(TL.x, TL.y).Rotated(center, rotation);
  const auto b = Vector2(BR.x, TL.y).Rotated(center, rotation);
  const auto c = Vector2(BR.x, BR.y).Rotated(center, rotation);
  const auto d = Vector2(TL.x, BR.y).Rotated(center, rotation);

  display_list_.emplace_back(WBGUIVERTEX{a.x, a.y, u1f, v1f, Color});
  display_list_.emplace_back(WBGUIVERTEX{b.x, b.y, u2f, v1f, Color});
  display_list_.emplace_back(WBGUIVERTEX{c.x, c.y, u2f, v2f, Color});
  display_list_.emplace_back(WBGUIVERTEX{d.x, d.y, u1f, v2f, Color});
}

WBGUIVERTEX Lerp(const WBGUIVERTEX& a, const WBGUIVERTEX& b, float t) {
  WBGUIVERTEX r;
  r.pos = math::Lerp(a.pos, b.pos, t);
  r.uv = math::Lerp(a.uv, b.uv, t);
  r.color = Lerp(a.color, b.color, t);
  return r;
}

void DrawAPI::ClipTriX(int32_t x, bool KeepRight,
                       std::array<WBGUIVERTEX, 6>& Vertices,
                       int32_t& VertexCount) {
  std::array<WBGUIVERTEX, 6> NewVertices;
  int32_t NewVertexCount = 0;

  if (VertexCount > 6) {
    Log_Err("Something got fucked up");
  }

  WBGUIVERTEX S = Vertices[VertexCount - 1];
  for (int32_t e = 0; e < VertexCount; e++) {
    WBGUIVERTEX E = Vertices[e];

    const bool eInside =
        (E.pos.x > x && KeepRight) || (E.pos.x < x && !KeepRight);
    const bool sInside =
        (S.pos.x > x && KeepRight) || (S.pos.x < x && !KeepRight);

    if (eInside) {
      if (!sInside) {
        NewVertices[NewVertexCount++] =
            Lerp(S, E, (x - S.pos.x) / (E.pos.x - S.pos.x));
      }
      NewVertices[NewVertexCount++] = E;
    } else if (sInside) {
      NewVertices[NewVertexCount++] =
          Lerp(S, E, (x - S.pos.x) / (E.pos.x - S.pos.x));
    }
    S = E;
  }

  for (int32_t y = 0; y < NewVertexCount; y++) {
    Vertices[y] = NewVertices[y];
  }
  VertexCount = NewVertexCount;
}

void DrawAPI::ClipTriY(int32_t y, bool KeepBottom,
                       std::array<WBGUIVERTEX, 6>& Vertices,
                       int32_t& VertexCount) {
  std::array<WBGUIVERTEX, 6> NewVertices;
  int32_t NewVertexCount = 0;

  WBGUIVERTEX S = Vertices[VertexCount - 1];
  for (int32_t e = 0; e < VertexCount; e++) {
    WBGUIVERTEX E = Vertices[e];

    const bool eInside =
        (E.pos.y > y && KeepBottom) || (E.pos.y < y && !KeepBottom);
    const bool sInside =
        (S.pos.y > y && KeepBottom) || (S.pos.y < y && !KeepBottom);

    if (eInside) {
      if (!sInside) {
        NewVertices[NewVertexCount++] =
            Lerp(S, E, (y - S.pos.y) / (E.pos.y - S.pos.y));
      }
      NewVertices[NewVertexCount++] = E;
    } else if (sInside) {
      NewVertices[NewVertexCount++] =
          Lerp(S, E, (y - S.pos.y) / (E.pos.y - S.pos.y));
    }
    S = E;
  }

  for (int32_t x = 0; x < NewVertexCount; x++) Vertices[x] = NewVertices[x];
  VertexCount = NewVertexCount;
}

void DrawAPI::AddDisplayTri(const Point& _p1, const Point& _p2,
                            const Point& _p3, const float u1, const float v1,
                            const float u2, const float v2, const float u3,
                            const float v3, const CColor a, const CColor b,
                            const CColor c) {
  Point p1 = _p1 + offset_;
  Point p2 = _p2 + offset_;
  Point p3 = _p3 + offset_;

  Rect Bound = Rect(p1, p1);
  Bound.x1 = std::min(std::min(p1.x, p2.x), p3.x);
  Bound.x2 = std::max(std::max(p1.x, p2.x), p3.x);
  Bound.y1 = std::min(std::min(p1.y, p2.y), p3.y);
  Bound.y2 = std::max(std::max(p1.y, p2.y), p3.y);

  Bound |= crop_rect_ | Bound;
  if (Bound.Width() <= 0 || Bound.Height() <= 0) return;

  if (draw_mode_ != WBDRAWMODE::WBD_TRIANGLES) RenderDisplayList();
  draw_mode_ = WBDRAWMODE::WBD_TRIANGLES;

  // max vertex count is 6, when all triangle edges are cut
  std::array<WBGUIVERTEX, 6> Vertices;
  int32_t VertexCount = 3;
  Vertices[0].pos =
      Vector4(static_cast<float>(p1.x), static_cast<float>(p1.y), 0, 1);
  Vertices[0].uv = Vector2(u1, v1);
  Vertices[0].color = a;
  Vertices[1].pos =
      Vector4(static_cast<float>(p2.x), static_cast<float>(p2.y), 0, 1);
  Vertices[1].uv = Vector2(u2, v2);
  Vertices[1].color = b;
  Vertices[2].pos =
      Vector4(static_cast<float>(p3.x), static_cast<float>(p3.y), 0, 1);
  Vertices[2].uv = Vector2(u3, v3);
  Vertices[2].color = c;

  if (!crop_rect_.Contains(p1) || !crop_rect_.Contains(p2) ||
      !crop_rect_.Contains(p3)) {
    ClipTriX(crop_rect_.x1, true, Vertices, VertexCount);
    ClipTriY(crop_rect_.y1, true, Vertices, VertexCount);
    ClipTriX(crop_rect_.x2, false, Vertices, VertexCount);
    ClipTriY(crop_rect_.y2, false, Vertices, VertexCount);
  }

  for (int32_t x = 0; x < VertexCount; x++) {
    const Vector2 p = Vector2(Vertices[x].pos.x, Vertices[x].pos.y);
    Vertices[x].pos = Vector4(p.x, p.y, 0, 1);
  }

  for (int32_t x = 2; x < VertexCount; x++) {
    display_list_.emplace_back(Vertices[0]);
    display_list_.emplace_back(Vertices[x - 1]);
    display_list_.emplace_back(Vertices[x]);
  }
}

void DrawAPI::RenderDisplayList() {
  if (display_list_.empty()) return;
  if (!vertex_buffer_) return;

  // update texture atlas if needed
  if (atlas_ &&
      device_->GetTexture(renderer::Sampler::kPs0) == atlas_->GetTexture()) {
    atlas_->UpdateTexture();
  }

  device_->SetVertexBuffer(vertex_buffer_.get(), 0);

  int32_t VxCount = display_list_.size();
  while (VxCount > 0) {
    const int32_t Count = std::min(VxCount, VertexBufferVertexCount);

    void* Buffer = nullptr;

    if (!vertex_buffer_->Lock(&Buffer, 0, Count * sizeof(WBGUIVERTEX),
                              renderer::kLockDiscard)) {
      Log_Err("[gui] Error locking UI Vertex Buffer during draw.");
      return;
    }

    memcpy(Buffer, &display_list_[display_list_.size() - VxCount],
           Count * sizeof(WBGUIVERTEX));

    if (!vertex_buffer_->UnLock()) {
      Log_Err("[gui] Error unlocking UI Vertex Buffer during draw.");
      return;
    }

    switch (draw_mode_) {
      case WBDRAWMODE::WBD_RECTANGLES:
        if (!device_->DrawIndexedTriangles(Count / 2, Count)) {
          Log_Err("[gui] Error drawing UI Triangles ({:d} {:d})", Count / 2,
                  Count);
          return;
        }
        break;
      case WBDRAWMODE::WBD_LINES:
        if (!device_->DrawLines(Count / 2)) {
          Log_Err("[gui] Error drawing UI Lines ({:d})", Count / 2);
          return;
        }
        break;
      case WBDRAWMODE::WBD_TRIANGLES:
        if (!device_->DrawTriangles(Count / 3)) {
          Log_Err("[gui] Error drawing UI Triangles ({:d})", Count / 3);
          return;
        }
        break;
      default:
        break;
    }

    VxCount -= Count;
  }

  display_list_.clear();
}

DrawAPI::DrawAPI()
    : offset_(0, 0),
      crop_rect_(0, 0, 0, 0),
      draw_mode_(WBDRAWMODE::WBD_RECTANGLES),
      opacity_(255),
      app_(nullptr),
      atlas_(nullptr),
      device_(nullptr) {}

DrawAPI::~DrawAPI() = default;

bool DrawAPI::Initialize(Application* Application, renderer::Device* Dev,
                         Atlas* atlas) {
  app_ = Application;
  device_ = Dev;
  atlas_ = atlas;
  if (!device_ || !atlas_) return false;

  vertex_buffer_ = device_->CreateVertexBufferDynamic(
      sizeof(WBGUIVERTEX) * VertexBufferRectCount * 4);
  if (!vertex_buffer_) {
    Log_Err("[gui] Error creating UI Vertex Buffer");
    return false;
  }

  rect_index_buffer_ = device_->CreateIndexBuffer(VertexBufferRectCount * 6);
  if (!rect_index_buffer_) {
    Log_Err("[gui] Error creating UI Index Buffer");
    return false;
  }

  uint16_t* Locked = nullptr;

  if (rect_index_buffer_->Lock(reinterpret_cast<void**>(&Locked))) {
    for (int32_t x = 0; x < VertexBufferRectCount; x++) {
      Locked[x * 6 + 0] = x * 4;
      Locked[x * 6 + 1] = x * 4 + 1;
      Locked[x * 6 + 2] = x * 4 + 2;
      Locked[x * 6 + 3] = x * 4;
      Locked[x * 6 + 4] = x * 4 + 2;
      Locked[x * 6 + 5] = x * 4 + 3;
    }
    if (!rect_index_buffer_->UnLock()) {
      Log_Err("[gui] Error unlocking UI Index Buffer during init");
      return false;
    }
  } else {
    Log_Err("[gui] Error locking UI Index Buffer during init");
    return false;
  }

  LPCSTR shader =
      "Texture2D GuiTexture:register(t0);"
      "SamplerState Sampler:register(s0);"
      "cbuffer resdata : register(b0)"
      "{"
      "    float4 resolution;"
      "}"
      "struct VSIN { float4 Position : POSITIONT; float2 UV : TEXCOORD0; "
      "float4 Color : COLOR0; };"
      "struct VSOUT { float4 Position : SV_POSITION; float2 UV : TEXCOORD0; "
      "float4 Color : COLOR0; };"
      "VSOUT vsmain(VSIN x) { VSOUT k; "
      "k.Position=float4(x.Position.x/resolution.x*2-1,-x.Position.y/"
      "resolution.y*2+1,0,1); k.UV=x.UV; k.Color=x.Color; return k; }"
      "float4 psmain(VSOUT x) : SV_TARGET0 { return "
      "x.Color*GuiTexture.Sample(Sampler,x.UV); }";

  unsigned char raw_ui_vxshader[] = {
      68,  88,  66,  67,  24,  203, 140, 68,  113, 109, 242, 251, 155, 3,   14,
      72,  212, 85,  154, 162, 1,   0,   0,   0,   176, 3,   0,   0,   5,   0,
      0,   0,   52,  0,   0,   0,   0,   1,   0,   0,   116, 1,   0,   0,   232,
      1,   0,   0,   52,  3,   0,   0,   82,  68,  69,  70,  196, 0,   0,   0,
      1,   0,   0,   0,   68,  0,   0,   0,   1,   0,   0,   0,   28,  0,   0,
      0,   0,   4,   254, 255, 0,   1,   0,   0,   144, 0,   0,   0,   60,  0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   1,   0,   0,   0,   1,   0,   0,   0,
      114, 101, 115, 100, 97,  116, 97,  0,   60,  0,   0,   0,   1,   0,   0,
      0,   92,  0,   0,   0,   16,  0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   116, 0,   0,   0,   0,   0,   0,   0,   16,  0,   0,   0,   2,
      0,   0,   0,   128, 0,   0,   0,   0,   0,   0,   0,   114, 101, 115, 111,
      108, 117, 116, 105, 111, 110, 0,   171, 1,   0,   3,   0,   1,   0,   4,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   77,  105, 99,  114, 111, 115,
      111, 102, 116, 32,  40,  82,  41,  32,  72,  76,  83,  76,  32,  83,  104,
      97,  100, 101, 114, 32,  67,  111, 109, 112, 105, 108, 101, 114, 32,  49,
      48,  46,  48,  46,  49,  48,  48,  49,  49,  46,  49,  54,  51,  56,  52,
      0,   73,  83,  71,  78,  108, 0,   0,   0,   3,   0,   0,   0,   8,   0,
      0,   0,   80,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   3,
      0,   0,   0,   0,   0,   0,   0,   15,  3,   0,   0,   90,  0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   3,   0,   0,   0,   1,   0,   0,
      0,   3,   3,   0,   0,   99,  0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   3,   0,   0,   0,   2,   0,   0,   0,   15,  15,  0,   0,   80,
      79,  83,  73,  84,  73,  79,  78,  84,  0,   84,  69,  88,  67,  79,  79,
      82,  68,  0,   67,  79,  76,  79,  82,  0,   171, 171, 171, 79,  83,  71,
      78,  108, 0,   0,   0,   3,   0,   0,   0,   8,   0,   0,   0,   80,  0,
      0,   0,   0,   0,   0,   0,   1,   0,   0,   0,   3,   0,   0,   0,   0,
      0,   0,   0,   15,  0,   0,   0,   92,  0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   3,   0,   0,   0,   1,   0,   0,   0,   3,   12,  0,
      0,   101, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   3,   0,
      0,   0,   2,   0,   0,   0,   15,  0,   0,   0,   83,  86,  95,  80,  79,
      83,  73,  84,  73,  79,  78,  0,   84,  69,  88,  67,  79,  79,  82,  68,
      0,   67,  79,  76,  79,  82,  0,   171, 83,  72,  68,  82,  68,  1,   0,
      0,   64,  0,   1,   0,   81,  0,   0,   0,   89,  0,   0,   4,   70,  142,
      32,  0,   0,   0,   0,   0,   1,   0,   0,   0,   95,  0,   0,   3,   50,
      16,  16,  0,   0,   0,   0,   0,   95,  0,   0,   3,   50,  16,  16,  0,
      1,   0,   0,   0,   95,  0,   0,   3,   242, 16,  16,  0,   2,   0,   0,
      0,   103, 0,   0,   4,   242, 32,  16,  0,   0,   0,   0,   0,   1,   0,
      0,   0,   101, 0,   0,   3,   50,  32,  16,  0,   1,   0,   0,   0,   101,
      0,   0,   3,   242, 32,  16,  0,   2,   0,   0,   0,   104, 0,   0,   2,
      1,   0,   0,   0,   14,  0,   0,   8,   18,  0,   16,  0,   0,   0,   0,
      0,   10,  16,  16,  0,   0,   0,   0,   0,   10,  128, 32,  0,   0,   0,
      0,   0,   0,   0,   0,   0,   50,  0,   0,   9,   18,  32,  16,  0,   0,
      0,   0,   0,   10,  0,   16,  0,   0,   0,   0,   0,   1,   64,  0,   0,
      0,   0,   0,   64,  1,   64,  0,   0,   0,   0,   128, 191, 14,  0,   0,
      9,   18,  0,   16,  0,   0,   0,   0,   0,   26,  16,  16,  128, 65,  0,
      0,   0,   0,   0,   0,   0,   26,  128, 32,  0,   0,   0,   0,   0,   0,
      0,   0,   0,   50,  0,   0,   9,   34,  32,  16,  0,   0,   0,   0,   0,
      10,  0,   16,  0,   0,   0,   0,   0,   1,   64,  0,   0,   0,   0,   0,
      64,  1,   64,  0,   0,   0,   0,   128, 63,  54,  0,   0,   8,   194, 32,
      16,  0,   0,   0,   0,   0,   2,   64,  0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   128, 63,  54,  0,   0,   5,
      50,  32,  16,  0,   1,   0,   0,   0,   70,  16,  16,  0,   1,   0,   0,
      0,   54,  0,   0,   5,   242, 32,  16,  0,   2,   0,   0,   0,   70,  30,
      16,  0,   2,   0,   0,   0,   62,  0,   0,   1,   83,  84,  65,  84,  116,
      0,   0,   0,   8,   0,   0,   0,   1,   0,   0,   0,   0,   0,   0,   0,
      6,   0,   0,   0,   4,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   3,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  };

  constexpr int raw_ui_vxshader_size = 944;

  unsigned char raw_ui_pxshader[] = {
      68,  88,  66,  67,  24,  211, 192, 194, 1,   98,  151, 138, 225, 231, 139,
      185, 192, 223, 161, 191, 1,   0,   0,   0,   160, 2,   0,   0,   5,   0,
      0,   0,   52,  0,   0,   0,   224, 0,   0,   0,   84,  1,   0,   0,   136,
      1,   0,   0,   36,  2,   0,   0,   82,  68,  69,  70,  164, 0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   2,   0,   0,   0,   28,  0,   0,
      0,   0,   4,   255, 255, 0,   1,   0,   0,   111, 0,   0,   0,   92,  0,
      0,   0,   3,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   1,   0,   0,   0,   1,   0,   0,   0,
      100, 0,   0,   0,   2,   0,   0,   0,   5,   0,   0,   0,   4,   0,   0,
      0,   255, 255, 255, 255, 0,   0,   0,   0,   1,   0,   0,   0,   13,  0,
      0,   0,   83,  97,  109, 112, 108, 101, 114, 0,   71,  117, 105, 84,  101,
      120, 116, 117, 114, 101, 0,   77,  105, 99,  114, 111, 115, 111, 102, 116,
      32,  40,  82,  41,  32,  72,  76,  83,  76,  32,  83,  104, 97,  100, 101,
      114, 32,  67,  111, 109, 112, 105, 108, 101, 114, 32,  49,  48,  46,  48,
      46,  49,  48,  48,  49,  49,  46,  49,  54,  51,  56,  52,  0,   171, 73,
      83,  71,  78,  108, 0,   0,   0,   3,   0,   0,   0,   8,   0,   0,   0,
      80,  0,   0,   0,   0,   0,   0,   0,   1,   0,   0,   0,   3,   0,   0,
      0,   0,   0,   0,   0,   15,  0,   0,   0,   92,  0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   3,   0,   0,   0,   1,   0,   0,   0,   3,
      3,   0,   0,   101, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      3,   0,   0,   0,   2,   0,   0,   0,   15,  15,  0,   0,   83,  86,  95,
      80,  79,  83,  73,  84,  73,  79,  78,  0,   84,  69,  88,  67,  79,  79,
      82,  68,  0,   67,  79,  76,  79,  82,  0,   171, 79,  83,  71,  78,  44,
      0,   0,   0,   1,   0,   0,   0,   8,   0,   0,   0,   32,  0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   3,   0,   0,   0,   0,   0,   0,
      0,   15,  0,   0,   0,   83,  86,  95,  84,  65,  82,  71,  69,  84,  0,
      171, 171, 83,  72,  68,  82,  148, 0,   0,   0,   64,  0,   0,   0,   37,
      0,   0,   0,   90,  0,   0,   3,   0,   96,  16,  0,   0,   0,   0,   0,
      88,  24,  0,   4,   0,   112, 16,  0,   0,   0,   0,   0,   85,  85,  0,
      0,   98,  16,  0,   3,   50,  16,  16,  0,   1,   0,   0,   0,   98,  16,
      0,   3,   242, 16,  16,  0,   2,   0,   0,   0,   101, 0,   0,   3,   242,
      32,  16,  0,   0,   0,   0,   0,   104, 0,   0,   2,   1,   0,   0,   0,
      69,  0,   0,   9,   242, 0,   16,  0,   0,   0,   0,   0,   70,  16,  16,
      0,   1,   0,   0,   0,   70,  126, 16,  0,   0,   0,   0,   0,   0,   96,
      16,  0,   0,   0,   0,   0,   56,  0,   0,   7,   242, 32,  16,  0,   0,
      0,   0,   0,   70,  14,  16,  0,   0,   0,   0,   0,   70,  30,  16,  0,
      2,   0,   0,   0,   62,  0,   0,   1,   83,  84,  65,  84,  116, 0,   0,
      0,   3,   0,   0,   0,   1,   0,   0,   0,   0,   0,   0,   0,   3,   0,
      0,   0,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  };

  constexpr int raw_ui_pxshader_size = 672;

  vx_shader_ = device_->CreateVertexShaderFromBlob(raw_ui_vxshader,
                                                   raw_ui_vxshader_size);
  if (!vx_shader_) {
    Log_Warn(
        "[gui] Couldn't compile GUI VertexShader - this won't affect DX9 "
        "functionality");
    if (device_->GetAPIType() == renderer::DeviceApi::kDx11) {
      return false;
    }
  }

  px_shader_ =
      device_->CreatePixelShaderFromBlob(raw_ui_pxshader, raw_ui_pxshader_size);
  if (!px_shader_) {
    Log_Warn(
        "[gui] Couldn't compile GUI PixelShader - this won't affect DX9 "
        "functionality");
    if (device_->GetAPIType() == renderer::DeviceApi::kDx11) {
      return false;
    }
  }

  std::vector<renderer::VertexAttribute> Att = {
      renderer::VertexAttribute::kPositionT4,
      renderer::VertexAttribute::kTexCoord2,
      renderer::VertexAttribute::kColor4,
  };

  vertex_format_ = device_->CreateVertexFormat(Att, vx_shader_.get());
  if (!vertex_format_) {
    Log_Err("[gui] Error creating UI Vertex Format");
    return false;
  }

  gui_sampler_ = device_->CreateSamplerState();
  gui_blend_state_ = device_->CreateBlendState();
  if (!gui_blend_state_) {
    Log_Err("[gui] Error creating UI Blend State");
    return false;
  }

  gui_blend_state_->SetBlendEnable(0, true);
  gui_blend_state_->SetSrcBlend(0, renderer::BlendFactor::kSrcAlpha);
  gui_blend_state_->SetDestBlend(0, renderer::BlendFactor::kInvSrcAlpha);
  gui_blend_state_->SetSrcBlendAlpha(0, renderer::BlendFactor::kOne);
  gui_blend_state_->SetDestBlendAlpha(0, renderer::BlendFactor::kInvSrcAlpha);

  gui_raster_state_ = device_->CreateRasterizerState();
  if (!gui_raster_state_) {
    Log_Err("[gui] Error creating UI Raster State");
    return false;
  }

  gui_raster_state_->SetAntialiasedLineEnable(true);
  gui_raster_state_->SetCullMode(renderer::CullMode::kNone);

  gui_zstate_ = device_->CreateDepthStencilState();
  if (!gui_zstate_) {
    Log_Err("[gui] Error creating UI Z State");
    return false;
  }

  gui_zstate_->SetDepthFunc(renderer::ComparisonFunction::kLessEqual);

  resolution_data_ = device_->CreateConstantBuffer();
  if (!resolution_data_) {
    Log_Err("[gui] Error creating UI Resolution Data Buffer");
    return false;
  }

  uv_offset_ = device_->GetUVOffset();

  return true;
}

void DrawAPI::SetOffset(const Point& p) { offset_ = p; }

void DrawAPI::SetCropRect(const Rect& r) { crop_rect_ = parent_crop_rect_ | r; }

void DrawAPI::DrawRect(const Rect& r, CColor Color) {
  const Point white = atlas_->GetWhitePixelUV();
  const float u = UVTRANSLATION(white.x, atlas_->GetXRes());
  const float v = UVTRANSLATION(white.y, atlas_->GetYRes());
  AddDisplayRect(r, u, v, u, v, Color);
}

void DrawAPI::DrawRectBorder(const Rect& r, CColor Color) {
  DrawRect(Rect(r.TopLeft(), r.BottomLeft() + Point(1, 0)), Color);
  DrawRect(Rect(r.TopLeft(), r.TopRight() + Point(0, 1)), Color);
  DrawRect(Rect(r.TopRight() - Point(1, 0), r.BottomRight()), Color);
  DrawRect(Rect(r.BottomLeft() - Point(0, 1), r.BottomRight()), Color);
}

void DrawAPI::DrawRectRotated(const Rect& r, float u1, float v1, float u2,
                              float v2, CColor Color, float rotation) {
  AddDisplayRectRotated(r, u1, v1, u2, v2, Color, rotation);
}

void DrawAPI::DrawRect(const Rect& r, float u1, float v1, float u2, float v2) {
  AddDisplayRect(r, u1, v1, u2, v2, CColor{0xffffffff});
}

void DrawAPI::DrawRect(const Rect& r, float u1, float v1, float u2, float v2,
                       CColor Color) {
  AddDisplayRect(r, u1, v1, u2, v2, Color);
}

void DrawAPI::DrawLine(const Point& p1, const Point& p2, CColor Color) {
  const Point white = atlas_->GetWhitePixelUV();
  AddDisplayLine(p1, p2, UVTRANSLATION(white.x, atlas_->GetXRes()),
                 UVTRANSLATION(white.y, atlas_->GetYRes()),
                 UVTRANSLATION(white.x, atlas_->GetXRes()),
                 UVTRANSLATION(white.y, atlas_->GetYRes()), Color, Color);
}

void DrawAPI::DrawLine(const Point& p1, const Point& p2, CColor Color1,
                       CColor Color2) {
  const Point white = atlas_->GetWhitePixelUV();
  AddDisplayLine(p1, p2, UVTRANSLATION(white.x, atlas_->GetXRes()),
                 UVTRANSLATION(white.y, atlas_->GetYRes()),
                 UVTRANSLATION(white.x, atlas_->GetXRes()),
                 UVTRANSLATION(white.y, atlas_->GetYRes()), Color1, Color2);
}

void DrawAPI::DrawTriangle(const Point& p1, const Point& p2, const Point& p3,
                           CColor Color) {
  const Point white = atlas_->GetWhitePixelUV();
  AddDisplayTri(p1, p2, p3, UVTRANSLATION(white.x, atlas_->GetXRes()),
                UVTRANSLATION(white.y, atlas_->GetYRes()),
                UVTRANSLATION(white.x, atlas_->GetXRes()),
                UVTRANSLATION(white.y, atlas_->GetYRes()),
                UVTRANSLATION(white.x, atlas_->GetXRes()),
                UVTRANSLATION(white.y, atlas_->GetYRes()), Color, Color, Color);
}

void DrawAPI::DrawTriangle(const Point& p1, const Point& p2, const Point& p3,
                           CColor a, CColor b, CColor c) {
  const Point white = atlas_->GetWhitePixelUV();
  AddDisplayTri(p1, p2, p3, UVTRANSLATION(white.x, atlas_->GetXRes()),
                UVTRANSLATION(white.y, atlas_->GetYRes()),
                UVTRANSLATION(white.x, atlas_->GetXRes()),
                UVTRANSLATION(white.y, atlas_->GetYRes()),
                UVTRANSLATION(white.x, atlas_->GetXRes()),
                UVTRANSLATION(white.y, atlas_->GetYRes()), a, b, c);
}

void DrawAPI::DrawTriangle(const Point& p1, const Point& p2, const Point& p3,
                           float u1, float v1, float u2, float v2, float u3,
                           float v3, CColor a, CColor b, CColor c) {
  AddDisplayTri(p1, p2, p3, u1, v1, u2, v2, u3, v3, a, b, c);
}

void DrawAPI::DrawTriangle(const Point& p1, const Point& p2, const Point& p3,
                           float u1, float v1, float u2, float v2, float u3,
                           float v3) {
  AddDisplayTri(p1, p2, p3, u1, v1, u2, v2, u3, v3, CColor{0xffffffff},
                CColor{0xffffffff}, CColor{0xffffffff});
}

void DrawAPI::SetUIRenderState() {
  device_->SetViewport(Rect(0, 0, app_->GetXRes(), app_->GetYRes()));
  device_->SetIndexBuffer(rect_index_buffer_.get());

  device_->SetRenderState(gui_blend_state_.get());
  device_->SetRenderState(gui_zstate_.get());
  device_->SetRenderState(gui_raster_state_.get());

  device_->SetVertexShader(vx_shader_.get());
  device_->SetVertexFormat(vertex_format_.get());
  device_->SetVertexBuffer(vertex_buffer_.get(), 0);
  device_->SetGeometryShader(nullptr);
  device_->SetHullShader(nullptr);
  device_->SetDomainShader(nullptr);
  device_->SetPixelShader(px_shader_.get());
  device_->SetTexture(renderer::Sampler::kPs0, atlas_->GetTexture());

  device_->SetRenderTarget(nullptr);

  if (gui_sampler_) gui_sampler_->Apply(renderer::Sampler::kPs0);

  resolution_data_->Reset();

  Vector4 Resolution = Vector4(static_cast<float>(app_->GetXRes()),
                               static_cast<float>(app_->GetYRes()), 0, 1);
  resolution_data_->AddData(&Resolution, sizeof(Vector4));
  resolution_data_->Upload();
  device_->SetShaderConstants(resolution_data_.get());
}

void DrawAPI::SetUIBlendState(
    std::unique_ptr<renderer::BlendState>&& BlendState) {
  gui_blend_state_.swap(BlendState);
}

void DrawAPI::SetUISamplerState(
    std::unique_ptr<renderer::SamplerState>&& SamplerState) {
  gui_sampler_.swap(SamplerState);
}

void DrawAPI::SetRenderView(Rect r) { device_->SetViewport(r); }

Size DrawAPI::GetAtlasElementSize(WBATLASHANDLE h) {
  if (!atlas_) return Size(0, 0);
  return atlas_->GetSize(h);
}

static int32_t defragmentReportCount = 0;

bool DrawAPI::RequestAtlasImageUse(WBATLASHANDLE h, Rect* r) {
  if (!atlas_) return false;

  if (!atlas_->RequestImageUse(h, r)) {
    Log_Dbg("Request Atlas Image Failed for {:d}", h);
    // atlas is full, attempt to fix

    // first flush display buffer to avoid problems from remapping the atlas
    RenderDisplayList();

    // next make space in the atlas
    atlas_->Optimize();

    // Try adding the image again
    if (!atlas_->RequestImageUse(h, r)) {
      // atlas is really, really, really full. fail.

      if (defragmentReportCount == 100) {
        Log_Err(
            "[gui] MID-FRAME TEXTURE ATLAS OPTIMIZATION REPORT COUNT REACHED "
            "100, STOPPING REPORTING");
      }

      if (defragmentReportCount < 100) {
        Log_Err(
            "[gui] GUI Texture Atlas too small to render current frame. "
            "Flushing mid-frame, thiss will result in reduced performance.");
      }

      defragmentReportCount++;

      atlas_->Reset();
      if (!atlas_->RequestImageUse(h, r)) {
        Log_Err("[gui] Image {:d} does not fit the atlas.", h);
        return false;
      }
    }
  }

  return true;
}

void DrawAPI::DrawAtlasElement(WBATLASHANDLE h, int32_t x, int32_t y,
                               CColor Color) {
  if (!atlas_) return;

  const Point pos = Point(x, y) + offset_;

  if (!crop_rect_.Intersects(Rect(pos, pos + atlas_->GetSize(h)))) {
    // pre-cull invisible items to spare unneeded atlas use
    return;
  }

  Rect UV;
  if (!RequestAtlasImageUse(h, &UV)) return;

  DrawRect(Rect(x, y, UV.Width() + x, UV.Height() + y),
           UVTRANSLATION(UV.x1, atlas_->GetXRes()),
           UVTRANSLATION(UV.y1, atlas_->GetYRes()),
           UVTRANSLATION(UV.x2, atlas_->GetXRes()),
           UVTRANSLATION(UV.y2, atlas_->GetYRes()), Color);
}

void DrawAPI::DrawAtlasElement(WBATLASHANDLE h, const Rect& Position,
                               bool TileX, bool TileY, bool StretchX,
                               bool StretchY, CColor Color /*=0xffffffff*/) {
  if (!atlas_) return;

  const Rect rect = crop_rect_ | (Position + offset_);
  if (rect.Width() <= 0 || rect.Height() <= 0) {
    // pre-cull invisible items to spare unneeded atlas use
    return;
  }

  Rect UV;
  if (!RequestAtlasImageUse(h, &UV)) return;

  Size tilesize = Position.Size();
  if (TileX) tilesize.x = UV.Width();
  if (TileY) tilesize.y = UV.Height();

  Vector2 uvmod = Vector2(0, 0);

  Rect target = Rect(Point(0, 0), UV.Size()) + Position.TopLeft();
  target.x2 = target.x1 + Position.Width();
  if (!TileX && StretchX) uvmod.x = 0.5;

  // crop unneeded tiles
  if (TileX) {
    if (target.x1 < crop_rect_.x1 - offset_.x) {
      target.x1 +=
          ((crop_rect_.x1 - offset_.x - target.x1) / tilesize.x) * tilesize.x;
    }
    if (target.x2 > crop_rect_.x2 - offset_.x) target.x2 = crop_rect_.x2;
  }

  target.y2 = target.y1 + Position.Height();
  if (!TileY && StretchY) uvmod.y = 0.5;

  // crop unneeded tiles
  if (TileY) {
    if (target.y1 < crop_rect_.y1 - offset_.y) {
      target.y1 +=
          ((crop_rect_.y1 - offset_.y - target.y1) / tilesize.y) * tilesize.y;
    }
    if (target.y2 > crop_rect_.y2 - offset_.y) target.y2 = crop_rect_.y2;
  }

  const Rect p = Rect(Point(0, 0), tilesize);

  const Rect cr = crop_rect_;
  SetCropRect(target + offset_);

  for (int32_t x = target.x1; x < target.x2; x += tilesize.x) {
    for (int32_t y = target.y1; y < target.y2; y += tilesize.y) {
      DrawRect(p + Point(x, y),
               UVTRANSLATION(UV.x1 + uvmod.x, atlas_->GetXRes()),
               UVTRANSLATION(UV.y1 + uvmod.y, atlas_->GetYRes()),
               UVTRANSLATION(UV.x2 - uvmod.x, atlas_->GetXRes()),
               UVTRANSLATION(UV.y2 - uvmod.y, atlas_->GetYRes()), Color);
    }
  }

  crop_rect_ = cr;
}

void DrawAPI::DrawAtlasElementRotated(WBATLASHANDLE h, const Rect& position,
                                      CColor Color, float rotation) {
  if (!atlas_) return;

  const Rect pos = position + offset_;

  if (!crop_rect_.Intersects(pos)) {
    // pre-cull invisible items to spare unneeded atlas use
    return;
  }

  Rect UV;
  if (!RequestAtlasImageUse(h, &UV)) return;

  DrawRectRotated(pos, UVTRANSLATION(UV.x1 + 0.5f, atlas_->GetXRes()),
                  UVTRANSLATION(UV.y1 + 0.5f, atlas_->GetYRes()),
                  UVTRANSLATION(UV.x2 - 0.5f, atlas_->GetXRes()),
                  UVTRANSLATION(UV.y2 - 0.5f, atlas_->GetYRes()), Color,
                  rotation);
}

void DrawAPI::SetCropToClient(const Widget* i) {
  if (!i) return;
  SetCropRect(GetCropRect() | i->ClientToScreen(i->GetClientRect()));
}

void DrawAPI::FlushDrawBuffer() { RenderDisplayList(); }
// DrawAPI definitions...

renderer::Device* DrawAPI::GetDevice() { return device_; }

renderer::ConstantBuffer* DrawAPI::GetResolutionData() {
  return resolution_data_.get();
}

void DrawAPI::SetPixelShader(std::unique_ptr<renderer::PixelShader>&& shader) {
  px_shader_.swap(shader);
}

void DrawAPI::SetOpacity(uint8_t o) { opacity_ = o; }

void ZoomToMouseCenter(Point& Offset, int32_t& Zoom, int32_t NewZoom,
                       Point ZoomCenter) {
  const float dz = NewZoom / static_cast<float>(Zoom);

  const Vector2 v = Vector2(static_cast<float>(ZoomCenter.x),
                            static_cast<float>(ZoomCenter.y)) *
                    (1 - dz);
  Offset += Point(static_cast<int32_t>(v.x), static_cast<int32_t>(v.y));

  Zoom = NewZoom;
}

void ZoomToMouseCenter(Point& Offset, float& Zoom, float NewZoom, Point Pos) {
  const float dz = NewZoom / Zoom;

  Offset.x = static_cast<int32_t>(Pos.x + Offset.x - Pos.x * dz);
  Offset.y = static_cast<int32_t>(Pos.y + Offset.y - Pos.y * dz);

  Zoom = NewZoom;
}

}  // namespace gui
