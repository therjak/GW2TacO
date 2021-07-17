#pragma once
#include "Bedrock/BaseLib/BaseLib.h"
#include "GW2TacO.h"
#include <string_view>
#include <string>
#include <unordered_map>

void LoadConfig();
void SaveConfig();
void ToggleConfigValue(std::string_view value);
int32_t GetConfigValue(std::string_view value);
void SetConfigValue(std::string_view value, int32_t val);
TBOOL HasConfigValue(std::string_view value);
TBOOL HasConfigString(std::string_view value);
void SetConfigString(std::string_view value,
                     std::string_view val);
std::string GetConfigString(std::string_view value);

TBOOL HasWindowData( std::string_view windowname );
TBOOL IsWindowOpen( std::string_view windowname );
void SetWindowOpenState( std::string_view windowname, TBOOL Open );
CRect GetWindowPosition( std::string_view windowname );
void SetWindowPosition( std::string_view windowname, CRect Pos );

void GetKeyBindings( std::unordered_map<int32_t, TacOKeyAction> &KeyBindings );
void DeleteKeyBinding( int32_t key );
void SetKeyBinding( TacOKeyAction action, int32_t key );

void GetScriptKeyBindings( std::unordered_map<int32_t, std::string> &ScriptKeyBindings );
void SetScriptKeyBinding( std::string_view scriptEvent, int32_t key );
void DeleteScriptKeyBinding( std::string_view scriptEvent );

void LoadMarkerCategoryVisibilityInfo();

void AutoSaveConfig();
void RemoveConfigEntry(std::string_view value);
