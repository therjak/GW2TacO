#pragma once
#include <unordered_map>
#include <vector>

#include "../BaseLib/Color.h"
#include "DrawAPI.h"

enum WBMETRICTYPE {
  WB_UNDEFINED = 0,
  WB_PIXELS,
  WB_RELATIVE,

  WB_METRIC_COUNT,  // used as array size, don't remove
};

class CWBMetricValue {
  float Metrics[WB_METRIC_COUNT]{0};
  bool MetricsUsed[WB_METRIC_COUNT]{0};
  bool AutoSize = false;

 public:
  CWBMetricValue();
  void SetMetric(WBMETRICTYPE w, float Value);
  void SetValue(float Relative, float Pixels);
  float GetValue(float ParentSize, int32_t ContentSize);
  void SetAutoSize(bool Auto);
  bool IsAutoResizer();
};

enum WBPOSITIONTYPE {
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
  std::unordered_map<WBPOSITIONTYPE, CWBMetricValue> Positions;

 public:
  void SetValue(WBPOSITIONTYPE p, float Relative, float Pixels);
  void SetMetric(WBPOSITIONTYPE p, WBMETRICTYPE m, float Value);
  void SetAutoSize(WBPOSITIONTYPE p);
  void ClearMetrics(WBPOSITIONTYPE p);
  CRect GetPosition(CSize ParentSize, CSize ContentSize, const CRect& Original);
  CRect GetPadding(CSize ParentSize, const CRect& BorderSize);

  bool IsWidthSet();
  bool IsHeightSet();
  int32_t GetWidth(CSize ParentSize, CSize ContentSize);
  int32_t GetHeight(CSize ParentSize, CSize ContentSize);

  bool IsAutoResizer();
};

class CWBPositionDescriptorPixels {
  bool Set[6];
  int32_t Positions[6];

 public:
  CWBPositionDescriptorPixels();
  void SetValue(WBPOSITIONTYPE p, int32_t Pixels);
  FORCEINLINE CRect GetPosition(CSize ParentSize);
};

enum WBSKINELEMENTBEHAVIOR {
  WB_SKINBEHAVIOR_PIXELCORRECT = 0,
  WB_SKINBEHAVIOR_STRETCH,
  WB_SKINBEHAVIOR_TILE,
};

enum WBRECTSIDE {
  WB_RECTSIDE_LEFT = 0,
  WB_RECTSIDE_TOP = 1,
  WB_RECTSIDE_RIGHT = 2,
  WB_RECTSIDE_BOTTOM = 3,
};

class CWBSkinElement {
  std::string Name;
  WBATLASHANDLE Handle;
  WBSKINELEMENTBEHAVIOR DefaultBehavior[2];  // x-y stretching behaviors

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

  FORCEINLINE void Render(CWBDrawAPI* API, CRect& Pos);
  CSize GetElementSize(CWBDrawAPI* API);
};

class CWBMosaicImage {
  CWBPositionDescriptorPixels Position;
  bool Tiling[2];
  bool Stretching[2];
  WBATLASHANDLE Handle;
  CColor Color;

 public:
  CWBMosaicImage();
  void SetPositionValue(WBPOSITIONTYPE p, int32_t Pixels);
  void SetTiling(int32_t Axis, bool v);
  void SetStretching(int32_t Axis, bool v);
  void SetHandle(WBATLASHANDLE handle);
  void SetColor(CColor color);

  FORCEINLINE void Render(CWBDrawAPI* API, CRect& Pos);
};

class CWBMosaic {
  std::string Name;
  std::vector<CWBMosaicImage> Images;
  int32_t Overshoot[4];

 public:
  CWBMosaic();
  CWBMosaic(const CWBMosaic& Copy);
  CWBMosaic& operator=(const CWBMosaic& Copy);

  void SetName(std::string_view Name);
  std::string& GetName();
  void AddImage(const CWBMosaicImage& Image);
  void Flush();
  void Render(CWBDrawAPI* API, const CRect& Position);
  void SetOverShoot(WBRECTSIDE side, int32_t val);
};

typedef uint32_t WBSKINELEMENTID;

class CWBSkin {
  std::vector<CWBSkinElement> SkinItems;
  std::vector<CWBMosaic> Mosaics;

  CWBSkinElement* GetElement(std::string_view Name);

 public:
  void AddElement(std::string_view Name, WBATLASHANDLE Handle,
                  WBSKINELEMENTBEHAVIOR Xbehav, WBSKINELEMENTBEHAVIOR Ybehav);
  CWBMosaic* AddMosaic(std::string_view Name, std::string_view Description,
                       int32_t OverShootLeft = 0, int32_t OverShootTop = 0,
                       int32_t OverShootRight = 0, int32_t OverShootBottom = 0);

  void RenderElement(CWBDrawAPI* API, WBSKINELEMENTID ID, CRect& Pos);
  void RenderElement(CWBDrawAPI* API, std::string_view Name, CRect& Pos);
  WBSKINELEMENTID GetElementID(std::string_view Name);
  CWBSkinElement* GetElement(WBSKINELEMENTID id);
  CSize GetElementSize(CWBDrawAPI* API, WBSKINELEMENTID id);
};
