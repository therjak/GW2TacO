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
  if (!cachedTypeName.empty()) return cachedTypeName;

  if (!Parent) return "";
  if (Parent == &CategoryRoot) {
    std::string n = name;
    std::transform(n.begin(), n.end(), n.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return n;
  }
  std::string pname = Parent->GetFullTypeName();
  std::string s = pname + "." + name;
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  cachedTypeName = s;
  return s;
}

bool GW2TacticalCategory::IsVisible() const {
  if (visibilityCached) {
    return cachedVisibility;
  }
  if (!Parent) {
    return IsDisplayed;
  }
  return IsDisplayed && Parent->IsVisible();
}

void GW2TacticalCategory::CacheVisibility() {
  cachedVisibility = IsVisible();
  for (auto& c : children) {
    c->CacheVisibility();
  }
}

bool GW2TacticalCategory::visibilityCached = false;

void GW2TacticalCategory::CalculateVisibilityCache() {
  visibilityCached = false;
  CacheVisibility();
  visibilityCached = true;
}

GW2TacticalCategory* GetCategory(std::string_view sv) {
  std::string s(sv);
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  const auto& fc = CategoryMap.find(s);
  if (fc != CategoryMap.end()) return fc->second;
  return nullptr;
}
