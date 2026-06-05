module;
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "src/base/color.h"
#include "src/base/string_format.h"

module whiteboard;

import :draw_api;
import :skin;

using math::Rect;
using math::Size;

namespace gui {

//////////////////////////////////////////////////////////////////////////
// metrics

MetricValue::MetricValue() = default;

void MetricValue::SetMetric(WBMETRICTYPE w, float Value) {
  MetricsAt(w) = Value;
  MetricsUsedAt(w) = true;
}

void MetricValue::SetValue(float Relative, float Pixels) {
  MetricsAt(WBMETRICTYPE::WB_RELATIVE) = Relative;
  MetricsAt(WBMETRICTYPE::WB_PIXELS) = Pixels;
  MetricsUsedAt(WBMETRICTYPE::WB_RELATIVE) = true;
  MetricsUsedAt(WBMETRICTYPE::WB_PIXELS) = true;
}

float MetricValue::GetValue(float ParentSize, int32_t ContentSize) const {
  if (auto_size_) return ContentSize + 0.5f;
  float v = 0;
  if (MetricsUsedAt(WBMETRICTYPE::WB_PIXELS)) {
    v += MetricsAt(WBMETRICTYPE::WB_PIXELS);
  }
  if (MetricsUsedAt(WBMETRICTYPE::WB_RELATIVE)) {
    v += MetricsAt(WBMETRICTYPE::WB_RELATIVE) * ParentSize;
  }
  return v;
}

bool MetricValue::IsAutoResizer() { return auto_size_; }

void MetricValue::SetAutoSize(bool Auto) { auto_size_ = Auto; }

//////////////////////////////////////////////////////////////////////////
// position descriptor - general

Rect PositionDescriptor::GetPosition(Size ParentSize, Size ContentSize,
                                     const Rect& Original) {
  Rect r(0, 0, 0, 0);

  int32_t Width = 0;
  int32_t Height = 0;
  int32_t Top = 0;
  int32_t Left = 0;
  int32_t Right = 0;
  int32_t Bottom = 0;

  const bool WidthSet =
      positions_.find(WBPOSITIONTYPE::WB_WIDTH) != positions_.end();
  const bool HeightSet =
      positions_.find(WBPOSITIONTYPE::WB_HEIGHT) != positions_.end();
  const bool TopSet =
      positions_.find(WBPOSITIONTYPE::WB_MARGIN_TOP) != positions_.end();
  const bool LeftSet =
      positions_.find(WBPOSITIONTYPE::WB_MARGIN_LEFT) != positions_.end();
  const bool RightSet =
      positions_.find(WBPOSITIONTYPE::WB_MARGIN_RIGHT) != positions_.end();
  const bool BottomSet =
      positions_.find(WBPOSITIONTYPE::WB_MARGIN_BOTTOM) != positions_.end();

  if (WidthSet) {
    Width = static_cast<int32_t>(positions_[WBPOSITIONTYPE::WB_WIDTH].GetValue(
        static_cast<float>(ParentSize.x), ContentSize.x));
  }
  if (HeightSet) {
    Height =
        static_cast<int32_t>(positions_[WBPOSITIONTYPE::WB_HEIGHT].GetValue(
            static_cast<float>(ParentSize.y), ContentSize.y));
  }
  if (TopSet) {
    Top =
        static_cast<int32_t>(positions_[WBPOSITIONTYPE::WB_MARGIN_TOP].GetValue(
            static_cast<float>(ParentSize.y), 0));
  }
  if (LeftSet) {
    Left = static_cast<int32_t>(
        positions_[WBPOSITIONTYPE::WB_MARGIN_LEFT].GetValue(
            static_cast<float>(ParentSize.x), 0));
  }
  if (RightSet) {
    Right = static_cast<int32_t>(
        positions_[WBPOSITIONTYPE::WB_MARGIN_RIGHT].GetValue(
            static_cast<float>(ParentSize.x), 0));
  }
  if (BottomSet) {
    Bottom = static_cast<int32_t>(
        positions_[WBPOSITIONTYPE::WB_MARGIN_BOTTOM].GetValue(
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

Rect PositionDescriptor::GetPadding(Size ParentSize, const Rect& BorderSizes) {
  Rect r(0, 0, 0, 0);

  r.x1 =
      static_cast<int32_t>(positions_[WBPOSITIONTYPE::WB_PADDING_LEFT].GetValue(
          static_cast<float>(ParentSize.x), 0)) +
      BorderSizes.x1;
  r.y1 =
      static_cast<int32_t>(positions_[WBPOSITIONTYPE::WB_PADDING_TOP].GetValue(
          static_cast<float>(ParentSize.y), 0)) +
      BorderSizes.y1;
  r.x2 = ParentSize.x -
         static_cast<int32_t>(
             positions_[WBPOSITIONTYPE::WB_PADDING_RIGHT].GetValue(
                 static_cast<float>(ParentSize.x), 0)) -
         BorderSizes.x2;
  r.y2 = ParentSize.y -
         static_cast<int32_t>(
             positions_[WBPOSITIONTYPE::WB_PADDING_BOTTOM].GetValue(
                 static_cast<float>(ParentSize.y), 0)) -
         BorderSizes.y2;

  return r;
}

void PositionDescriptor::SetMetric(WBPOSITIONTYPE p, WBMETRICTYPE m,
                                   float Value) {
  positions_[p].SetMetric(m, Value);
}

void PositionDescriptor::SetValue(WBPOSITIONTYPE p, float Relative,
                                  float Pixels) {
  positions_[p].SetValue(Relative, Pixels);
}

void PositionDescriptor::ClearMetrics(WBPOSITIONTYPE p) { positions_.erase(p); }

bool PositionDescriptor::IsWidthSet() const {
  return positions_.find(WBPOSITIONTYPE::WB_WIDTH) != positions_.end();
}

bool PositionDescriptor::IsHeightSet() const {
  return positions_.find(WBPOSITIONTYPE::WB_HEIGHT) != positions_.end();
}

int32_t PositionDescriptor::GetWidth(Size ParentSize, Size ContentSize) const {
  const auto pos = positions_.find(WBPOSITIONTYPE::WB_WIDTH);
  if (pos == positions_.end()) {
    return 0;
  }
  return static_cast<int32_t>(
      pos->second.GetValue(static_cast<float>(ParentSize.x), ContentSize.x));
}

int32_t PositionDescriptor::GetHeight(Size ParentSize, Size ContentSize) const {
  const auto pos = positions_.find(WBPOSITIONTYPE::WB_HEIGHT);
  if (pos == positions_.end()) {
    return 0;
  }
  return static_cast<int32_t>(
      pos->second.GetValue(static_cast<float>(ParentSize.y), ContentSize.y));
}

bool PositionDescriptor::IsAutoResizer() {
  const bool WidthSet =
      positions_.find(WBPOSITIONTYPE::WB_WIDTH) != positions_.end();
  if (WidthSet && positions_[WBPOSITIONTYPE::WB_WIDTH].IsAutoResizer()) {
    return true;
  }

  const bool HeightSet =
      positions_.find(WBPOSITIONTYPE::WB_HEIGHT) != positions_.end();
  if (HeightSet && positions_[WBPOSITIONTYPE::WB_HEIGHT].IsAutoResizer()) {
    return true;
  }

  return false;
}

void PositionDescriptor::SetAutoSize(WBPOSITIONTYPE p) {
  positions_[p].SetAutoSize(true);
}

//////////////////////////////////////////////////////////////////////////
// position descriptor - pixels

PositionDescriptorPixels::PositionDescriptorPixels() = default;

void PositionDescriptorPixels::SetValue(WBPOSITIONTYPE p, int32_t Pixels) {
  if (static_cast<int32_t>(p) < 0 || p > WBPOSITIONTYPE::WB_HEIGHT) return;
  PositionsAt(p) = Pixels;
  SetAt(p) = true;
}

Rect PositionDescriptorPixels::GetPosition(Size ParentSize) {
  Rect r(0, 0, 0, 0);

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

MosaicImage::MosaicImage() = default;

void MosaicImage::SetPositionValue(WBPOSITIONTYPE p, int32_t Pixels) {
  position_.SetValue(p, Pixels);
}

void MosaicImage::SetTiling(int32_t Axis, bool y) { tiling_[Axis] = y; }

void MosaicImage::SetStretching(int32_t Axis, bool y) { stretching_[Axis] = y; }

void MosaicImage::SetHandle(WBATLASHANDLE handle) { handle_ = handle; }

void MosaicImage::Render(DrawAPI* API, const Rect& Pos) {
  const Rect Croprect = API->GetCropRect();
  API->SetCropRect(Pos + API->GetOffset());

  const Rect displaypos = position_.GetPosition(Pos.Size()) + Pos.TopLeft();

  API->DrawAtlasElement(handle_, displaypos, tiling_[0], tiling_[1],
                        stretching_[0], stretching_[1], color_);

  API->SetCropRect(Croprect);
}

void MosaicImage::SetColor(CColor color) { color_ = color; }

void Mosaic::AddImage(const MosaicImage& Image) { images_.push_back(Image); }

void Mosaic::Render(DrawAPI* API, const Rect& Position) {
  for (auto& image : images_) {
    image.Render(API,
                 Position + Rect(OvershootAt(WBRECTSIDE::WB_RECTSIDE_LEFT),
                                 OvershootAt(WBRECTSIDE::WB_RECTSIDE_TOP),
                                 OvershootAt(WBRECTSIDE::WB_RECTSIDE_RIGHT),
                                 OvershootAt(WBRECTSIDE::WB_RECTSIDE_BOTTOM)));
  }
}

void Mosaic::SetName(std::string_view name) { name_ = name; }

std::string& Mosaic::GetName() { return name_; }

Mosaic::Mosaic(const Mosaic& Copy)
    : name_(Copy.name_), images_(Copy.images_), overshoot_(Copy.overshoot_) {}

Mosaic::Mosaic() = default;

Mosaic& Mosaic::operator=(const Mosaic& Copy) {
  if (&Copy == this) return *this;
  name_ = Copy.name_;
  images_ = Copy.images_;
  overshoot_ = Copy.overshoot_;
  return *this;
}

void Mosaic::Flush() { images_.clear(); }

void Mosaic::SetOverShoot(WBRECTSIDE side, int32_t val) {
  OvershootAt(side) = val;
}

void SkinElement::SetName(std::string_view name) { name_ = name; }

std::string& SkinElement::GetName() { return name_; }

void SkinElement::Render(DrawAPI* API, const Rect& Pos) {
  API->DrawAtlasElement(
      handle_, Pos,
      default_behavior_[0] == WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_TILE,
      default_behavior_[1] == WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_TILE,
      default_behavior_[0] == WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_STRETCH,
      default_behavior_[1] == WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_STRETCH);
}

void SkinElement::SetHandle(WBATLASHANDLE h) { handle_ = h; }

void SkinElement::SetBehavior(int32_t Axis, WBSKINELEMENTBEHAVIOR Behavior) {
  default_behavior_[Axis] = Behavior;
}

WBSKINELEMENTBEHAVIOR SkinElement::GetBehavior(int32_t Axis) {
  return default_behavior_[Axis];
}

SkinElement& SkinElement::operator=(const SkinElement& Copy) {
  if (&Copy == this) return *this;
  name_ = Copy.name_;
  handle_ = Copy.handle_;
  default_behavior_[0] = Copy.default_behavior_[0];
  default_behavior_[1] = Copy.default_behavior_[1];
  return *this;
}

SkinElement::SkinElement(const SkinElement& Copy)
    : name_(Copy.name_),
      handle_(Copy.handle_),
      default_behavior_(Copy.default_behavior_) {}

SkinElement::SkinElement()
    : handle_(0),
      default_behavior_{WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_PIXELCORRECT,
                        WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_PIXELCORRECT} {}

WBATLASHANDLE SkinElement::GetHandle() { return handle_; }

Size SkinElement::GetElementSize(DrawAPI* API) {
  return API->GetAtlasElementSize(handle_);
}

void Skin::RenderElement(DrawAPI* API, WBSKINELEMENTID ID, const Rect& Pos) {
  if (ID == 0xffffffff) return;

  const uint32_t idx = ID & 0x7fffffff;

  if (!(ID & 0x80000000)) {
    if (idx < mosaics_.size()) {
      mosaics_[idx].Render(API, Pos);
    }
    return;
  }

  if (idx < skin_items_.size()) {
    skin_items_[idx].Render(API, Pos);
  }
}

void Skin::RenderElement(DrawAPI* API, std::string_view Name, const Rect& Pos) {
  RenderElement(API, GetElementID(Name), Pos);
}

WBSKINELEMENTID Skin::GetElementID(std::string_view Name) {
  for (uint32_t x = 0; x < mosaics_.size(); x++) {
    if (mosaics_[x].GetName() == Name) return x;
  }

  for (uint32_t x = 0; x < skin_items_.size(); x++) {
    if (skin_items_[x].GetName() == Name) return x | 0x80000000;
  }

  return 0xffffffff;
}

void Skin::AddElement(std::string_view Name, WBATLASHANDLE Handle,
                      WBSKINELEMENTBEHAVIOR Xbehav,
                      WBSKINELEMENTBEHAVIOR Ybehav) {
  for (auto& skin : skin_items_) {
    if (skin.GetName() == Name) {
      skin.SetHandle(Handle);
      skin.SetBehavior(0, Xbehav);
      skin.SetBehavior(1, Ybehav);
      return;
    }
  }

  skin_items_.emplace_back(SkinElement());
  skin_items_.back().SetName(Name);
  skin_items_.back().SetHandle(Handle);
  skin_items_.back().SetBehavior(0, Xbehav);
  skin_items_.back().SetBehavior(1, Ybehav);
}

Mosaic* Skin::AddMosaic(std::string_view Name, std::string_view Description,
                        int32_t OverShootLeft, int32_t OverShootTop,
                        int32_t OverShootRight, int32_t OverShootBottom) {
  for (auto& mosaic : mosaics_) {
    if (mosaic.GetName() == Name) {
      return &mosaic;
    }
  }

  mosaics_.emplace_back(Mosaic());
  mosaics_.back().SetName(Name);

  mosaics_.back().SetOverShoot(WBRECTSIDE::WB_RECTSIDE_LEFT, OverShootLeft);
  mosaics_.back().SetOverShoot(WBRECTSIDE::WB_RECTSIDE_TOP, OverShootTop);
  mosaics_.back().SetOverShoot(WBRECTSIDE::WB_RECTSIDE_RIGHT, OverShootRight);
  mosaics_.back().SetOverShoot(WBRECTSIDE::WB_RECTSIDE_BOTTOM, OverShootBottom);

  auto Lines = Split(Description, ")");
  for (const auto& l : Lines) {
    auto Data = Split(Trim(l), "(");
    MosaicImage i;

    if (Data.size() == 2) {
      SkinElement* e = GetElement(Trim(Data[0]));
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

        auto Data2 = Split(Data[1], ";");

        for (const auto& d2 : Data2) {
          auto keyvalue = Split(d2, ":");
          if (keyvalue.size() > 0) {
            const auto key = Trim(keyvalue[0]);
            std::string value;

            if (keyvalue.size() > 1) value = Trim(keyvalue[1]);

            if (key == "top") {
              i.SetPositionValue(WBPOSITIONTYPE::WB_MARGIN_TOP,
                                 std::stoi(value));
            }
            if (key == "left") {
              i.SetPositionValue(WBPOSITIONTYPE::WB_MARGIN_LEFT,
                                 std::stoi(value));
            }
            if (key == "right") {
              i.SetPositionValue(WBPOSITIONTYPE::WB_MARGIN_RIGHT,
                                 std::stoi(value));
            }
            if (key == "bottom") {
              i.SetPositionValue(WBPOSITIONTYPE::WB_MARGIN_BOTTOM,
                                 std::stoi(value));
            }
            if (key == "width") {
              i.SetPositionValue(WBPOSITIONTYPE::WB_WIDTH, std::stoi(value));
            }
            if (key == "height") {
              i.SetPositionValue(WBPOSITIONTYPE::WB_HEIGHT, std::stoi(value));
            }

            if (key == "repeat-x") i.SetTiling(0, true);
            if (key == "repeat-y") i.SetTiling(1, true);
            if (key == "stretch-x") i.SetStretching(0, true);
            if (key == "stretch-y") i.SetStretching(1, true);
            if (key == "color") {
              uint32_t v = std::stoi(value, nullptr, 16);
              if (value.size() <= 6) v = v | 0xff000000;
              i.SetColor(CColor(v));
            }
          }
        }
        mosaics_.back().AddImage(i);
      }
    }
  }

  return &mosaics_.back();
}

SkinElement* Skin::GetElement(std::string_view Name) {
  for (auto& skin : skin_items_) {
    if (skin.GetName() == Name) {
      return &skin;
    }
  }
  return nullptr;
}

SkinElement* Skin::GetElement(WBSKINELEMENTID id) {
  if (id == 0xffffffff) return nullptr;
  if (!(id & 0x80000000)) return nullptr;  // mosaic
  const uint32_t idx = id & 0x7fffffff;
  if (idx >= skin_items_.size()) return nullptr;
  return &skin_items_[idx];
}

Size Skin::GetElementSize(DrawAPI* API, WBSKINELEMENTID ID) {
  if (ID == 0xffffffff) return Size(0, 0);

  const uint32_t idx = ID & 0x7fffffff;

  if (!(ID & 0x80000000)) return Size(0, 0);  // mosaics don't have sizes

  if (idx < skin_items_.size()) return skin_items_[idx].GetElementSize(API);

  return Size(0, 0);
}

}  // namespace gui
