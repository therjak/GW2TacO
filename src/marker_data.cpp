module;

#include <cstdio>
#include <cstring>
#include <format>
#include "base/color.h"
#include "poi_behavior.h"
#include "util/xml_node.h"

module taco.marker_data;

import taco.string_set;

MarkerTypeData::MarkerTypeData() {
  memset(&bits, 0, sizeof(bits));
  bits.miniMapVisible = true;
  bits.bigMapVisible = true;
  bits.inGameVisible = true;
  bits.scaleWithZoom = true;
}

void MarkerTypeData::Read(const CXMLNode& n, bool StoreSaveState) {
  bool _iconFileSaved = n.HasAttribute("iconFile");
  bool _sizeSaved = n.HasAttribute("iconSize");
  bool _alphaSaved = n.HasAttribute("alpha");
  bool _fadeNearSaved = n.HasAttribute("fadeNear");
  bool _fadeFarSaved = n.HasAttribute("fadeFar");
  bool _heightSaved = n.HasAttribute("heightOffset");
  bool _behaviorSaved = n.HasAttribute("behavior");
  bool _resetLengthSaved = n.HasAttribute("resetLength");
  bool _autoTriggerSaved = n.HasAttribute("autoTrigger");
  bool _hasCountdownSaved = n.HasAttribute("hasCountdown");
  bool _triggerRangeSaved = n.HasAttribute("triggerRange");
  bool _minSizeSaved = n.HasAttribute("minSize");
  bool _maxSizeSaved = n.HasAttribute("maxSize");
  bool _colorSaved = n.HasAttribute("color");
  bool _trailDataSaved = n.HasAttribute("trailData");
  bool _animSpeedSaved = n.HasAttribute("animSpeed");
  bool _textureSaved = n.HasAttribute("texture");
  bool _trailScaleSaved = n.HasAttribute("trailScale");
  bool _toggleCategorySaved = n.HasAttribute("toggleCategory");
  bool _achievementIdSaved = n.HasAttribute("achievementId");
  bool _achievementBitSaved = n.HasAttribute("achievementBit");
  bool _miniMapVisibleSaved = n.HasAttribute("miniMapVisibility");
  bool _bigMapVisibleSaved = n.HasAttribute("mapVisibility");
  bool _inGameVisibleSaved = n.HasAttribute("inGameVisibility");
  bool _scaleWithZoomSaved = n.HasAttribute("scaleOnMapWithZoom");
  bool _miniMapSizeSaved = n.HasAttribute("mapDisplaySize");
  bool _miniMapFadeOutLevelSaved = n.HasAttribute("mapFadeoutScaleLevel");
  bool _keepOnMapEdgeSaved = n.HasAttribute("keepOnMapEdge");
  bool _infoSaved = n.HasAttribute("info");
  bool _infoRangeSaved = n.HasAttribute("infoRange");

  if (StoreSaveState) {
    bits.iconFileSaved = _iconFileSaved;
    bits.sizeSaved = _sizeSaved;
    bits.alphaSaved = _alphaSaved;
    bits.fadeNearSaved = _fadeNearSaved;
    bits.fadeFarSaved = _fadeFarSaved;
    bits.heightSaved = _heightSaved;
    bits.behaviorSaved = _behaviorSaved;
    bits.resetLengthSaved = _resetLengthSaved;
    bits.autoTriggerSaved = _autoTriggerSaved;
    bits.hasCountdownSaved = _hasCountdownSaved;
    bits.triggerRangeSaved = _triggerRangeSaved;
    bits.minSizeSaved = _minSizeSaved;
    bits.maxSizeSaved = _maxSizeSaved;
    bits.colorSaved = _colorSaved;
    bits.trailDataSaved = _trailDataSaved;
    bits.animSpeedSaved = _animSpeedSaved;
    bits.textureSaved = _textureSaved;
    bits.trailScaleSaved = _trailScaleSaved;
    bits.toggleCategorySaved = _toggleCategorySaved;
    bits.achievementIdSaved = _achievementIdSaved;
    bits.achievementBitSaved = _achievementBitSaved;
    bits.miniMapVisibleSaved = _miniMapVisibleSaved;
    bits.bigMapVisibleSaved = _bigMapVisibleSaved;
    bits.inGameVisibleSaved = _inGameVisibleSaved;
    bits.scaleWithZoomSaved = _scaleWithZoomSaved;
    bits.miniMapSizeSaved = _miniMapSizeSaved;
    bits.miniMapFadeOutLevelSaved = _miniMapFadeOutLevelSaved;
    bits.keepOnMapEdgeSaved = _keepOnMapEdgeSaved;
    bits.infoSaved = _infoSaved;
    bits.infoRangeSaved = _infoRangeSaved;
  }

  if (_iconFileSaved) iconFile = AddStringToSet(n.GetAttribute("iconFile"));

  if (_sizeSaved) n.GetAttributeAsFloat("iconSize", &size);

  if (_alphaSaved) n.GetAttributeAsFloat("alpha", &alpha);
  if (_fadeNearSaved) n.GetAttributeAsFloat("fadeNear", &fadeNear);
  if (_fadeFarSaved) n.GetAttributeAsFloat("fadeFar", &fadeFar);
  if (_heightSaved) n.GetAttributeAsFloat("heightOffset", &height);
  if (_behaviorSaved) {
    int32_t x = 0;
    n.GetAttributeAsInteger("behavior", &x);
    behavior = static_cast<POIBehavior>(x);
  }
  if (_resetLengthSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("resetLength", &val);
    resetLength = val;
  }
  if (_autoTriggerSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("autoTrigger", &val);
    bits.autoTrigger = val != 0;
  }
  if (_hasCountdownSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("hasCountdown", &val);
    bits.hasCountdown = val != 0;
  }
  if (_triggerRangeSaved) n.GetAttributeAsFloat("triggerRange", &triggerRange);
  if (_minSizeSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("minSize", &val);
    minSize = val;
  }
  if (_maxSizeSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("maxSize", &val);
    maxSize = val;
  }
  if (_colorSaved) {
    auto colorStr = n.GetAttributeAsString("color");
    uint32_t colHex = 0xffffffff;
    std::sscanf(colorStr.c_str(), "%x", &colHex);
    color = CColor(colHex);
  }
  if (_trailDataSaved) trailData = AddStringToSet(n.GetAttribute("trailData"));
  if (_animSpeedSaved) n.GetAttributeAsFloat("animSpeed", &animSpeed);
  if (_textureSaved) texture = AddStringToSet(n.GetAttribute("texture"));
  if (_trailScaleSaved) n.GetAttributeAsFloat("trailScale", &trailScale);
  if (_toggleCategorySaved) {
    toggleCategory = AddStringToSet(n.GetAttribute("toggleCategory"));
  }
  if (_achievementIdSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("achievementId", &val);
    achievementId = val;
  }
  if (_achievementBitSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("achievementBit", &val);
    achievementBit = val;
  }
  if (_miniMapVisibleSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("miniMapVisibility", &val);
    bits.miniMapVisible = val != 0;
  }
  if (_bigMapVisibleSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("mapVisibility", &val);
    bits.bigMapVisible = val != 0;
  }
  if (_inGameVisibleSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("inGameVisibility", &val);
    bits.inGameVisible = val != 0;
  }
  if (_scaleWithZoomSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("scaleOnMapWithZoom", &val);
    bits.scaleWithZoom = val != 0;
  }
  if (_miniMapFadeOutLevelSaved) {
    n.GetAttributeAsFloat("mapFadeoutScaleLevel", &miniMapFadeOutLevel);
  }

  if (_miniMapSizeSaved) {
    int32_t x = 0;
    n.GetAttributeAsInteger("mapDisplaySize", &x);
    miniMapSize = x;
  }
  if (_keepOnMapEdgeSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("keepOnMapEdge", &val);
    bits.keepOnMapEdge = val != 0;
  }
  if (_infoSaved) info = AddStringToSet(n.GetAttributeAsString("info"));
  if (_infoRangeSaved) n.GetAttributeAsFloat("infoRange", &infoRange);
}

void MarkerTypeData::Write(CXMLNode* n) {
  if (bits.iconFileSaved) n->SetAttribute("iconFile", iconFile);
  if (bits.sizeSaved) n->SetAttributeFromFloat("iconSize", size);
  if (bits.alphaSaved) n->SetAttributeFromFloat("alpha", alpha);
  if (bits.fadeNearSaved) n->SetAttributeFromFloat("fadeNear", fadeNear);
  if (bits.fadeFarSaved) n->SetAttributeFromFloat("fadeFar", fadeFar);
  if (bits.heightSaved) n->SetAttributeFromFloat("heightOffset", height);
  if (bits.behaviorSaved) {
    n->SetAttributeFromInteger("behavior", static_cast<int32_t>(behavior));
  }
  if (bits.resetLengthSaved) {
    n->SetAttributeFromInteger("resetLength", resetLength);
  }
  if (bits.autoTriggerSaved) {
    n->SetAttributeFromInteger("autoTrigger", bits.autoTrigger);
  }
  if (bits.hasCountdownSaved) {
    n->SetAttributeFromInteger("hasCountdown", bits.hasCountdown);
  }
  if (bits.triggerRangeSaved) {
    n->SetAttributeFromFloat("triggerRange", triggerRange);
  }
  if (bits.minSizeSaved) n->SetAttributeFromInteger("minSize", minSize);
  if (bits.maxSizeSaved) n->SetAttributeFromInteger("maxSize", maxSize);
  if (bits.colorSaved) {
    n->SetAttribute("color", std::format("{:x}", color.argb()));
  }
  if (bits.trailDataSaved) n->SetAttribute("trailData", trailData);
  if (bits.animSpeedSaved) n->SetAttributeFromFloat("animSpeed", animSpeed);
  if (bits.textureSaved) n->SetAttribute("texture", texture);
  if (bits.trailScaleSaved) n->SetAttributeFromFloat("trailScale", trailScale);
  if (bits.toggleCategorySaved) {
    n->SetAttribute("toggleCategory", toggleCategory);
  }
  if (bits.achievementIdSaved) {
    n->SetAttributeFromInteger("achievementId", achievementId);
  }
  if (bits.achievementBitSaved) {
    n->SetAttributeFromInteger("achievementBit", achievementBit);
  }
  if (bits.miniMapVisibleSaved) {
    n->SetAttributeFromInteger("miniMapVisibility", bits.miniMapVisible);
  }
  if (bits.bigMapVisibleSaved) {
    n->SetAttributeFromInteger("mapVisibility", bits.bigMapVisible);
  }
  if (bits.inGameVisibleSaved) {
    n->SetAttributeFromInteger("inGameVisibility", bits.inGameVisible);
  }
  if (bits.scaleWithZoomSaved) {
    n->SetAttributeFromInteger("scaleOnMapWithZoom", bits.scaleWithZoom);
  }
  if (bits.miniMapFadeOutLevelSaved) {
    n->SetAttributeFromFloat("mapFadeoutScaleLevel", miniMapFadeOutLevel);
  }
  if (bits.miniMapSizeSaved) {
    n->SetAttributeFromInteger("mapDisplaySize", miniMapSize);
  }
  if (bits.keepOnMapEdgeSaved) {
    n->SetAttributeFromInteger("keepOnMapEdge", bits.keepOnMapEdge);
  }
  if (bits.infoSaved) n->SetAttribute("info", info);
  if (bits.infoRangeSaved) n->SetAttributeFromFloat("infoRange", infoRange);
}
