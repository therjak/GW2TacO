#pragma once
#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "src/base/color.h"
#include "src/white_board/draw_api.h"

enum class WBMETRICTYPE : uint8_t {
  WB_UNDEFINED = 0,
  WB_PIXELS,
  WB_RELATIVE,
};

class CWBMetricValue {
 public:
  CWBMetricValue();
  void SetMetric(WBMETRICTYPE w, float Value);
  void SetValue(float Relative, float Pixels);
  [[nodiscard]] float GetValue(float ParentSize, int32_t ContentSize) const;
  void SetAutoSize(bool Auto);
  bool IsAutoResizer();

 private:
  float& MetricsAt(WBMETRICTYPE t) { return Metrics[static_cast<uint8_t>(t)]; }
  [[nodiscard]] const float& MetricsAt(WBMETRICTYPE t) const {
    return Metrics[static_cast<uint8_t>(t)];
  }
  bool& MetricsUsedAt(WBMETRICTYPE t) {
    return MetricsUsed[static_cast<uint8_t>(t)];
  }
  [[nodiscard]] const bool& MetricsUsedAt(WBMETRICTYPE t) const {
    return MetricsUsed[static_cast<uint8_t>(t)];
  }
  std::array<float, 3> Metrics{0};
  std::array<bool, 3> MetricsUsed{false};
  bool AutoSize = false;
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

class CWBPositionDescriptor {
 public:
  void SetValue(WBPOSITIONTYPE p, float Relative, float Pixels);
  void SetMetric(WBPOSITIONTYPE p, WBMETRICTYPE m, float Value);
  void SetAutoSize(WBPOSITIONTYPE p);
  void ClearMetrics(WBPOSITIONTYPE p);
  math::CRect GetPosition(math::CSize ParentSize, math::CSize ContentSize,
                          const math::CRect& Original);
  math::CRect GetPadding(math::CSize ParentSize, const math::CRect& BorderSize);

  [[nodiscard]] bool IsWidthSet() const;
  [[nodiscard]] bool IsHeightSet() const;
  [[nodiscard]] int32_t GetWidth(math::CSize ParentSize,
                                 math::CSize ContentSize) const;
  [[nodiscard]] int32_t GetHeight(math::CSize ParentSize,
                                  math::CSize ContentSize) const;

  bool IsAutoResizer();

 private:
  std::unordered_map<WBPOSITIONTYPE, CWBMetricValue> Positions;
};

class CWBPositionDescriptorPixels {
 public:
  CWBPositionDescriptorPixels();
  void SetValue(WBPOSITIONTYPE p, int32_t Pixels);
  FORCEINLINE math::CRect GetPosition(math::CSize ParentSize);

 private:
  bool& SetAt(WBPOSITIONTYPE p) { return Set[static_cast<uint16_t>(p)]; }
  int32_t& PositionsAt(WBPOSITIONTYPE p) {
    return Positions[static_cast<uint16_t>(p)];
  }

  std::array<bool, 6> Set = {false};
  std::array<int32_t, 6> Positions = {0};
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

class CWBSkinElement {
 public:
  CWBSkinElement();
  CWBSkinElement(const CWBSkinElement& Copy);
  CWBSkinElement& operator=(const CWBSkinElement& Copy);

  void SetHandle(WBATLASHANDLE h);
  void SetBehavior(int32_t Axis, WBSKINELEMENTBEHAVIOR Behavior);
  WBSKINELEMENTBEHAVIOR GetBehavior(int32_t Axis);
  void SetName(std::string_view Name);
  WBATLASHANDLE GetHandle();
  std::string& GetName();

  FORCEINLINE void Render(CWBDrawAPI* API, const math::CRect& Pos);
  math::CSize GetElementSize(CWBDrawAPI* API);

 private:
  std::string Name;
  WBATLASHANDLE Handle;
  // x-y stretching behaviors
  std::array<WBSKINELEMENTBEHAVIOR, 2> DefaultBehavior{};
};

class CWBMosaicImage {
 public:
  CWBMosaicImage();
  void SetPositionValue(WBPOSITIONTYPE p, int32_t Pixels);
  void SetTiling(int32_t Axis, bool v);
  void SetStretching(int32_t Axis, bool v);
  void SetHandle(WBATLASHANDLE handle);
  void SetColor(CColor color);

  FORCEINLINE void Render(CWBDrawAPI* API, const math::CRect& Pos);

 private:
  CWBPositionDescriptorPixels Position;
  std::array<bool, 2> Tiling = {false};
  std::array<bool, 2> Stretching = {false};
  WBATLASHANDLE Handle = 0;
  CColor Color = CColor(0xffffffff);
};

class CWBMosaic {
 public:
  CWBMosaic();
  CWBMosaic(const CWBMosaic& Copy);
  CWBMosaic& operator=(const CWBMosaic& Copy);

  void SetName(std::string_view Name);
  std::string& GetName();
  void AddImage(const CWBMosaicImage& Image);
  void Flush();
  void Render(CWBDrawAPI* API, const math::CRect& Position);
  void SetOverShoot(WBRECTSIDE side, int32_t val);

 private:
  int32_t& OvershootAt(WBRECTSIDE s) {
    return Overshoot[static_cast<uint8_t>(s)];
  }

  std::string Name;
  std::vector<CWBMosaicImage> Images;
  std::array<int32_t, 4> Overshoot = {0};
};

typedef uint32_t WBSKINELEMENTID;

class CWBSkin {
 public:
  void AddElement(std::string_view Name, WBATLASHANDLE Handle,
                  WBSKINELEMENTBEHAVIOR Xbehav, WBSKINELEMENTBEHAVIOR Ybehav);
  CWBMosaic* AddMosaic(std::string_view Name, std::string_view Description,
                       int32_t OverShootLeft = 0, int32_t OverShootTop = 0,
                       int32_t OverShootRight = 0, int32_t OverShootBottom = 0);

  void RenderElement(CWBDrawAPI* API, WBSKINELEMENTID ID,
                     const math::CRect& Pos);
  void RenderElement(CWBDrawAPI* API, std::string_view Name,
                     const math::CRect& Pos);
  WBSKINELEMENTID GetElementID(std::string_view Name);
  CWBSkinElement* GetElement(WBSKINELEMENTID id);
  math::CSize GetElementSize(CWBDrawAPI* API, WBSKINELEMENTID id);

 private:
  CWBSkinElement* GetElement(std::string_view Name);

  std::vector<CWBSkinElement> SkinItems;
  std::vector<CWBMosaic> Mosaics;
};
