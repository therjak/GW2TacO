module;

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

export module taco.tactical_category;

import taco.marker_data;

export class GW2TacticalCategory {
 public:
  virtual ~GW2TacticalCategory() = default;

  std::string GetFullTypeName();
  [[nodiscard]] bool IsVisible() const;
  void CacheVisibility();
  void CalculateVisibilityCache();

  std::string name;
  std::string display_name;

  std::string_view zip_file;

  MarkerTypeData data;
  bool keep_save_state = false;
  bool is_only_separator = false;
  GW2TacticalCategory* parent = nullptr;
  std::vector<std::unique_ptr<GW2TacticalCategory>> children;

  bool is_displayed = true;
  bool cached_visibility_ = true;
  static bool visibility_cached_;

 private:
  std::string cached_type_name_;
};

export extern GW2TacticalCategory CategoryRoot;
export extern std::unordered_map<std::string, GW2TacticalCategory*> CategoryMap;
export GW2TacticalCategory* GetCategory(std::string_view sv);
