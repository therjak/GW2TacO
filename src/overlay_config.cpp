#include "src/overlay_config.h"

#include <format>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>

#include "src/base/logger.h"
#include "src/base/timer.h"
#include "src/util/xml_document.h"

using math::CPoint;
using math::CRect;

// These maps must be sorted. So no unordered_map.
std::map<std::string, int32_t> ConfigNums;
std::map<std::string, std::string> ConfigStrings;
bool configChanged = false;
auto lastConfigChangeTime = globalTimer.GetTime();

void LoadConfig() {
  CXMLDocument d;
  if (!d.LoadFromFile("TacOConfig.xml")) {
    SetConfigValue("EditMode", 0);
    SetConfigValue("InterfaceSize", 1);
    SetConfigValue("CloseWithGW2", 1);
    SetWindowOpenState("MapTimer", true);
    SetWindowPosition("MapTimer", CRect(6, 97, 491, 813));
    return;
  }
  ConfigNums.clear();
  ConfigStrings.clear();

  if (!d.GetDocumentNode().GetChildCount("TacOConfig")) return;
  CXMLNode root = d.GetDocumentNode().GetChild("TacOConfig");

  for (int32_t x = 0; x < root.GetChildCount(); x++) {
    auto item = root.GetChild(x);
    if (item.HasAttribute("Data")) {
      int32_t data = 0;
      item.GetAttributeAsInteger("Data", &data);
      ConfigNums[item.GetNodeName()] = data;
    }

    if (item.HasAttribute("String")) {
      ConfigStrings[item.GetNodeName()] = item.GetAttributeAsString("String");
    }
  }

  configChanged = false;
}

void SaveConfig() {
  CXMLDocument doc;
  CXMLNode root = doc.GetDocumentNode();
  auto& tacoConfig = root.AddChild("TacOConfig");
  for (const auto& kdp : ConfigNums) {
    tacoConfig.AddChild(kdp.first).SetAttributeFromInteger("Data", kdp.second);
  }
  for (const auto& kdp : ConfigStrings) {
    tacoConfig.AddChild(kdp.first).SetAttribute("String", kdp.second);
  }
  doc.SaveToFile("TacOConfig.xml");
}

void ToggleConfigValue(std::string_view key) {
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  std::string k(key);
  if (ConfigNums.find(k) != ConfigNums.end()) {
    int32_t v = ConfigNums[k];
    ConfigNums[k] = !v;
  } else {
    ConfigNums[k] = 0;
  }
}

int32_t GetConfigValue(std::string_view value) {
  std::string v(value);
  if (ConfigNums.find(v) != ConfigNums.end()) {
    return ConfigNums[v];
  }
  return 0;
}

void SetConfigValue(std::string_view value, int32_t val) {
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  ConfigNums[std::string(value)] = val;
}

bool HasConfigValue(std::string_view value) {
  return ConfigNums.find(std::string(value)) != ConfigNums.end();
}

bool HasConfigString(std::string_view value) {
  return ConfigStrings.find(std::string(value)) != ConfigStrings.end();
}

void SetConfigString(std::string_view value, std::string_view val) {
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  ConfigStrings[std::string(value)] = val;
}

std::string GetConfigString(std::string_view value) {
  std::string v(value);
  auto f = ConfigStrings.find(v);
  if (f != ConfigStrings.end()) return f->second;
  return "";
}

bool IsWindowOpen(std::string_view windowname) {
  std::string s(windowname);
  return GetConfigValue((s + "_open"));
}

void SetWindowOpenState(std::string_view windowname, bool Open) {
  std::string s(windowname);
  SetConfigValue((s + "_open"), static_cast<int>(Open));
}

CRect GetWindowPosition(std::string_view windowname) {
  CRect r;
  std::string s(windowname);
  r.x1 = GetConfigValue((s + "_x1"));
  r.y1 = GetConfigValue((s + "_y1"));
  r.x2 = GetConfigValue((s + "_x2"));
  r.y2 = GetConfigValue((s + "_y2"));
  return r;
}

void SetWindowPosition(std::string_view windowname, CRect Pos) {
  std::string s(windowname);
  SetConfigValue((s + "_x1"), Pos.x1);
  SetConfigValue((s + "_y1"), Pos.y1);
  SetConfigValue((s + "_x2"), Pos.x2);
  SetConfigValue((s + "_y2"), Pos.y2);
}

bool HasWindowData(std::string_view windowname) {
  std::string s(windowname);
  return HasConfigValue((s + "_open")) && HasConfigValue((s + "_x1")) &&
         HasConfigValue((s + "_y1")) && HasConfigValue((s + "_x2")) &&
         HasConfigValue((s + "_y2"));
}

void GetKeyBindings(std::unordered_map<int32_t, TacOKeyAction>& KeyBindings) {
  KeyBindings.clear();

  for (const auto& kdp : ConfigNums) {
    if (kdp.first.find("KeyboardKey_") != 0) continue;
    int32_t key = 0;
    if (std::sscanf(kdp.first.c_str(), "KeyboardKey_%d", &key) != 1) continue;
    if (kdp.second == static_cast<int32_t>(TacOKeyAction::NoAction)) continue;
    KeyBindings[key] = static_cast<TacOKeyAction>(kdp.second);
  }

  if (KeyBindings.empty()) {
    SetKeyBinding(TacOKeyAction::AddPOI, '+');
    SetKeyBinding(TacOKeyAction::RemovePOI, '-');
    SetKeyBinding(TacOKeyAction::ActivatePOI, 'F');
    SetKeyBinding(TacOKeyAction::ActivatePOI, 'f');
    SetKeyBinding(TacOKeyAction::EditNotepad, ']');

    KeyBindings['+'] = TacOKeyAction::AddPOI;
    KeyBindings['-'] = TacOKeyAction::RemovePOI;
    KeyBindings['F'] = TacOKeyAction::ActivatePOI;
    KeyBindings['f'] = TacOKeyAction::ActivatePOI;
    KeyBindings[']'] = TacOKeyAction::EditNotepad;
  }
}

void SetKeyBinding(TacOKeyAction action, int32_t key) {
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  ConfigNums[std::format("KeyboardKey_{:d}", key)] =
      static_cast<int32_t>(action);
}

void DeleteKeyBinding(int32_t keyToDelete) {
  auto elm = ConfigNums.begin();
  while (elm != ConfigNums.end()) {
    if (elm->first.find("KeyboardKey_") != 0) {
      ++elm;
      continue;
    }
    int32_t key = 0;
    if (std::sscanf(elm->first.c_str(), "KeyboardKey_%d", &key) != 1) {
      ++elm;
      continue;
    }

    if (key == keyToDelete) {
      elm = ConfigNums.erase(elm);
    } else {
      ++elm;
    }
  }
}

void GetScriptKeyBindings(
    std::unordered_map<int32_t, std::string>& ScriptKeyBindings) {
  ScriptKeyBindings.clear();

  for (const auto& kdp : ConfigNums) {
    if (kdp.first.find("ScriptKey_") != 0) continue;
    auto key = kdp.first.substr(10);
    if (key.empty()) continue;
    ScriptKeyBindings[kdp.second] = key;
  }
}

void SetScriptKeyBinding(std::string_view scriptEvent, int32_t key) {
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  ConfigNums[std::format("ScriptKey_{:s}", scriptEvent)] = key;
}

void DeleteScriptKeyBinding(std::string_view scriptEvent) {
  auto it = ConfigNums.begin();
  while (it != ConfigNums.end()) {
    if (it->first.find("ScriptKey_") != 0) {
      ++it;
      continue;
    }
    auto key = it->first.substr(10);
    if (key != scriptEvent) {
      ++it;
      continue;
    }
    it = ConfigNums.erase(it);
  }
}

GW2TacticalCategory* GetCategory(std::string_view s);

void LoadMarkerCategoryVisibilityInfo() {
  for (auto& kdp : ConfigNums) {
    if (kdp.first.find("CategoryVisible_") != 0) continue;

    auto str = kdp.first.substr(16);
    auto cat = GetCategory(str);
    if (cat) {
      cat->IsDisplayed = kdp.second != 0;
    }
  }
  CategoryRoot.CalculateVisibilityCache();
}

void AutoSaveConfig() {
  if (!configChanged) return;

  if (globalTimer.GetTime() - lastConfigChangeTime > 10000) {
    configChanged = false;
    SaveConfig();
  }
}

void RemoveConfigEntry(std::string_view name) {
  ConfigStrings.erase(std::string(name));
  ConfigNums.erase(std::string(name));
}

void InitConfig() {
  if (!HasConfigValue("CheckForUpdates")) {
    SetConfigValue("CheckForUpdates", 1);
  }

  if (!HasConfigValue("HideOnLoadingScreens")) {
    SetConfigValue("HideOnLoadingScreens", 1);
  }

  if (!HasConfigValue("KeybindsEnabled")) {
    SetConfigValue("KeybindsEnabled", 1);
  }

  if (!HasConfigValue("Vsync")) {
    SetConfigValue("Vsync", 1);
  }

  if (!HasConfigValue("SmoothCharacterPos")) {
    SetConfigValue("SmoothCharacterPos", 1);
  }

  if (!HasConfigValue("FrameThrottling")) {
    SetConfigValue("FrameThrottling", 1);
  }

  if (!HasConfigValue("EnableTPNotificationIcon")) {
    SetConfigValue("EnableTPNotificationIcon", 1);
  }

  if (!HasConfigValue("LogTrails")) {
    SetConfigValue("LogTrails", 0);
  }

  if (!HasConfigValue("CloseWithGW2")) {
    SetConfigValue("CloseWithGW2", 1);
  }

  if (!HasConfigValue("InfoLineVisible")) {
    SetConfigValue("InfoLineVisible", 0);
  }

  if (!HasConfigValue("TacticalIconsOnEdge")) {
    SetConfigValue("TacticalIconsOnEdge", 1);
  }

  if (!HasConfigValue("TacticalLayerVisible")) {
    SetConfigValue("TacticalLayerVisible", 1);
  }

  if (!HasConfigValue("DrawWvWNames")) {
    SetConfigValue("DrawWvWNames", 1);
  }

  if (!HasConfigValue("TacticalDrawDistance")) {
    SetConfigValue("TacticalDrawDistance", 0);
  }

  if (!HasConfigValue("UseMetricDisplay")) {
    SetConfigValue("UseMetricDisplay", 0);
  }

  if (!HasConfigValue("OpacityIngame")) {
    SetConfigValue("OpacityIngame", 0);
  }

  if (!HasConfigValue("OpacityMap")) {
    SetConfigValue("OpacityMap", 0);
  }

  if (!HasConfigValue("TacticalInfoTextVisible")) {
    SetConfigValue("TacticalInfoTextVisible", 1);
  }

  if (!HasConfigValue("ShowMinimapMarkers")) {
    SetConfigValue("ShowMinimapMarkers", 1);
  }

  if (!HasConfigValue("ShowBigmapMarkers")) {
    SetConfigValue("ShowBigmapMarkers", 1);
  }

  if (!HasConfigValue("ShowInGameMarkers")) {
    SetConfigValue("ShowInGameMarkers", 1);
  }

  if (!HasConfigValue("HPGridVisible")) {
    SetConfigValue("HPGridVisible", 1);
  }

  if (!HasConfigValue("LocationalTimersVisible")) {
    SetConfigValue("LocationalTimersVisible", 1);
  }

  if (!HasConfigValue("MapTimerVisible")) {
    SetConfigValue("MapTimerVisible", 1);
  }

  if (!HasConfigValue("MapTimerCompact")) {
    SetConfigValue("MapTimerCompact", 1);
  }

  if (!HasConfigValue("MapTimerCategories")) {
    SetConfigValue("MapTimerCategories", 1);
  }

  if (!HasConfigValue("MapTimerCategoryLineWidth")) {
    SetConfigValue("MapTimerCategoryLineWidth", 3);
  }

  if (!HasConfigValue("AutoHideMarkerEditor")) {
    SetConfigValue("AutoHideMarkerEditor", 1);
  }
  if (!HasConfigValue("TacticalLayerVisible")) {
    SetConfigValue("TacticalLayerVisible", 1);
  }
  if (!HasConfigValue("MouseHighlightVisible")) {
    SetConfigValue("MouseHighlightVisible", 0);
  }

  if (!HasConfigValue("MouseHighlightColor")) {
    SetConfigValue("MouseHighlightColor", 0);
  }

  if (!HasConfigValue("MouseHighlightOutline")) {
    SetConfigValue("MouseHighlightOutline", 0);
  }

  if (!HasConfigValue("CompactRaidWindow")) {
    SetConfigValue("CompactRaidWindow", 0);
  }

  if (!HasConfigValue("RangeCirclesVisible")) {
    SetConfigValue("RangeCirclesVisible", 0);
  }

  if (!HasConfigValue("RangeCircleTransparency")) {
    SetConfigValue("RangeCircleTransparency", 100);
  }

  if (!HasConfigValue("RangeCircle90")) {
    SetConfigValue("RangeCircle90", 0);
  }

  if (!HasConfigValue("RangeCircle120")) {
    SetConfigValue("RangeCircle120", 1);
  }

  if (!HasConfigValue("RangeCircle180")) {
    SetConfigValue("RangeCircle180", 0);
  }

  if (!HasConfigValue("RangeCircle240")) {
    SetConfigValue("RangeCircle240", 0);
  }

  if (!HasConfigValue("RangeCircle300")) {
    SetConfigValue("RangeCircle300", 1);
  }

  if (!HasConfigValue("RangeCircle400")) {
    SetConfigValue("RangeCircle400", 1);
  }

  if (!HasConfigValue("RangeCircle600")) {
    SetConfigValue("RangeCircle600", 1);
  }

  if (!HasConfigValue("RangeCircle900")) {
    SetConfigValue("RangeCircle900", 1);
  }

  if (!HasConfigValue("RangeCircle1200")) {
    SetConfigValue("RangeCircle1200", 1);
  }

  if (!HasConfigValue("RangeCircle1500")) {
    SetConfigValue("RangeCircle1500", 0);
  }

  if (!HasConfigValue("RangeCircle1600")) {
    SetConfigValue("RangeCircle1600", 0);
  }

  if (!HasConfigValue("TacticalCompassVisible")) {
    SetConfigValue("TacticalCompassVisible", 0);
  }
}
