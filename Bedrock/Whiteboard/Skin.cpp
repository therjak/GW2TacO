#include "Skin.h"

#include <string>

#include "../BaseLib/string_format.h"

//////////////////////////////////////////////////////////////////////////
// metrics

CWBMetricValue::CWBMetricValue() = default;

void CWBMetricValue::SetMetric(WBMETRICTYPE w, float Value) {
  MetricsAt(w) = Value;
  MetricsUsedAt(w) = true;
}

void CWBMetricValue::SetValue(float Relative, float Pixels) {
  MetricsAt(WBMETRICTYPE::WB_RELATIVE) = Relative;
  MetricsAt(WBMETRICTYPE::WB_PIXELS) = Pixels;
  MetricsUsedAt(WBMETRICTYPE::WB_RELATIVE) = true;
  MetricsUsedAt(WBMETRICTYPE::WB_PIXELS) = true;
}

float CWBMetricValue::GetValue(float ParentSize, int32_t ContentSize) {
  if (AutoSize) return ContentSize + 0.5f;
  float v = 0;
  if (MetricsUsedAt(WBMETRICTYPE::WB_PIXELS))
    v += MetricsAt(WBMETRICTYPE::WB_PIXELS);
  if (MetricsUsedAt(WBMETRICTYPE::WB_RELATIVE))
    v += MetricsAt(WBMETRICTYPE::WB_RELATIVE) * ParentSize;
  return v;
}

bool CWBMetricValue::IsAutoResizer() { return AutoSize; }

void CWBMetricValue::SetAutoSize(bool Auto) { AutoSize = Auto; }

//////////////////////////////////////////////////////////////////////////
// position descriptor - general

CRect CWBPositionDescriptor::GetPosition(CSize ParentSize, CSize ContentSize,
                                         const CRect& Original) {
  CRect r(0, 0, 0, 0);

  int32_t Width = 0;
  int32_t Height = 0;
  int32_t Top = 0;
  int32_t Left = 0;
  int32_t Right = 0;
  int32_t Bottom = 0;

  const bool WidthSet =
      Positions.find(WBPOSITIONTYPE::WB_WIDTH) != Positions.end();
  const bool HeightSet =
      Positions.find(WBPOSITIONTYPE::WB_HEIGHT) != Positions.end();
  const bool TopSet =
      Positions.find(WBPOSITIONTYPE::WB_MARGIN_TOP) != Positions.end();
  const bool LeftSet =
      Positions.find(WBPOSITIONTYPE::WB_MARGIN_LEFT) != Positions.end();
  const bool RightSet =
      Positions.find(WBPOSITIONTYPE::WB_MARGIN_RIGHT) != Positions.end();
  const bool BottomSet =
      Positions.find(WBPOSITIONTYPE::WB_MARGIN_BOTTOM) != Positions.end();

  if (WidthSet) {
    Width = static_cast<int32_t>(Positions[WBPOSITIONTYPE::WB_WIDTH].GetValue(
        static_cast<float>(ParentSize.x), ContentSize.x));
  }
  if (HeightSet) {
    Height = static_cast<int32_t>(Positions[WBPOSITIONTYPE::WB_HEIGHT].GetValue(
        static_cast<float>(ParentSize.y), ContentSize.y));
  }
  if (TopSet) {
    Top =
        static_cast<int32_t>(Positions[WBPOSITIONTYPE::WB_MARGIN_TOP].GetValue(
            static_cast<float>(ParentSize.y), 0));
  }
  if (LeftSet) {
    Left =
        static_cast<int32_t>(Positions[WBPOSITIONTYPE::WB_MARGIN_LEFT].GetValue(
            static_cast<float>(ParentSize.x), 0));
  }
  if (RightSet) {
    Right = static_cast<int32_t>(
        Positions[WBPOSITIONTYPE::WB_MARGIN_RIGHT].GetValue(
            static_cast<float>(ParentSize.x), 0));
  }
  if (BottomSet) {
    Bottom = static_cast<int32_t>(
        Positions[WBPOSITIONTYPE::WB_MARGIN_BOTTOM].GetValue(
            static_cast<float>(ParentSize.y), 0));
  }

  r.x1 = LeftSet ? Left : (ParentSize.x - (Right + Width));
  r.y1 = TopSet ? Top : (ParentSize.y - (Bottom + Height));
  r.x2 = RightSet ? (ParentSize.x - Right) : (Left + Width);
  r.y2 = BottomSet ? (ParentSize.y - Bottom) : (Top + Height);

  if (!LeftSet && !RightSet) {
    r.x1 = (ParentSize.x - Width) / 2;
    r.x2 = r.x1 + Width;
  }

  if (!TopSet && !BottomSet) {
    r.y1 = (ParentSize.y - Height) / 2;
    r.y2 = r.y1 + Height;
  }

  if (!((LeftSet && RightSet) || WidthSet)) {  // x position not valid
    r.x1 = Original.x1;
    r.x2 = Original.x2;
  }

  if (!((TopSet && BottomSet) || HeightSet)) {  // y position not valid
    r.y1 = Original.y1;
    r.y2 = Original.y2;
  }

  return r;
}

CRect CWBPositionDescriptor::GetPadding(CSize ParentSize,
                                        const CRect& BorderSizes) {
  CRect r(0, 0, 0, 0);

  r.x1 =
      static_cast<int32_t>(Positions[WBPOSITIONTYPE::WB_PADDING_LEFT].GetValue(
          static_cast<float>(ParentSize.x), 0)) +
      BorderSizes.x1;
  r.y1 =
      static_cast<int32_t>(Positions[WBPOSITIONTYPE::WB_PADDING_TOP].GetValue(
          static_cast<float>(ParentSize.y), 0)) +
      BorderSizes.y1;
  r.x2 =
      ParentSize.x -
      static_cast<int32_t>(Positions[WBPOSITIONTYPE::WB_PADDING_RIGHT].GetValue(
          static_cast<float>(ParentSize.x), 0)) -
      BorderSizes.x2;
  r.y2 = ParentSize.y -
         static_cast<int32_t>(
             Positions[WBPOSITIONTYPE::WB_PADDING_BOTTOM].GetValue(
                 static_cast<float>(ParentSize.y), 0)) -
         BorderSizes.y2;

  return r;
}

void CWBPositionDescriptor::SetMetric(WBPOSITIONTYPE p, WBMETRICTYPE m,
                                      float Value) {
  Positions[p].SetMetric(m, Value);
}

void CWBPositionDescriptor::SetValue(WBPOSITIONTYPE p, float Relative,
                                     float Pixels) {
  Positions[p].SetValue(Relative, Pixels);
}

void CWBPositionDescriptor::ClearMetrics(WBPOSITIONTYPE p) {
  Positions.erase(p);
}

bool CWBPositionDescriptor::IsWidthSet() {
  return Positions.find(WBPOSITIONTYPE::WB_WIDTH) != Positions.end();
}

bool CWBPositionDescriptor::IsHeightSet() {
  return Positions.find(WBPOSITIONTYPE::WB_HEIGHT) != Positions.end();
}

int32_t CWBPositionDescriptor::GetWidth(CSize ParentSize, CSize ContentSize) {
  const bool WidthSet =
      Positions.find(WBPOSITIONTYPE::WB_WIDTH) != Positions.end();
  if (WidthSet)
    return static_cast<int32_t>(Positions[WBPOSITIONTYPE::WB_WIDTH].GetValue(
        static_cast<float>(ParentSize.x), ContentSize.x));
  return 0;
}

int32_t CWBPositionDescriptor::GetHeight(CSize ParentSize, CSize ContentSize) {
  const bool HeightSet =
      Positions.find(WBPOSITIONTYPE::WB_HEIGHT) != Positions.end();
  if (HeightSet)
    return static_cast<int32_t>(Positions[WBPOSITIONTYPE::WB_HEIGHT].GetValue(
        static_cast<float>(ParentSize.y), ContentSize.y));
  return 0;
}

bool CWBPositionDescriptor::IsAutoResizer() {
  const bool WidthSet =
      Positions.find(WBPOSITIONTYPE::WB_WIDTH) != Positions.end();
  if (WidthSet && Positions[WBPOSITIONTYPE::WB_WIDTH].IsAutoResizer())
    return true;

  const bool HeightSet =
      Positions.find(WBPOSITIONTYPE::WB_HEIGHT) != Positions.end();
  if (HeightSet && Positions[WBPOSITIONTYPE::WB_HEIGHT].IsAutoResizer())
    return true;

  return false;
}

void CWBPositionDescriptor::SetAutoSize(WBPOSITIONTYPE p) {
  Positions[p].SetAutoSize(true);
}

//////////////////////////////////////////////////////////////////////////
// position descriptor - pixels

CWBPositionDescriptorPixels::CWBPositionDescriptorPixels() {
  for (int32_t x = 0; x < 6; x++) {
    Set[x] = false;
    Positions[x] = 0;
  }
}

void CWBPositionDescriptorPixels::SetValue(WBPOSITIONTYPE p, int32_t Pixels) {
  if (static_cast<int32_t>(p) < 0 || p > WBPOSITIONTYPE::WB_HEIGHT) return;
  PositionsAt(p) = Pixels;
  SetAt(p) = true;
}

FORCEINLINE CRect CWBPositionDescriptorPixels::GetPosition(CSize ParentSize) {
  CRect r(0, 0, 0, 0);

  r.x1 = SetAt(WBPOSITIONTYPE::WB_MARGIN_LEFT)
             ? PositionsAt(WBPOSITIONTYPE::WB_MARGIN_LEFT)
             : (ParentSize.x - (PositionsAt(WBPOSITIONTYPE::WB_MARGIN_RIGHT) +
                                PositionsAt(WBPOSITIONTYPE::WB_WIDTH)));
  r.y1 = SetAt(WBPOSITIONTYPE::WB_MARGIN_TOP)
             ? PositionsAt(WBPOSITIONTYPE::WB_MARGIN_TOP)
             : (ParentSize.y - (PositionsAt(WBPOSITIONTYPE::WB_MARGIN_BOTTOM) +
                                PositionsAt(WBPOSITIONTYPE::WB_HEIGHT)));
  r.x2 = SetAt(WBPOSITIONTYPE::WB_MARGIN_RIGHT)
             ? (ParentSize.x - PositionsAt(WBPOSITIONTYPE::WB_MARGIN_RIGHT))
             : (PositionsAt(WBPOSITIONTYPE::WB_MARGIN_LEFT) +
                PositionsAt(WBPOSITIONTYPE::WB_WIDTH));
  r.y2 = SetAt(WBPOSITIONTYPE::WB_MARGIN_BOTTOM)
             ? (ParentSize.y - PositionsAt(WBPOSITIONTYPE::WB_MARGIN_BOTTOM))
             : (PositionsAt(WBPOSITIONTYPE::WB_MARGIN_TOP) +
                PositionsAt(WBPOSITIONTYPE::WB_HEIGHT));

  if (!SetAt(WBPOSITIONTYPE::WB_MARGIN_LEFT) &&
      !SetAt(WBPOSITIONTYPE::WB_MARGIN_RIGHT)) {
    r.x1 = (ParentSize.x - PositionsAt(WBPOSITIONTYPE::WB_WIDTH)) / 2;
    r.x2 = r.x1 + PositionsAt(WBPOSITIONTYPE::WB_WIDTH);
  }

  if (!SetAt(WBPOSITIONTYPE::WB_MARGIN_TOP) &&
      !SetAt(WBPOSITIONTYPE::WB_MARGIN_BOTTOM)) {
    r.y1 = (ParentSize.y - PositionsAt(WBPOSITIONTYPE::WB_HEIGHT)) / 2;
    r.y2 = r.y1 + PositionsAt(WBPOSITIONTYPE::WB_HEIGHT);
  }

  return r;
}

CWBMosaicImage::CWBMosaicImage() = default;

void CWBMosaicImage::SetPositionValue(WBPOSITIONTYPE p, int32_t Pixels) {
  Position.SetValue(p, Pixels);
}

void CWBMosaicImage::SetTiling(int32_t Axis, bool y) { Tiling[Axis] = y; }

void CWBMosaicImage::SetStretching(int32_t Axis, bool y) {
  Stretching[Axis] = y;
}

void CWBMosaicImage::SetHandle(WBATLASHANDLE handle) { Handle = handle; }

FORCEINLINE void CWBMosaicImage::Render(CWBDrawAPI* API, const CRect& Pos) {
  const CRect Croprect = API->GetCropRect();
  API->SetCropRect(Pos + API->GetOffset());

  const CRect displaypos = Position.GetPosition(Pos.Size()) + Pos.TopLeft();

  API->DrawAtlasElement(Handle, displaypos, Tiling[0], Tiling[1], Stretching[0],
                        Stretching[1], Color);

  API->SetCropRect(Croprect);
}

void CWBMosaicImage::SetColor(CColor color) { Color = color; }

void CWBMosaic::AddImage(const CWBMosaicImage& Image) {
  Images.push_back(Image);
}

void CWBMosaic::Render(CWBDrawAPI* API, const CRect& Position) {
  for (auto& image : Images)
    image.Render(API,
                 Position + CRect(OvershootAt(WBRECTSIDE::WB_RECTSIDE_LEFT),
                                  OvershootAt(WBRECTSIDE::WB_RECTSIDE_TOP),
                                  OvershootAt(WBRECTSIDE::WB_RECTSIDE_RIGHT),
                                  OvershootAt(WBRECTSIDE::WB_RECTSIDE_BOTTOM)));
}

void CWBMosaic::SetName(std::string_view name) { Name = name; }

std::string& CWBMosaic::GetName() { return Name; }

CWBMosaic::CWBMosaic(const CWBMosaic& Copy) {
  Name = Copy.Name;
  Images = Copy.Images;
  Overshoot = Copy.Overshoot;
}

CWBMosaic::CWBMosaic() = default;

CWBMosaic& CWBMosaic::operator=(const CWBMosaic& Copy) {
  if (&Copy == this) return *this;
  Name = Copy.Name;
  Images = Copy.Images;
  Overshoot = Copy.Overshoot;
  return *this;
}

void CWBMosaic::Flush() { Images.clear(); }

void CWBMosaic::SetOverShoot(WBRECTSIDE side, int32_t val) {
  OvershootAt(side) = val;
}

void CWBSkinElement::SetName(std::string_view name) { Name = name; }

std::string& CWBSkinElement::GetName() { return Name; }

FORCEINLINE void CWBSkinElement::Render(CWBDrawAPI* API, const CRect& Pos) {
  API->DrawAtlasElement(
      Handle, Pos,
      DefaultBehavior[0] == WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_TILE,
      DefaultBehavior[1] == WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_TILE,
      DefaultBehavior[0] == WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_STRETCH,
      DefaultBehavior[1] == WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_STRETCH);
}

void CWBSkinElement::SetHandle(WBATLASHANDLE h) { Handle = h; }

void CWBSkinElement::SetBehavior(int32_t Axis, WBSKINELEMENTBEHAVIOR Behavior) {
  DefaultBehavior[Axis] = Behavior;
}

WBSKINELEMENTBEHAVIOR CWBSkinElement::GetBehavior(int32_t Axis) {
  return DefaultBehavior[Axis];
}

CWBSkinElement& CWBSkinElement::operator=(const CWBSkinElement& Copy) {
  if (&Copy == this) return *this;
  Name = Copy.Name;
  Handle = Copy.Handle;
  DefaultBehavior[0] = Copy.DefaultBehavior[0];
  DefaultBehavior[1] = Copy.DefaultBehavior[1];
  return *this;
}

CWBSkinElement::CWBSkinElement(const CWBSkinElement& Copy) {
  Name = Copy.Name;
  Handle = Copy.Handle;
  DefaultBehavior[0] = Copy.DefaultBehavior[0];
  DefaultBehavior[1] = Copy.DefaultBehavior[1];
}

CWBSkinElement::CWBSkinElement() {
  Handle = 0;
  DefaultBehavior[0] = WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_PIXELCORRECT;
  DefaultBehavior[1] = WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_PIXELCORRECT;
}

WBATLASHANDLE CWBSkinElement::GetHandle() { return Handle; }

CSize CWBSkinElement::GetElementSize(CWBDrawAPI* API) {
  return API->GetAtlasElementSize(Handle);
}

void CWBSkin::RenderElement(CWBDrawAPI* API, WBSKINELEMENTID ID,
                            const CRect& Pos) {
  if (ID == 0xffffffff) return;

  const uint32_t idx = ID & 0x7fffffff;

  if (!(ID & 0x80000000)) {
    if (idx < Mosaics.size()) {
      Mosaics[idx].Render(API, Pos);
    }
    return;
  }

  if (idx < SkinItems.size()) {
    SkinItems[idx].Render(API, Pos);
  }
}

void CWBSkin::RenderElement(CWBDrawAPI* API, std::string_view Name,
                            const CRect& Pos) {
  RenderElement(API, GetElementID(Name), Pos);
}

WBSKINELEMENTID CWBSkin::GetElementID(std::string_view Name) {
  for (uint32_t x = 0; x < Mosaics.size(); x++)
    if (Mosaics[x].GetName() == Name) return x;

  for (uint32_t x = 0; x < SkinItems.size(); x++)
    if (SkinItems[x].GetName() == Name) return x | 0x80000000;

  return 0xffffffff;
}

void CWBSkin::AddElement(std::string_view Name, WBATLASHANDLE Handle,
                         WBSKINELEMENTBEHAVIOR Xbehav,
                         WBSKINELEMENTBEHAVIOR Ybehav) {
  for (auto& skin : SkinItems)
    if (skin.GetName() == Name) {
      skin.SetHandle(Handle);
      skin.SetBehavior(0, Xbehav);
      skin.SetBehavior(1, Ybehav);
      return;
    }

  SkinItems.emplace_back(CWBSkinElement());
  SkinItems.back().SetName(Name);
  SkinItems.back().SetHandle(Handle);
  SkinItems.back().SetBehavior(0, Xbehav);
  SkinItems.back().SetBehavior(1, Ybehav);
}

CWBMosaic* CWBSkin::AddMosaic(std::string_view Name,
                              std::string_view Description,
                              int32_t OverShootLeft, int32_t OverShootTop,
                              int32_t OverShootRight, int32_t OverShootBottom) {
  for (auto& mosaic : Mosaics) {
    if (mosaic.GetName() == Name) {
      return &mosaic;
    }
  }

  Mosaics.emplace_back(CWBMosaic());
  Mosaics.back().SetName(Name);

  Mosaics.back().SetOverShoot(WBRECTSIDE::WB_RECTSIDE_LEFT, OverShootLeft);
  Mosaics.back().SetOverShoot(WBRECTSIDE::WB_RECTSIDE_TOP, OverShootTop);
  Mosaics.back().SetOverShoot(WBRECTSIDE::WB_RECTSIDE_RIGHT, OverShootRight);
  Mosaics.back().SetOverShoot(WBRECTSIDE::WB_RECTSIDE_BOTTOM, OverShootBottom);

  auto Lines = Split(Description, _T( ")" ));
  for (const auto& l : Lines) {
    auto Data = Split(Trim(l), _T( "(" ));
    CWBMosaicImage i;

    if (Data.size() == 2) {
      CWBSkinElement* e = GetElement(Trim(Data[0]));
      if (e) {
        i.SetHandle(e->GetHandle());
        i.SetTiling(0, e->GetBehavior(0) ==
                           WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_TILE);
        i.SetTiling(1, e->GetBehavior(1) ==
                           WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_TILE);
        i.SetStretching(0, e->GetBehavior(0) ==
                               WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_STRETCH);
        i.SetStretching(1, e->GetBehavior(1) ==
                               WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_STRETCH);

        auto Data2 = Split(Data[1], _T( ";" ));

        for (const auto& d2 : Data2) {
          auto keyvalue = Split(d2, _T( ":" ));
          if (keyvalue.size() > 0) {
            const auto key = Trim(keyvalue[0]);
            std::string value;

            if (keyvalue.size() > 1) value = Trim(keyvalue[1]);

            if (key == _T( "top" )) {
              i.SetPositionValue(WBPOSITIONTYPE::WB_MARGIN_TOP,
                                 std::stoi(value));
            }
            if (key == _T( "left" )) {
              i.SetPositionValue(WBPOSITIONTYPE::WB_MARGIN_LEFT,
                                 std::stoi(value));
            }
            if (key == _T( "right" )) {
              i.SetPositionValue(WBPOSITIONTYPE::WB_MARGIN_RIGHT,
                                 std::stoi(value));
            }
            if (key == _T( "bottom" )) {
              i.SetPositionValue(WBPOSITIONTYPE::WB_MARGIN_BOTTOM,
                                 std::stoi(value));
            }
            if (key == _T( "width" )) {
              i.SetPositionValue(WBPOSITIONTYPE::WB_WIDTH, std::stoi(value));
            }
            if (key == _T( "height" )) {
              i.SetPositionValue(WBPOSITIONTYPE::WB_HEIGHT, std::stoi(value));
            }

            if (key == _T( "repeat-x" )) i.SetTiling(0, true);
            if (key == _T( "repeat-y" )) i.SetTiling(1, true);
            if (key == _T( "stretch-x" )) i.SetStretching(0, true);
            if (key == _T( "stretch-y" )) i.SetStretching(1, true);
            if (key == _T( "color" )) {
              uint32_t v = std::stoi(value, nullptr, 16);
              if (value.size() <= 6) v = v | 0xff000000;
              i.SetColor(CColor(v));
            }
          }
        }
        Mosaics.back().AddImage(i);
      }
    }
  }

  return &Mosaics.back();
}

CWBSkinElement* CWBSkin::GetElement(std::string_view Name) {
  for (auto& skin : SkinItems) {
    if (skin.GetName() == Name) {
      return &skin;
    }
  }
  return nullptr;
}

CWBSkinElement* CWBSkin::GetElement(WBSKINELEMENTID id) {
  if (id == 0xffffffff) return nullptr;
  if (!(id & 0x80000000)) return nullptr;  // mosaic
  const uint32_t idx = id & 0x7fffffff;
  if (idx >= SkinItems.size()) return nullptr;
  return &SkinItems[idx];
}

CSize CWBSkin::GetElementSize(CWBDrawAPI* API, WBSKINELEMENTID ID) {
  if (ID == 0xffffffff) return CSize(0, 0);

  const uint32_t idx = ID & 0x7fffffff;

  if (!(ID & 0x80000000)) return CSize(0, 0);  // mosaics don't have sizes

  if (idx < SkinItems.size()) return SkinItems[idx].GetElementSize(API);

  return CSize(0, 0);
}
