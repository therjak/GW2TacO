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

enum class WBMETRICTYPE : uint8_t {
  WB_UNDEFINED = 0,
  WB_PIXELS,
  WB_RELATIVE,
};

class MetricValue {
 public:
  MetricValue();
  void SetMetric(WBMETRICTYPE w, float Value);
  void SetValue(float Relative, float Pixels);
  [[nodiscard]] float GetValue(float ParentSize, int32_t ContentSize) const;
  void SetAutoSize(bool Auto);
  bool IsAutoResizer();

 private:
  float& MetricsAt(WBMETRICTYPE t) { return metrics_[static_cast<uint8_t>(t)]; }
  [[nodiscard]] const float& MetricsAt(WBMETRICTYPE t) const {
    return metrics_[static_cast<uint8_t>(t)];
  }
  bool& MetricsUsedAt(WBMETRICTYPE t) {
    return metrics_used_[static_cast<uint8_t>(t)];
  }
  [[nodiscard]] const bool& MetricsUsedAt(WBMETRICTYPE t) const {
    return metrics_used_[static_cast<uint8_t>(t)];
  }
  std::array<float, 3> metrics_{0};
  std::array<bool, 3> metrics_used_{false};
  bool auto_size_ = false;
};

enum class WBPOSITIONTYPE : uint16_t {
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
  void SetValue(WBPOSITIONTYPE p, float Relative, float Pixels);
  void SetMetric(WBPOSITIONTYPE p, WBMETRICTYPE m, float Value);
  void SetAutoSize(WBPOSITIONTYPE p);
  void ClearMetrics(WBPOSITIONTYPE p);
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
  std::unordered_map<WBPOSITIONTYPE, MetricValue> positions_;
};

class PositionDescriptorPixels {
 public:
  PositionDescriptorPixels();
  void SetValue(WBPOSITIONTYPE p, int32_t Pixels);
  math::Rect GetPosition(math::Size ParentSize);

 private:
  bool& SetAt(WBPOSITIONTYPE p) { return set_[static_cast<uint16_t>(p)]; }
  int32_t& PositionsAt(WBPOSITIONTYPE p) {
    return positions_[static_cast<uint16_t>(p)];
  }

  std::array<bool, 6> set_ = {false};
  std::array<int32_t, 6> positions_ = {0};
};

enum class WBSKINELEMENTBEHAVIOR : uint8_t {
  WB_SKINBEHAVIOR_PIXELCORRECT = 0,
  WB_SKINBEHAVIOR_STRETCH,
  WB_SKINBEHAVIOR_TILE,
};

enum class WBRECTSIDE : uint8_t {
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

  void SetHandle(WBATLASHANDLE h);
  void SetBehavior(int32_t Axis, WBSKINELEMENTBEHAVIOR Behavior);
  WBSKINELEMENTBEHAVIOR GetBehavior(int32_t Axis);
  void SetName(std::string_view Name);
  WBATLASHANDLE GetHandle();
  std::string& GetName();

  void Render(DrawAPI* API, const math::Rect& Pos);
  math::Size GetElementSize(DrawAPI* API);

 private:
  std::string name_;
  WBATLASHANDLE handle_;
  // x-y stretching behaviors
  std::array<WBSKINELEMENTBEHAVIOR, 2> default_behavior_{};
};

class MosaicImage {
 public:
  MosaicImage();
  void SetPositionValue(WBPOSITIONTYPE p, int32_t Pixels);
  void SetTiling(int32_t Axis, bool v);
  void SetStretching(int32_t Axis, bool v);
  void SetHandle(WBATLASHANDLE handle);
  void SetColor(CColor color);

  void Render(DrawAPI* API, const math::Rect& Pos);

 private:
  PositionDescriptorPixels position_;
  std::array<bool, 2> tiling_ = {false};
  std::array<bool, 2> stretching_ = {false};
  WBATLASHANDLE handle_ = 0;
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
  void SetOverShoot(WBRECTSIDE side, int32_t val);

 private:
  int32_t& OvershootAt(WBRECTSIDE s) {
    return overshoot_[static_cast<uint8_t>(s)];
  }

  std::string name_;
  std::vector<MosaicImage> images_;
  std::array<int32_t, 4> overshoot_ = {0};
};

typedef uint32_t WBSKINELEMENTID;

class Skin {
 public:
  void AddElement(std::string_view Name, WBATLASHANDLE Handle,
                  WBSKINELEMENTBEHAVIOR Xbehav, WBSKINELEMENTBEHAVIOR Ybehav);
  Mosaic* AddMosaic(std::string_view Name, std::string_view Description,
                    int32_t OverShootLeft = 0, int32_t OverShootTop = 0,
                    int32_t OverShootRight = 0, int32_t OverShootBottom = 0);

  void RenderElement(DrawAPI* API, WBSKINELEMENTID ID, const math::Rect& Pos);
  void RenderElement(DrawAPI* API, std::string_view Name,
                     const math::Rect& Pos);
  WBSKINELEMENTID GetElementID(std::string_view Name);
  SkinElement* GetElement(WBSKINELEMENTID id);
  math::Size GetElementSize(DrawAPI* API, WBSKINELEMENTID id);

 private:
  SkinElement* GetElement(std::string_view Name);

  std::vector<SkinElement> skin_items_;
  std::vector<Mosaic> mosaics_;
};

}  // namespace gui
