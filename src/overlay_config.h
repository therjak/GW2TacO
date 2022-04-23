#pragma once
#include <string>
#include <string_view>
#include <unordered_map>

#include "src/base/rectangle.h"
#include "src/gw2_taco.h"

void LoadConfig();
void SaveConfig();
void InitConfig();
void ToggleConfigValue(std::string_view value);
int32_t GetConfigValue(std::string_view value);
void SetConfigValue(std::string_view value, int32_t val);
bool HasConfigValue(std::string_view value);
bool HasConfigString(std::string_view value);
void SetConfigString(std::string_view value, std::string_view val);
std::string GetConfigString(std::string_view value);

bool HasWindowData(std::string_view windowname);
bool IsWindowOpen(std::string_view windowname);
void SetWindowOpenState(std::string_view windowname, bool Open);
math::CRect GetWindowPosition(std::string_view windowname);
void SetWindowPosition(std::string_view windowname, math::CRect Pos);

void GetKeyBindings(std::unordered_map<int32_t, TacOKeyAction>& KeyBindings);
void DeleteKeyBinding(int32_t key);
void SetKeyBinding(TacOKeyAction action, int32_t key);

void GetScriptKeyBindings(
    std::unordered_map<int32_t, std::string>& ScriptKeyBindings);
void SetScriptKeyBinding(std::string_view scriptEvent, int32_t key);
void DeleteScriptKeyBinding(std::string_view scriptEvent);

void LoadMarkerCategoryVisibilityInfo();

void AutoSaveConfig();
void RemoveConfigEntry(std::string_view value);
