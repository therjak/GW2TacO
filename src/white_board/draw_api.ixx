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

class CWBApplication;
class CWBItem;

struct WBDISPLAYLINE {
  math::Point p1, p2;
  float u1 = 0, v1 = 0, u2 = 0, v2 = 0;
  CColor c1, c2;
};

struct WBGUIVERTEX {
  math::Vector4 Pos;
  math::Vector2 UV;
  CColor Color;

  WBGUIVERTEX() = default;

  WBGUIVERTEX(const float x, const float y, const float u, const float v,
              const CColor& color)
      : Pos(x, y, 0, 1), UV(u, v), Color(color) {}

  WBGUIVERTEX(const math::Vector2& pos, const math::Vector2& uv,
              const CColor& color)
      : Pos(pos.x, pos.y, 0, 1), UV(uv.x, uv.y), Color(color) {}
};

enum class WBDRAWMODE : uint8_t { WBD_RECTANGLES, WBD_LINES, WBD_TRIANGLES };

class CWBDrawAPI {
  friend class CWBApplication;
  // these are the only things that need to access the
  // renderdisplaylist function

  friend class CWBItem;
  // screen space top left corner of the client rect for
  // the currently drawn item

 public:
  CWBDrawAPI();
  virtual ~CWBDrawAPI();

  bool Initialize(CWBApplication* App, renderer::Device* Device, CAtlas* Atlas);

  void DrawRect(const math::Rect& r, CColor Color);
  void DrawRect(const math::Rect& r, float u1, float v1, float u2, float v2);
  void DrawRect(const math::Rect& r, float u1, float v1, float u2, float v2,
                CColor Color);
  void DrawRectBorder(const math::Rect& r, CColor Color);
  void DrawRectRotated(const math::Rect& r, float u1, float v1, float u2,
                       float v2, CColor Color, float rotation);

  void DrawLine(const math::Point& p1, const math::Point& p2, CColor Color);
  void DrawLine(const math::Point& p1, const math::Point& p2, CColor Color1,
                CColor Color2);

  void DrawTriangle(const math::Point& p1, const math::Point& p2,
                    const math::Point& p3, CColor Color);
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
  void SetParentCropRect(const math::Rect& r) { ParentCropRect = r; };
  math::Rect& GetCropRect() { return CropRect; }
  math::Rect& GetParentCropRect() { return ParentCropRect; }
  math::Point& GetOffset() { return Offset; }

  void SetUIRenderState();
  void SetUIBlendState(std::unique_ptr<renderer::BlendState>&& BlendState);
  void SetUISamplerState(
      std::unique_ptr<renderer::SamplerState>&& SamplerState);

  math::Size GetAtlasElementSize(WBATLASHANDLE h);
  void DrawAtlasElement(WBATLASHANDLE h, int32_t x, int32_t y,
                        CColor Color = CColor(0xffffffff));
  void DrawAtlasElement(WBATLASHANDLE h, const math::Rect& Position, bool TileX,
                        bool TileY, bool StretchX, bool StretchY,
                        CColor Color = CColor(0xffffffff));
  void DrawAtlasElementRotated(WBATLASHANDLE h, const math::Rect& Position,
                               CColor Color, float rotation);
  void SetCropToClient(const CWBItem* i);

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
  void ClipTriX(int32_t x, bool KeepRight, std::array<WBGUIVERTEX, 6>& Vertices,
                int32_t& VertexCount);
  void ClipTriY(int32_t y, bool KeepBottom,
                std::array<WBGUIVERTEX, 6>& Vertices, int32_t& VertexCount);
  void RenderDisplayList();

  bool RequestAtlasImageUse(WBATLASHANDLE h, math::Rect* r);

  template <class UV, class RES>
  float UVTRANSLATION(UV uv, RES res) {
    return (uv + UVOffset) / static_cast<float>(res);
  }

  math::Point Offset;
  // screen space window rect for the currently drawn item
  math::Rect CropRect;
  WBDRAWMODE DrawMode;
  uint8_t Opacity;
  CWBApplication* App;

  std::vector<WBGUIVERTEX> DisplayList;
  CAtlas* Atlas;
  renderer::Device* Device;

  float UVOffset = 0;  // texel offset to fix 0.5 texel shift in Directx9

  math::Rect ParentCropRect;

  std::unique_ptr<renderer::IndexBuffer> rectIndexBuffer;
  std::unique_ptr<renderer::VertexBuffer> VertexBuffer;
  std::unique_ptr<renderer::VertexFormat> VertexFormat;

  std::unique_ptr<renderer::VertexShader> VxShader;
  std::unique_ptr<renderer::PixelShader> PxShader;

  std::unique_ptr<renderer::SamplerState> GuiSampler;
  std::unique_ptr<renderer::BlendState> GuiBlendState;
  std::unique_ptr<renderer::RasterizerState> GuiRasterState;
  std::unique_ptr<renderer::DepthStencilState> GuiZState;

  std::unique_ptr<renderer::ConstantBuffer> ResolutionData;
};

// helper functions for common use cases
void ZoomToMouseCenter(math::Point& Offset, int32_t& Zoom, int32_t NewZoom,
                       math::Point ZoomCenter);
void ZoomToMouseCenter(math::Point& Offset, float& Zoom, float NewZoom,
                       math::Point Pos);

}  // namespace gui
