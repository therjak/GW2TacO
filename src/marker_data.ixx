module;

#include <cstdint>
#include <string_view>
#include <vector>

#include "base/color.h"
#include "util/xml_node.h"

export module taco.marker_data;
import taco.poi_behavior;

export struct MarkerTypeData {
  MarkerTypeData();
  void Read(const CXMLNode& n, bool StoreSaveState);
  void Write(CXMLNode* n);

  struct {
    bool needsExportToUserData : 1;
    bool iconFileSaved : 1;
    bool sizeSaved : 1;
    bool alphaSaved : 1;
    bool fadeNearSaved : 1;
    bool fadeFarSaved : 1;
    bool heightSaved : 1;
    bool behaviorSaved : 1;
    bool resetLengthSaved : 1;
    bool autoTriggerSaved : 1;
    bool hasCountdownSaved : 1;
    bool triggerRangeSaved : 1;
    bool minSizeSaved : 1;
    bool maxSizeSaved : 1;
    bool colorSaved : 1;
    bool trailDataSaved : 1;
    bool animSpeedSaved : 1;
    bool textureSaved : 1;
    bool trailScaleSaved : 1;
    bool toggleCategorySaved : 1;
    bool achievementIdSaved : 1;
    bool achievementBitSaved : 1;
    bool autoTrigger : 1;
    bool hasCountdown : 1;
    bool miniMapVisible : 1;
    bool bigMapVisible : 1;
    bool inGameVisible : 1;
    bool miniMapVisibleSaved : 1;
    bool bigMapVisibleSaved : 1;
    bool inGameVisibleSaved : 1;
    bool scaleWithZoom : 1;
    bool scaleWithZoomSaved : 1;
    bool miniMapSizeSaved : 1;
    bool miniMapFadeOutLevelSaved : 1;
    bool keepOnMapEdge : 1;
    bool keepOnMapEdgeSaved : 1;
    bool infoSaved : 1;
    bool infoRangeSaved : 1;
  } bits{};

  float size = 1.0;
  float alpha = 1.0f;
  float fadeNear = -1;
  float fadeFar = -1;
  float height = 1.5f;
  float triggerRange = 2.0f;
  float animSpeed = 1;
  float trailScale = 1;
  int32_t miniMapSize = 20;
  float miniMapFadeOutLevel = 100.0f;
  float infoRange = 2.0f;

  POIBehavior behavior = POIBehavior::AlwaysVisible;
  CColor color = CColor(0xffffffff);

  int16_t resetLength = 0;
  int16_t minSize = 5;
  int16_t maxSize = 2048;

  std::string_view iconFile;
  std::string_view trailData;
  std::string_view texture;
  std::string_view toggleCategory;
  int16_t achievementId = -1;
  int16_t achievementBit = -1;
  std::string_view info;
};
