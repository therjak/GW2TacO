module;
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/base/color.h"

export module whiteboard:skin;

import :draw_api;
import math;

export namespace gui {

enum class MetricType : uint8_t {
  WB_UNDEFINED = 0,
  WB_PIXELS,
  WB_RELATIVE,
};

class MetricValue {
 public:
  MetricValue();
  void SetMetric(MetricType w, float Value);
  void SetValue(float Relative, float Pixels);
  [[nodiscard]] float GetValue(float ParentSize, int32_t ContentSize) const;
  void SetAutoSize(bool Auto);
  bool IsAutoResizer();

 private:
  float& MetricsAt(MetricType t) { return metrics_[static_cast<uint8_t>(t)]; }
  [[nodiscard]] const float& MetricsAt(MetricType t) const {
    return metrics_[static_cast<uint8_t>(t)];
  }
  bool& MetricsUsedAt(MetricType t) {
    return metrics_used_[static_cast<uint8_t>(t)];
  }
  [[nodiscard]] const bool& MetricsUsedAt(MetricType t) const {
    return metrics_used_[static_cast<uint8_t>(t)];
  }
  std::array<float, 3> metrics_{0};
  std::array<bool, 3> metrics_used_{false};
  bool auto_size_ = false;
};

enum class PositionType : uint16_t {
  WB_MARGIN_LEFT = 0,
  WB_MARGIN_RIGHT = 1,
  WB_MARGIN_TOP = 2,
  WB_MARGIN_BOTTOM = 3,
  WB_WIDTH = 4,
  WB_HEIGHT = 5,
  WB_PADDING_LEFT = 6,
  WB_PADDING_RIGHT = 7,
  WB_PADDING_TOP = 8,
  WB_PADDING_BOTTOM = 9
};

class PositionDescriptor {
 public:
  void SetValue(PositionType p, float Relative, float Pixels);
  void SetMetric(PositionType p, MetricType m, float Value);
  void SetAutoSize(PositionType p);
  void ClearMetrics(PositionType p);
  math::Rect GetPosition(math::Size ParentSize, math::Size ContentSize,
                         const math::Rect& Original);
  math::Rect GetPadding(math::Size ParentSize, const math::Rect& BorderSize);

  [[nodiscard]] bool IsWidthSet() const;
  [[nodiscard]] bool IsHeightSet() const;
  [[nodiscard]] int32_t GetWidth(math::Size ParentSize,
                                 math::Size ContentSize) const;
  [[nodiscard]] int32_t GetHeight(math::Size ParentSize,
                                  math::Size ContentSize) const;

  bool IsAutoResizer();

 private:
  std::unordered_map<PositionType, MetricValue> positions_;
};

class PositionDescriptorPixels {
 public:
  PositionDescriptorPixels();
  void SetValue(PositionType p, int32_t Pixels);
  math::Rect GetPosition(math::Size ParentSize);

 private:
  bool& SetAt(PositionType p) { return set_[static_cast<uint16_t>(p)]; }
  int32_t& PositionsAt(PositionType p) {
    return positions_[static_cast<uint16_t>(p)];
  }

  std::array<bool, 6> set_ = {false};
  std::array<int32_t, 6> positions_ = {0};
};

enum class SkinElementBehavior : uint8_t {
  WB_SKINBEHAVIOR_PIXELCORRECT = 0,
  WB_SKINBEHAVIOR_STRETCH,
  WB_SKINBEHAVIOR_TILE,
};

enum class RectSide : uint8_t {
  WB_RECTSIDE_LEFT = 0,
  WB_RECTSIDE_TOP = 1,
  WB_RECTSIDE_RIGHT = 2,
  WB_RECTSIDE_BOTTOM = 3,
};

class SkinElement {
 public:
  SkinElement();
  SkinElement(const SkinElement& Copy);
  SkinElement& operator=(const SkinElement& Copy);

  void SetHandle(AtlasHandle h);
  void SetBehavior(int32_t Axis, SkinElementBehavior Behavior);
  SkinElementBehavior GetBehavior(int32_t Axis);
  void SetName(std::string_view Name);
  AtlasHandle GetHandle();
  std::string& GetName();

  void Render(DrawAPI* API, const math::Rect& Pos);
  math::Size GetElementSize(DrawAPI* API);

 private:
  std::string name_;
  AtlasHandle handle_;
  // x-y stretching behaviors
  std::array<SkinElementBehavior, 2> default_behavior_{};
};

class MosaicImage {
 public:
  MosaicImage();
  void SetPositionValue(PositionType p, int32_t Pixels);
  void SetTiling(int32_t Axis, bool v);
  void SetStretching(int32_t Axis, bool v);
  void SetHandle(AtlasHandle handle);
  void SetColor(CColor color);

  void Render(DrawAPI* API, const math::Rect& Pos);

 private:
  PositionDescriptorPixels position_;
  std::array<bool, 2> tiling_ = {false};
  std::array<bool, 2> stretching_ = {false};
  AtlasHandle handle_ = 0;
  CColor color_ = CColor(0xffffffff);
};

class Mosaic {
 public:
  Mosaic();
  Mosaic(const Mosaic& Copy);
  Mosaic& operator=(const Mosaic& Copy);

  void SetName(std::string_view Name);
  std::string& GetName();
  void AddImage(const MosaicImage& Image);
  void Flush();
  void Render(DrawAPI* API, const math::Rect& Position);
  void SetOverShoot(RectSide side, int32_t val);

 private:
  int32_t& OvershootAt(RectSide s) {
    return overshoot_[static_cast<uint8_t>(s)];
  }

  std::string name_;
  std::vector<MosaicImage> images_;
  std::array<int32_t, 4> overshoot_ = {0};
};

typedef uint32_t SkinElementId;

class Skin {
 public:
  void AddElement(std::string_view Name, AtlasHandle Handle,
                  SkinElementBehavior Xbehav, SkinElementBehavior Ybehav);
  Mosaic* AddMosaic(std::string_view Name, std::string_view Description,
                    int32_t OverShootLeft = 0, int32_t OverShootTop = 0,
                    int32_t OverShootRight = 0, int32_t OverShootBottom = 0);

  void RenderElement(DrawAPI* API, SkinElementId ID, const math::Rect& Pos);
  void RenderElement(DrawAPI* API, std::string_view Name,
                     const math::Rect& Pos);
  SkinElementId GetElementID(std::string_view Name);
  SkinElement* GetElement(SkinElementId id);
  math::Size GetElementSize(DrawAPI* API, SkinElementId id);

 private:
  SkinElement* GetElement(std::string_view Name);

  std::vector<SkinElement> skin_items_;
  std::vector<Mosaic> mosaics_;
};

}  // namespace gui
