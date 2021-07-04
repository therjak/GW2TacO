#pragma once
#include "Bedrock/BaseLib/BaseLib.h"
#include "GW2TacO.h"
#include <string_view>
#include <string>

void LoadConfig();
void SaveConfig();
void ToggleConfigValue(std::string_view value);
void ToggleConfigValue( CString &value );
int32_t GetConfigValue(std::string_view value);
void SetConfigValue(std::string_view value, int32_t val);
TBOOL HasConfigValue(std::string_view value);
TBOOL HasConfigString(std::string_view value);
void SetConfigString(std::string_view value,
                     std::string_view val);
CString GetConfigString(std::string_view value);

TBOOL HasWindowData( TCHAR *windowname );
TBOOL IsWindowOpen( TCHAR *windowname );
void SetWindowOpenState( TCHAR *windowname, TBOOL Open );
CRect GetWindowPosition( TCHAR *windowname );
void SetWindowPosition( TCHAR *windowname, CRect Pos );

void GetKeyBindings( CDictionary<int32_t, TacOKeyAction> &KeyBindings );
void DeleteKeyBinding( int32_t key );
void SetKeyBinding( TacOKeyAction action, int32_t key );

void GetScriptKeyBindings( CDictionary<int32_t, CString> &ScriptKeyBindings );
void SetScriptKeyBinding( const CString& scriptEvent, int32_t key );
void DeleteScriptKeyBinding( const CString& scriptEvent );

void LoadMarkerCategoryVisibilityInfo();

void AutoSaveConfig();
void RemoveConfigEntry(std::string_view value);
