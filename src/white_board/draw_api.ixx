module;

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "src/base/color.h"
#include "src/core2/core2.h"

export module whiteboard:draw_api;

import :atlas;
import :font;
import math;

export namespace gui {

class Application;
class Widget;

struct DisplayLine {
  math::Point p1, p2;
  float u1 = 0, v1 = 0, u2 = 0, v2 = 0;
  CColor c1, c2;
};

struct Vertex {
  math::Vector4 pos;
  math::Vector2 uv;
  CColor color;

  Vertex() = default;

  Vertex(const float x, const float y, const float u, const float v,
         const CColor& color)
      : pos(x, y, 0, 1), uv(u, v), color(color) {}

  Vertex(const math::Vector2& pos, const math::Vector2& uv, const CColor& color)
      : pos(pos.x, pos.y, 0, 1), uv(uv.x, uv.y), color(color) {}
};

enum class DrawMode : uint8_t { WBD_RECTANGLES, WBD_LINES, WBD_TRIANGLES };

class DrawAPI {
  friend class Application;
  // these are the only things that need to access the
  // renderdisplaylist function

  friend class Widget;
  // screen space top left corner of the client rect for
  // the currently drawn item

 public:
  DrawAPI();
  virtual ~DrawAPI();

  bool Initialize(Application* App, renderer::Device* Device, Atlas* Atlas);

  void DrawRect(const math::Rect& r, CColor color);
  void DrawRect(const math::Rect& r, float u1, float v1, float u2, float v2);
  void DrawRect(const math::Rect& r, float u1, float v1, float u2, float v2,
                CColor color);
  void DrawRectBorder(const math::Rect& r, CColor color);
  void DrawRectRotated(const math::Rect& r, float u1, float v1, float u2,
                       float v2, CColor color, float rotation);

  void DrawLine(const math::Point& p1, const math::Point& p2, CColor color);
  void DrawLine(const math::Point& p1, const math::Point& p2, CColor color1,
                CColor color2);

  void DrawTriangle(const math::Point& p1, const math::Point& p2,
                    const math::Point& p3, CColor color);
  void DrawTriangle(const math::Point& p1, const math::Point& p2,
                    const math::Point& p3, CColor a, CColor b, CColor c);
  void DrawTriangle(const math::Point& p1, const math::Point& p2,
                    const math::Point& p3, float u1, float v1, float u2,
                    float v2, float u3, float v3, CColor a, CColor b, CColor c);
  void DrawTriangle(const math::Point& p1, const math::Point& p2,
                    const math::Point& p3, float u1, float v1, float u2,
                    float v2, float u3, float v3);

  void SetOffset(const math::Point& p);
  void SetCropRect(const math::Rect& r);
  void SetParentCropRect(const math::Rect& r) { parent_crop_rect_ = r; };
  math::Rect& GetCropRect() { return crop_rect_; }
  math::Rect& GetParentCropRect() { return parent_crop_rect_; }
  math::Point& GetOffset() { return offset_; }

  void SetUIRenderState();
  void SetUIBlendState(std::unique_ptr<renderer::BlendState>&& BlendState);
  void SetUISamplerState(
      std::unique_ptr<renderer::SamplerState>&& SamplerState);

  math::Size GetAtlasElementSize(AtlasHandle h);
  void DrawAtlasElement(AtlasHandle h, int32_t x, int32_t y,
                        CColor color = CColor(0xffffffff));
  void DrawAtlasElement(AtlasHandle h, const math::Rect& Position, bool TileX,
                        bool TileY, bool StretchX, bool StretchY,
                        CColor color = CColor(0xffffffff));
  void DrawAtlasElementRotated(AtlasHandle h, const math::Rect& Position,
                               CColor color, float rotation);
  void SetCropToClient(const Widget* i);

  void FlushDrawBuffer();
  renderer::Device* GetDevice();
  renderer::ConstantBuffer* GetResolutionData();

  void SetOpacity(uint8_t o);
  void SetRenderView(math::Rect r);

  void SetPixelShader(std::unique_ptr<renderer::PixelShader>&& shader);

 private:
  void AddDisplayRect(const math::Rect& r, const float u1, const float v1,
                      const float u2, const float v3, const CColor a);
  void AddDisplayRectRotated(const math::Rect& r, const float u1,
                             const float v1, const float u2, const float v3,
                             const CColor a, float rotation);
  void AddDisplayLine(const math::Point& p1, const math::Point& p2,
                      const float u1, const float v1, const float u2,
                      const float v2, const CColor a, const CColor b);
  void AddDisplayTri(const math::Point& p1, const math::Point& p2,
                     const math::Point& p3, const float u1, const float v1,
                     const float u2, const float v2, const float u3,
                     const float v3, const CColor a, const CColor b,
                     const CColor c);
  void ClipTriX(int32_t x, bool KeepRight, std::array<Vertex, 6>& Vertices,
                int32_t& VertexCount);
  void ClipTriY(int32_t y, bool KeepBottom, std::array<Vertex, 6>& Vertices,
                int32_t& VertexCount);
  void RenderDisplayList();

  bool RequestAtlasImageUse(AtlasHandle h, math::Rect* r);

  template <class UV, class RES>
  float UVTRANSLATION(UV uv, RES res) {
    return (uv + uv_offset_) / static_cast<float>(res);
  }

  math::Point offset_;
  // screen space window rect for the currently drawn item
  math::Rect crop_rect_;
  DrawMode draw_mode_;
  uint8_t opacity_;
  Application* app_;

  std::vector<Vertex> display_list_;
  Atlas* atlas_;
  renderer::Device* device_;

  float uv_offset_ = 0;  // texel offset to fix 0.5 texel shift in Directx9

  math::Rect parent_crop_rect_;

  std::unique_ptr<renderer::IndexBuffer> rect_index_buffer_;
  std::unique_ptr<renderer::VertexBuffer> vertex_buffer_;
  std::unique_ptr<renderer::VertexFormat> vertex_format_;

  std::unique_ptr<renderer::VertexShader> vx_shader_;
  std::unique_ptr<renderer::PixelShader> px_shader_;

  std::unique_ptr<renderer::SamplerState> gui_sampler_;
  std::unique_ptr<renderer::BlendState> gui_blend_state_;
  std::unique_ptr<renderer::RasterizerState> gui_raster_state_;
  std::unique_ptr<renderer::DepthStencilState> gui_zstate_;

  std::unique_ptr<renderer::ConstantBuffer> resolution_data_;
};

// helper functions for common use cases
void ZoomToMouseCenter(math::Point& Offset, int32_t& Zoom, int32_t NewZoom,
                       math::Point ZoomCenter);
void ZoomToMouseCenter(math::Point& Offset, float& Zoom, float NewZoom,
                       math::Point Pos);

}  // namespace gui
