module;

#include <cstdint>
#include <string_view>
#include <vector>

#include "base/color.h"

export module taco.marker_data;

import taco.poi_behavior;
import xml;

export struct MarkerTypeData {
  MarkerTypeData();
  void Read(const CXMLNode& node, bool store_save_state);
  void Write(CXMLNode* node);

  struct {
    bool needs_export_to_user_data_ : 1;
    bool icon_file_saved_ : 1;
    bool size_saved_ : 1;
    bool alpha_saved_ : 1;
    bool fade_near_saved_ : 1;
    bool fade_far_saved_ : 1;
    bool height_saved_ : 1;
    bool behavior_saved_ : 1;
    bool reset_length_saved_ : 1;
    bool auto_trigger_saved_ : 1;
    bool has_countdown_saved_ : 1;
    bool trigger_range_saved_ : 1;
    bool min_size_saved_ : 1;
    bool max_size_saved_ : 1;
    bool color_saved_ : 1;
    bool trail_data_saved_ : 1;
    bool anim_speed_saved_ : 1;
    bool texture_saved_ : 1;
    bool trail_scale_saved_ : 1;
    bool toggle_category_saved_ : 1;
    bool achievement_id_saved_ : 1;
    bool achievement_bit_saved_ : 1;
    bool auto_trigger_ : 1;
    bool has_countdown_ : 1;
    bool mini_map_visible_ : 1;
    bool big_map_visible_ : 1;
    bool in_game_visible_ : 1;
    bool mini_map_visible_saved_ : 1;
    bool big_map_visible_saved_ : 1;
    bool in_game_visible_saved_ : 1;
    bool scale_with_zoom_ : 1;
    bool scale_with_zoom_saved_ : 1;
    bool mini_map_size_saved_ : 1;
    bool mini_map_fade_out_level_saved_ : 1;
    bool keep_on_map_edge_ : 1;
    bool keep_on_map_edge_saved_ : 1;
    bool info_saved_ : 1;
    bool info_range_saved_ : 1;
  } bits_{};

  float size_ = 1.0;
  float alpha_ = 1.0f;
  float fade_near_ = -1;
  float fade_far_ = -1;
  float height_ = 1.5f;
  float trigger_range_ = 2.0f;
  float anim_speed_ = 1;
  float trail_scale_ = 1;
  int32_t mini_map_size_ = 20;
  float mini_map_fade_out_level_ = 100.0f;
  float info_range_ = 2.0f;

  POIBehavior behavior_ = POIBehavior::AlwaysVisible;
  CColor color_ = CColor(0xffffffff);

  int16_t reset_length_ = 0;
  int16_t min_size_ = 5;
  int16_t max_size_ = 2048;

  std::string_view icon_file_;
  std::string_view trail_data_;
  std::string_view texture_;
  std::string_view toggle_category_;
  int16_t achievement_id_ = -1;
  int16_t achievement_bit_ = -1;
  std::string_view info_;
};
