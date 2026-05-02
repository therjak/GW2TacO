module;

#include <cstdio>
#include <cstring>
#include <format>

#include "base/color.h"

module taco.marker_data;

import taco.poi_behavior;
import taco.string_set;
import xml;

MarkerTypeData::MarkerTypeData() {
  memset(&bits_, 0, sizeof(bits_));
  bits_.mini_map_visible_ = true;
  bits_.big_map_visible_ = true;
  bits_.in_game_visible_ = true;
  bits_.scale_with_zoom_ = true;
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
    bits_.icon_file_saved_ = _iconFileSaved;
    bits_.size_saved_ = _sizeSaved;
    bits_.alpha_saved_ = _alphaSaved;
    bits_.fade_near_saved_ = _fadeNearSaved;
    bits_.fade_far_saved_ = _fadeFarSaved;
    bits_.height_saved_ = _heightSaved;
    bits_.behavior_saved_ = _behaviorSaved;
    bits_.reset_length_saved_ = _resetLengthSaved;
    bits_.auto_trigger_saved_ = _autoTriggerSaved;
    bits_.has_countdown_saved_ = _hasCountdownSaved;
    bits_.trigger_range_saved_ = _triggerRangeSaved;
    bits_.min_size_saved_ = _minSizeSaved;
    bits_.max_size_saved_ = _maxSizeSaved;
    bits_.color_saved_ = _colorSaved;
    bits_.trail_data_saved_ = _trailDataSaved;
    bits_.anim_speed_saved_ = _animSpeedSaved;
    bits_.texture_saved_ = _textureSaved;
    bits_.trail_scale_saved_ = _trailScaleSaved;
    bits_.toggle_category_saved_ = _toggleCategorySaved;
    bits_.achievement_id_saved_ = _achievementIdSaved;
    bits_.achievement_bit_saved_ = _achievementBitSaved;
    bits_.mini_map_visible_saved_ = _miniMapVisibleSaved;
    bits_.big_map_visible_saved_ = _bigMapVisibleSaved;
    bits_.in_game_visible_saved_ = _inGameVisibleSaved;
    bits_.scale_with_zoom_saved_ = _scaleWithZoomSaved;
    bits_.mini_map_size_saved_ = _miniMapSizeSaved;
    bits_.mini_map_fade_out_level_saved_ = _miniMapFadeOutLevelSaved;
    bits_.keep_on_map_edge_saved_ = _keepOnMapEdgeSaved;
    bits_.info_saved_ = _infoSaved;
    bits_.info_range_saved_ = _infoRangeSaved;
  }

  if (_iconFileSaved) icon_file_ = AddStringToSet(n.GetAttribute("iconFile"));

  if (_sizeSaved) n.GetAttributeAsFloat("iconSize", &size_);

  if (_alphaSaved) n.GetAttributeAsFloat("alpha", &alpha_);
  if (_fadeNearSaved) n.GetAttributeAsFloat("fadeNear", &fade_near_);
  if (_fadeFarSaved) n.GetAttributeAsFloat("fadeFar", &fade_far_);
  if (_heightSaved) n.GetAttributeAsFloat("heightOffset", &height_);
  if (_behaviorSaved) {
    int32_t x = 0;
    n.GetAttributeAsInteger("behavior", &x);
    behavior_ = static_cast<POIBehavior>(x);
  }
  if (_resetLengthSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("resetLength", &val);
    reset_length_ = val;
  }
  if (_autoTriggerSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("autoTrigger", &val);
    bits_.auto_trigger_ = val != 0;
  }
  if (_hasCountdownSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("hasCountdown", &val);
    bits_.has_countdown_ = val != 0;
  }
  if (_triggerRangeSaved)
    n.GetAttributeAsFloat("triggerRange", &trigger_range_);
  if (_minSizeSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("minSize", &val);
    min_size_ = val;
  }
  if (_maxSizeSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("maxSize", &val);
    max_size_ = val;
  }
  if (_colorSaved) {
    auto colorStr = n.GetAttributeAsString("color");
    uint32_t colHex = 0xffffffff;
    std::sscanf(colorStr.c_str(), "%x", &colHex);
    color_ = CColor(colHex);
  }
  if (_trailDataSaved)
    trail_data_ = AddStringToSet(n.GetAttribute("trailData"));
  if (_animSpeedSaved) n.GetAttributeAsFloat("animSpeed", &anim_speed_);
  if (_textureSaved) texture_ = AddStringToSet(n.GetAttribute("texture"));
  if (_trailScaleSaved) n.GetAttributeAsFloat("trailScale", &trail_scale_);
  if (_toggleCategorySaved) {
    toggle_category_ = AddStringToSet(n.GetAttribute("toggleCategory"));
  }
  if (_achievementIdSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("achievementId", &val);
    achievement_id_ = val;
  }
  if (_achievementBitSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("achievementBit", &val);
    achievement_bit_ = val;
  }
  if (_miniMapVisibleSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("miniMapVisibility", &val);
    bits_.mini_map_visible_ = val != 0;
  }
  if (_bigMapVisibleSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("mapVisibility", &val);
    bits_.big_map_visible_ = val != 0;
  }
  if (_inGameVisibleSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("inGameVisibility", &val);
    bits_.in_game_visible_ = val != 0;
  }
  if (_scaleWithZoomSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("scaleOnMapWithZoom", &val);
    bits_.scale_with_zoom_ = val != 0;
  }
  if (_miniMapFadeOutLevelSaved) {
    n.GetAttributeAsFloat("mapFadeoutScaleLevel", &mini_map_fade_out_level_);
  }

  if (_miniMapSizeSaved) {
    int32_t x = 0;
    n.GetAttributeAsInteger("mapDisplaySize", &x);
    mini_map_size_ = x;
  }
  if (_keepOnMapEdgeSaved) {
    int32_t val = 0;
    n.GetAttributeAsInteger("keepOnMapEdge", &val);
    bits_.keep_on_map_edge_ = val != 0;
  }
  if (_infoSaved) info_ = AddStringToSet(n.GetAttributeAsString("info"));
  if (_infoRangeSaved) n.GetAttributeAsFloat("infoRange", &info_range_);
}

void MarkerTypeData::Write(CXMLNode* n) {
  if (bits_.icon_file_saved_) n->SetAttribute("iconFile", icon_file_);
  if (bits_.size_saved_) n->SetAttributeFromFloat("iconSize", size_);
  if (bits_.alpha_saved_) n->SetAttributeFromFloat("alpha", alpha_);
  if (bits_.fade_near_saved_) n->SetAttributeFromFloat("fadeNear", fade_near_);
  if (bits_.fade_far_saved_) n->SetAttributeFromFloat("fadeFar", fade_far_);
  if (bits_.height_saved_) n->SetAttributeFromFloat("heightOffset", height_);
  if (bits_.behavior_saved_) {
    n->SetAttributeFromInteger("behavior", static_cast<int32_t>(behavior_));
  }
  if (bits_.reset_length_saved_) {
    n->SetAttributeFromInteger("resetLength", reset_length_);
  }
  if (bits_.auto_trigger_saved_) {
    n->SetAttributeFromInteger("autoTrigger", bits_.auto_trigger_);
  }
  if (bits_.has_countdown_saved_) {
    n->SetAttributeFromInteger("hasCountdown", bits_.has_countdown_);
  }
  if (bits_.trigger_range_saved_) {
    n->SetAttributeFromFloat("triggerRange", trigger_range_);
  }
  if (bits_.min_size_saved_) n->SetAttributeFromInteger("minSize", min_size_);
  if (bits_.max_size_saved_) n->SetAttributeFromInteger("maxSize", max_size_);
  if (bits_.color_saved_) {
    n->SetAttribute("color", std::format("{:x}", color_.argb()));
  }
  if (bits_.trail_data_saved_) n->SetAttribute("trailData", trail_data_);
  if (bits_.anim_speed_saved_)
    n->SetAttributeFromFloat("animSpeed", anim_speed_);
  if (bits_.texture_saved_) n->SetAttribute("texture", texture_);
  if (bits_.trail_scale_saved_)
    n->SetAttributeFromFloat("trailScale", trail_scale_);
  if (bits_.toggle_category_saved_) {
    n->SetAttribute("toggleCategory", toggle_category_);
  }
  if (bits_.achievement_id_saved_) {
    n->SetAttributeFromInteger("achievementId", achievement_id_);
  }
  if (bits_.achievement_bit_saved_) {
    n->SetAttributeFromInteger("achievementBit", achievement_bit_);
  }
  if (bits_.mini_map_visible_saved_) {
    n->SetAttributeFromInteger("miniMapVisibility", bits_.mini_map_visible_);
  }
  if (bits_.big_map_visible_saved_) {
    n->SetAttributeFromInteger("mapVisibility", bits_.big_map_visible_);
  }
  if (bits_.in_game_visible_saved_) {
    n->SetAttributeFromInteger("inGameVisibility", bits_.in_game_visible_);
  }
  if (bits_.scale_with_zoom_saved_) {
    n->SetAttributeFromInteger("scaleOnMapWithZoom", bits_.scale_with_zoom_);
  }
  if (bits_.mini_map_fade_out_level_saved_) {
    n->SetAttributeFromFloat("mapFadeoutScaleLevel", mini_map_fade_out_level_);
  }
  if (bits_.mini_map_size_saved_) {
    n->SetAttributeFromInteger("mapDisplaySize", mini_map_size_);
  }
  if (bits_.keep_on_map_edge_saved_) {
    n->SetAttributeFromInteger("keepOnMapEdge", bits_.keep_on_map_edge_);
  }
  if (bits_.info_saved_) n->SetAttribute("info", info_);
  if (bits_.info_range_saved_)
    n->SetAttributeFromFloat("infoRange", info_range_);
}
