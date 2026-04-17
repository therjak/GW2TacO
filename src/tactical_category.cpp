module;

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

module taco.tactical_category;

GW2TacticalCategory CategoryRoot;
std::unordered_map<std::string, GW2TacticalCategory*> CategoryMap;

std::string GW2TacticalCategory::GetFullTypeName() {
  if (!cached_type_name_.empty()) return cached_type_name_;

  if (!parent) return "";
  if (parent == &CategoryRoot) {
    std::string n = name;
    std::transform(n.begin(), n.end(), n.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return n;
  }
  std::string pname = parent->GetFullTypeName();
  std::string s = pname + "." + name;
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  cached_type_name_ = s;
  return s;
}

bool GW2TacticalCategory::IsVisible() const {
  if (visibility_cached_) {
    return cached_visibility_;
  }
  if (!parent) {
    return is_displayed;
  }
  return is_displayed && parent->IsVisible();
}

void GW2TacticalCategory::CacheVisibility() {
  cached_visibility_ = IsVisible();
  for (auto& c : children) {
    c->CacheVisibility();
  }
}

bool GW2TacticalCategory::visibility_cached_ = false;

void GW2TacticalCategory::CalculateVisibilityCache() {
  visibility_cached_ = false;
  CacheVisibility();
  visibility_cached_ = true;
}

GW2TacticalCategory* GetCategory(std::string_view sv) {
  std::string s(sv);
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  const auto& fc = CategoryMap.find(s);
  if (fc != CategoryMap.end()) return fc->second;
  return nullptr;
}
