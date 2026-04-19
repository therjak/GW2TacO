module;
#include <string>
#include <string_view>
#include <unordered_map>

export module taco.overlay_config;

import math;
import taco.gw2taco;

export void LoadConfig();
export void SaveConfig();
export void InitConfig();
export void ToggleConfigValue(std::string_view value);
export int32_t GetConfigValue(std::string_view value);
export void SetConfigValue(std::string_view value, int32_t val);
export bool HasConfigValue(std::string_view value);
export bool HasConfigString(std::string_view value);
export void SetConfigString(std::string_view value, std::string_view val);
export std::string GetConfigString(std::string_view value);

export bool HasWindowData(std::string_view window_name);
export bool IsWindowOpen(std::string_view window_name);
export void SetWindowOpenState(std::string_view window_name, bool open);
export math::Rect GetWindowPosition(std::string_view window_name);
export void SetWindowPosition(std::string_view window_name,
                              math::Rect position);

export void GetKeyBindings(
    std::unordered_map<int32_t, TacOKeyAction>& key_bindings);
export void DeleteKeyBinding(int32_t key);
export void SetKeyBinding(TacOKeyAction action, int32_t key);

export void GetScriptKeyBindings(
    std::unordered_map<int32_t, std::string>& script_key_bindings);
export void SetScriptKeyBinding(std::string_view script_event, int32_t key);
export void DeleteScriptKeyBinding(std::string_view script_event);

export void LoadMarkerCategoryVisibilityInfo();

export void AutoSaveConfig();
export void RemoveConfigEntry(std::string_view value);
