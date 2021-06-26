#pragma once
#include "Bedrock/BaseLib/BaseLib.h"
#include "GW2TacO.h"

void LoadConfig();
void SaveConfig();
void ToggleConfigValue( TCHAR *value );
void ToggleConfigValue( CString &value );
int32_t GetConfigValue( TCHAR *value );
void SetConfigValue( TCHAR *value, int32_t val );
TBOOL HasConfigValue( TCHAR *value );
TBOOL HasConfigString( TCHAR *value );
void SetConfigString( TCHAR *value, const CString& val );
CString GetConfigString( TCHAR *value );

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
void RemoveConfigEntry(TCHAR* value);
