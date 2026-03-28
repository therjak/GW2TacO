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
  std::string displayName;

  std::string_view zipFile;

  MarkerTypeData data;
  bool KeepSaveState = false;
  bool IsOnlySeparator = false;
  GW2TacticalCategory* Parent = nullptr;
  std::vector<std::unique_ptr<GW2TacticalCategory>> children;

  bool IsDisplayed = true;
  bool cachedVisibility = true;
  static bool visibilityCached;

 private:
  std::string cachedTypeName;
};

export extern GW2TacticalCategory CategoryRoot;
export extern std::unordered_map<std::string, GW2TacticalCategory*> CategoryMap;
export GW2TacticalCategory* GetCategory(std::string_view sv);
