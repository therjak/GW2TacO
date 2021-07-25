#pragma once

#include <vector>

#include "../BaseLib/Color.h"
#include "../CoRE2/Core2.h"
#include "Atlas.h"
#include "Font.h"

struct WBDISPLAYLINE {
  CPoint p1, p2;
  float u1 = 0, v1 = 0, u2 = 0, v2 = 0;
  CColor c1, c2;
};

struct WBGUIVERTEX {
  CVector4 Pos;
  CVector2 UV;
  CColor Color;

  WBGUIVERTEX() = default;

  WBGUIVERTEX(const float x, const float y, const float u, const float v,
              const CColor& color)
      : Pos(x, y, 0, 1), UV(u, v), Color(color) {}

  WBGUIVERTEX(const CVector2& pos, const CVector2& uv, const CColor& color)
      : Pos(pos.x, pos.y, 0, 1), UV(uv.x, uv.y), Color(color) {}
};

enum WBDRAWMODE { WBD_RECTANGLES, WBD_LINES, WBD_TRIANGLES };

class CWBDrawAPI {
  friend class CWBApplication;
  friend class CWBItem;  // these are the only things that need to access the
                         // renderdisplaylist function

  CPoint Offset;   // screen space top left corner of the client rect for the
                   // currently drawn item
  CRect CropRect;  // screen space window rect for the currently drawn item
  WBDRAWMODE DrawMode;
  uint8_t Opacity;
  CWBApplication* App;

  std::vector<WBGUIVERTEX> DisplayList;
  CAtlas* Atlas;
  CCoreDevice* Device;

  float UVOffset = 0;  // texel offset to fix 0.5 texel shift in Directx9

  CRect ParentCropRect;

  std::unique_ptr<CCoreIndexBuffer> rectIndexBuffer;
  std::unique_ptr<CCoreVertexBuffer> VertexBuffer;
  std::unique_ptr<CCoreVertexFormat> VertexFormat;

  std::unique_ptr<CCoreVertexShader> VxShader;
  std::unique_ptr<CCorePixelShader> PxShader;

  std::unique_ptr<CCoreSamplerState> GuiSampler;
  std::unique_ptr<CCoreBlendState> GuiBlendState;
  std::unique_ptr<CCoreRasterizerState> GuiRasterState;
  std::unique_ptr<CCoreDepthStencilState> GuiZState;

  std::unique_ptr<CCoreConstantBuffer> ResolutionData;

  void AddDisplayRect(const CRect& r, const float u1, const float v1,
                      const float u2, const float v3, const CColor a);
  void AddDisplayRectRotated(const CRect& r, const float u1, const float v1,
                             const float u2, const float v3, const CColor a,
                             float rotation);
  void AddDisplayLine(const CPoint& p1, const CPoint& p2, const float u1,
                      const float v1, const float u2, const float v2,
                      const CColor a, const CColor b);
  void AddDisplayTri(const CPoint& p1, const CPoint& p2, const CPoint& p3,
                     const float u1, const float v1, const float u2,
                     const float v2, const float u3, const float v3,
                     const CColor a, const CColor b, const CColor c);
  void ClipTriX(int32_t x, bool KeepRight, WBGUIVERTEX Vertices[6],
                int32_t& VertexCount);
  void ClipTriY(int32_t y, bool KeepBottom, WBGUIVERTEX Vertices[6],
                int32_t& VertexCount);
  void RenderDisplayList();

  bool RequestAtlasImageUse(WBATLASHANDLE h, CRect& r);

 public:
  CWBDrawAPI();
  virtual ~CWBDrawAPI();

  bool Initialize(CWBApplication* App, CCoreDevice* Device, CAtlas* Atlas);

  void DrawRect(const CRect& r, CColor Color);
  void DrawRect(const CRect& r, float u1, float v1, float u2, float v2);
  void DrawRect(const CRect& r, float u1, float v1, float u2, float v2,
                CColor Color);
  void DrawRectBorder(const CRect& r, CColor Color);
  void DrawRectRotated(const CRect& r, float u1, float v1, float u2, float v2,
                       CColor Color, float rotation);

  void DrawLine(const CPoint& p1, const CPoint& p2, CColor Color);
  void DrawLine(const CPoint& p1, const CPoint& p2, CColor Color1,
                CColor Color2);

  void DrawTriangle(const CPoint& p1, const CPoint& p2, const CPoint& p3,
                    CColor Color);
  void DrawTriangle(const CPoint& p1, const CPoint& p2, const CPoint& p3,
                    CColor a, CColor b, CColor c);
  void DrawTriangle(const CPoint& p1, const CPoint& p2, const CPoint& p3,
                    float u1, float v1, float u2, float v2, float u3, float v3,
                    CColor a, CColor b, CColor c);
  void DrawTriangle(const CPoint& p1, const CPoint& p2, const CPoint& p3,
                    float u1, float v1, float u2, float v2, float u3, float v3);

  void SetOffset(const CPoint& p);
  void SetCropRect(const CRect& r);
  void SetParentCropRect(CRect& r) { ParentCropRect = r; };
  INLINE CRect& GetCropRect() { return CropRect; }
  INLINE CRect& GetParentCropRect() { return ParentCropRect; }
  INLINE CPoint& GetOffset() { return Offset; }

  void SetUIRenderState();
  void SetUIBlendState(std::unique_ptr<CCoreBlendState>&& BlendState);
  void SetUISamplerState(std::unique_ptr<CCoreSamplerState>&& SamplerState);

  CSize GetAtlasElementSize(WBATLASHANDLE h);
  void DrawAtlasElement(WBATLASHANDLE h, int32_t x, int32_t y,
                        CColor Color = CColor(0xffffffff));
  void DrawAtlasElement(WBATLASHANDLE h, const CRect& Position, bool TileX,
                        bool TileY, bool StretchX, bool StretchY,
                        CColor Color = CColor(0xffffffff));
  void DrawAtlasElementRotated(WBATLASHANDLE h, const CRect& Position,
                               CColor Color, float rotation);
  void SetCropToClient(const CWBItem* i);

  void FlushDrawBuffer();
  CCoreDevice* GetDevice();
  CCoreConstantBuffer* GetResolutionData();

  void SetOpacity(uint8_t o);
  void SetRenderView(CRect r);

  void SetPixelShader(std::unique_ptr<CCorePixelShader>&& shader);
};

// helper functions for common use cases
void ZoomToMouseCenter(CPoint& Offset, int32_t& Zoom, int32_t NewZoom,
                       CPoint ZoomCenter);
void ZoomToMouseCenter(CPoint& Offset, float& Zoom, float NewZoom, CPoint Pos);
