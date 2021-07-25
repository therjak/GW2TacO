#include "Skin.h"

#include <string>

#include "../BaseLib/string_format.h"

//////////////////////////////////////////////////////////////////////////
// metrics

CWBMetricValue::CWBMetricValue() = default;

void CWBMetricValue::SetMetric(WBMETRICTYPE w, float Value) {
  Metrics[w] = Value;
  MetricsUsed[w] = true;
}

void CWBMetricValue::SetValue(float Relative, float Pixels) {
  Metrics[WB_RELATIVE] = Relative;
  Metrics[WB_PIXELS] = Pixels;
  MetricsUsed[WB_RELATIVE] = true;
  MetricsUsed[WB_PIXELS] = true;
}

float CWBMetricValue::GetValue(float ParentSize, int32_t ContentSize) {
  if (AutoSize) return ContentSize + 0.5f;
  float v = 0;
  if (MetricsUsed[WB_PIXELS]) v += Metrics[WB_PIXELS];
  if (MetricsUsed[WB_RELATIVE]) v += Metrics[WB_RELATIVE] * ParentSize;
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

  const bool WidthSet = Positions.find(WB_WIDTH) != Positions.end();
  const bool HeightSet = Positions.find(WB_HEIGHT) != Positions.end();
  const bool TopSet = Positions.find(WB_MARGIN_TOP) != Positions.end();
  const bool LeftSet = Positions.find(WB_MARGIN_LEFT) != Positions.end();
  const bool RightSet = Positions.find(WB_MARGIN_RIGHT) != Positions.end();
  const bool BottomSet = Positions.find(WB_MARGIN_BOTTOM) != Positions.end();

  if (WidthSet) {
    Width = static_cast<int32_t>(Positions[WB_WIDTH].GetValue(
        static_cast<float>(ParentSize.x), ContentSize.x));
  }
  if (HeightSet) {
    Height = static_cast<int32_t>(Positions[WB_HEIGHT].GetValue(
        static_cast<float>(ParentSize.y), ContentSize.y));
  }
  if (TopSet) {
    Top = static_cast<int32_t>(
        Positions[WB_MARGIN_TOP].GetValue(static_cast<float>(ParentSize.y), 0));
  }
  if (LeftSet) {
    Left = static_cast<int32_t>(Positions[WB_MARGIN_LEFT].GetValue(
        static_cast<float>(ParentSize.x), 0));
  }
  if (RightSet) {
    Right = static_cast<int32_t>(Positions[WB_MARGIN_RIGHT].GetValue(
        static_cast<float>(ParentSize.x), 0));
  }
  if (BottomSet) {
    Bottom = static_cast<int32_t>(Positions[WB_MARGIN_BOTTOM].GetValue(
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

  r.x1 = static_cast<int32_t>(Positions[WB_PADDING_LEFT].GetValue(
             static_cast<float>(ParentSize.x), 0)) +
         BorderSizes.x1;
  r.y1 = static_cast<int32_t>(Positions[WB_PADDING_TOP].GetValue(
             static_cast<float>(ParentSize.y), 0)) +
         BorderSizes.y1;
  r.x2 = ParentSize.x -
         static_cast<int32_t>(Positions[WB_PADDING_RIGHT].GetValue(
             static_cast<float>(ParentSize.x), 0)) -
         BorderSizes.x2;
  r.y2 = ParentSize.y -
         static_cast<int32_t>(Positions[WB_PADDING_BOTTOM].GetValue(
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
  return Positions.find(WB_WIDTH) != Positions.end();
}

bool CWBPositionDescriptor::IsHeightSet() {
  return Positions.find(WB_HEIGHT) != Positions.end();
}

int32_t CWBPositionDescriptor::GetWidth(CSize ParentSize, CSize ContentSize) {
  const bool WidthSet = Positions.find(WB_WIDTH) != Positions.end();
  if (WidthSet)
    return static_cast<int32_t>(Positions[WB_WIDTH].GetValue(
        static_cast<float>(ParentSize.x), ContentSize.x));
  return 0;
}

int32_t CWBPositionDescriptor::GetHeight(CSize ParentSize, CSize ContentSize) {
  const bool HeightSet = Positions.find(WB_HEIGHT) != Positions.end();
  if (HeightSet)
    return static_cast<int32_t>(Positions[WB_HEIGHT].GetValue(
        static_cast<float>(ParentSize.y), ContentSize.y));
  return 0;
}

bool CWBPositionDescriptor::IsAutoResizer() {
  const bool WidthSet = Positions.find(WB_WIDTH) != Positions.end();
  if (WidthSet && Positions[WB_WIDTH].IsAutoResizer()) return true;

  const bool HeightSet = Positions.find(WB_HEIGHT) != Positions.end();
  if (HeightSet && Positions[WB_HEIGHT].IsAutoResizer()) return true;

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
  if (static_cast<int32_t>(p) < 0 || p > WB_HEIGHT) return;
  Positions[p] = Pixels;
  Set[p] = true;
}

FORCEINLINE CRect CWBPositionDescriptorPixels::GetPosition(CSize ParentSize) {
  CRect r(0, 0, 0, 0);

  r.x1 =
      Set[WB_MARGIN_LEFT]
          ? Positions[WB_MARGIN_LEFT]
          : (ParentSize.x - (Positions[WB_MARGIN_RIGHT] + Positions[WB_WIDTH]));
  r.y1 = Set[WB_MARGIN_TOP] ? Positions[WB_MARGIN_TOP]
                            : (ParentSize.y - (Positions[WB_MARGIN_BOTTOM] +
                                               Positions[WB_HEIGHT]));
  r.x2 = Set[WB_MARGIN_RIGHT]
             ? (ParentSize.x - Positions[WB_MARGIN_RIGHT])
             : (Positions[WB_MARGIN_LEFT] + Positions[WB_WIDTH]);
  r.y2 = Set[WB_MARGIN_BOTTOM]
             ? (ParentSize.y - Positions[WB_MARGIN_BOTTOM])
             : (Positions[WB_MARGIN_TOP] + Positions[WB_HEIGHT]);

  if (!Set[WB_MARGIN_LEFT] && !Set[WB_MARGIN_RIGHT]) {
    r.x1 = (ParentSize.x - Positions[WB_WIDTH]) / 2;
    r.x2 = r.x1 + Positions[WB_WIDTH];
  }

  if (!Set[WB_MARGIN_TOP] && !Set[WB_MARGIN_BOTTOM]) {
    r.y1 = (ParentSize.y - Positions[WB_HEIGHT]) / 2;
    r.y2 = r.y1 + Positions[WB_HEIGHT];
  }

  return r;
}

CWBMosaicImage::CWBMosaicImage() {
  Tiling[0] = Tiling[1] = false;
  Stretching[0] = Stretching[1] = false;
  Handle = 0;
  Color = 0xffffffff;
}

void CWBMosaicImage::SetPositionValue(WBPOSITIONTYPE p, int32_t Pixels) {
  Position.SetValue(p, Pixels);
}

void CWBMosaicImage::SetTiling(int32_t Axis, bool y) { Tiling[Axis] = y; }

void CWBMosaicImage::SetStretching(int32_t Axis, bool y) {
  Stretching[Axis] = y;
}

void CWBMosaicImage::SetHandle(WBATLASHANDLE handle) { Handle = handle; }

FORCEINLINE void CWBMosaicImage::Render(CWBDrawAPI* API, CRect& Pos) {
  CRect Croprect = API->GetCropRect();
  API->SetCropRect(Pos + API->GetOffset());

  CRect displaypos = Position.GetPosition(Pos.Size()) + Pos.TopLeft();

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
    image.Render(API, Position + CRect(Overshoot[0], Overshoot[1], Overshoot[2],
                                       Overshoot[3]));
}

void CWBMosaic::SetName(std::string_view name) { Name = name; }

std::string& CWBMosaic::GetName() { return Name; }

CWBMosaic::CWBMosaic(const CWBMosaic& Copy) {
  Name = Copy.Name;
  Images = Copy.Images;
  for (int32_t x = 0; x < 4; x++) Overshoot[x] = Copy.Overshoot[x];
}

CWBMosaic::CWBMosaic() : Overshoot{0} {}

CWBMosaic& CWBMosaic::operator=(const CWBMosaic& Copy) {
  if (&Copy == this) return *this;
  Name = Copy.Name;
  Images = Copy.Images;
  for (int32_t x = 0; x < 4; x++) Overshoot[x] = Copy.Overshoot[x];
  return *this;
}

void CWBMosaic::Flush() { Images.clear(); }

void CWBMosaic::SetOverShoot(WBRECTSIDE side, int32_t val) {
  Overshoot[side] = val;
}

void CWBSkinElement::SetName(std::string_view name) { Name = name; }

std::string& CWBSkinElement::GetName() { return Name; }

FORCEINLINE void CWBSkinElement::Render(CWBDrawAPI* API, CRect& Pos) {
  API->DrawAtlasElement(Handle, Pos, DefaultBehavior[0] == WB_SKINBEHAVIOR_TILE,
                        DefaultBehavior[1] == WB_SKINBEHAVIOR_TILE,
                        DefaultBehavior[0] == WB_SKINBEHAVIOR_STRETCH,
                        DefaultBehavior[1] == WB_SKINBEHAVIOR_STRETCH);
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
  DefaultBehavior[0] = WB_SKINBEHAVIOR_PIXELCORRECT;
  DefaultBehavior[1] = WB_SKINBEHAVIOR_PIXELCORRECT;
}

WBATLASHANDLE CWBSkinElement::GetHandle() { return Handle; }

CSize CWBSkinElement::GetElementSize(CWBDrawAPI* API) {
  return API->GetAtlasElementSize(Handle);
}

void CWBSkin::RenderElement(CWBDrawAPI* API, WBSKINELEMENTID ID, CRect& Pos) {
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
                            CRect& Pos) {
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

  Mosaics.back().SetOverShoot(WB_RECTSIDE_LEFT, OverShootLeft);
  Mosaics.back().SetOverShoot(WB_RECTSIDE_TOP, OverShootTop);
  Mosaics.back().SetOverShoot(WB_RECTSIDE_RIGHT, OverShootRight);
  Mosaics.back().SetOverShoot(WB_RECTSIDE_BOTTOM, OverShootBottom);

  auto Lines = Split(Description, _T( ")" ));
  for (const auto& l : Lines) {
    auto Data = Split(Trim(l), _T( "(" ));
    CWBMosaicImage i;

    if (Data.size() == 2) {
      CWBSkinElement* e = GetElement(Trim(Data[0]));
      if (e) {
        i.SetHandle(e->GetHandle());
        i.SetTiling(0, e->GetBehavior(0) == WB_SKINBEHAVIOR_TILE);
        i.SetTiling(1, e->GetBehavior(1) == WB_SKINBEHAVIOR_TILE);
        i.SetStretching(0, e->GetBehavior(0) == WB_SKINBEHAVIOR_STRETCH);
        i.SetStretching(1, e->GetBehavior(1) == WB_SKINBEHAVIOR_STRETCH);

        auto Data2 = Split(Data[1], _T( ";" ));

        for (const auto& d2 : Data2) {
          auto keyvalue = Split(d2, _T( ":" ));
          if (keyvalue.size() > 0) {
            const auto key = Trim(keyvalue[0]);
            std::string value;

            if (keyvalue.size() > 1) value = Trim(keyvalue[1]);

            if (key == _T( "top" )) {
              i.SetPositionValue(WB_MARGIN_TOP, std::stoi(value));
            }
            if (key == _T( "left" )) {
              i.SetPositionValue(WB_MARGIN_LEFT, std::stoi(value));
            }
            if (key == _T( "right" )) {
              i.SetPositionValue(WB_MARGIN_RIGHT, std::stoi(value));
            }
            if (key == _T( "bottom" )) {
              i.SetPositionValue(WB_MARGIN_BOTTOM, std::stoi(value));
            }
            if (key == _T( "width" )) {
              i.SetPositionValue(WB_WIDTH, std::stoi(value));
            }
            if (key == _T( "height" )) {
              i.SetPositionValue(WB_HEIGHT, std::stoi(value));
            }

            if (key == _T( "repeat-x" )) i.SetTiling(0, true);
            if (key == _T( "repeat-y" )) i.SetTiling(1, true);
            if (key == _T( "stretch-x" )) i.SetStretching(0, true);
            if (key == _T( "stretch-y" )) i.SetStretching(1, true);
            if (key == _T( "color" )) {
              auto v = std::stoi(value, nullptr, 16);
              if (value.size() <= 6) v = v | 0xff000000;
              i.SetColor(v);
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
