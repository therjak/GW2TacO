#include "OverlayConfig.h"
#include "Bedrock/UtilLib/XMLDocument.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <map>

#include "Bedrock/BaseLib/string_format.h"

// These maps must be sorted. So no unordered_map.
std::map<std::string, int32_t> ConfigNums;
std::map<std::string, std::string> ConfigStrings;
bool configChanged = false;
auto lastConfigChangeTime = globalTimer.GetTime();

void LoadConfig()
{
  FORCEDDEBUGLOG( "Loading config." );
  CXMLDocument d;
  if ( !d.LoadFromFile( "TacOConfig.xml" ) )
  {
    FORCEDDEBUGLOG( "Config failed to load, setting defaults." );
    SetConfigValue( "EditMode", 0 );
    SetConfigValue( "InterfaceSize", 1 );
    SetConfigValue( "CloseWithGW2", 1 );
    SetWindowOpenState( "MapTimer", true );
    SetWindowPosition( "MapTimer", CRect( 6, 97, 491, 813 ) );
    return;
  }
  ConfigNums.clear();
  ConfigStrings.clear();
  FORCEDDEBUGLOG( "Config flushed." );

  if ( !d.GetDocumentNode().GetChildCount( "TacOConfig" ) ) return;
  CXMLNode root = d.GetDocumentNode().GetChild( "TacOConfig" );

  FORCEDDEBUGLOG( "Config root found." );

  for ( int32_t x = 0; x < root.GetChildCount(); x++ )
  {
    FORCEDDEBUGLOG( "Loading config value %d/%d.", x, root.GetChildCount() );

    auto item = root.GetChild( x );
    if ( item.HasAttribute( "Data" ) )
    {
      int32_t data = 0;
      item.GetAttributeAsInteger( "Data", &data );
      ConfigNums[ item.GetNodeName() ] = data;
    }

    if ( item.HasAttribute( "String" ) )
    {
      ConfigStrings[ item.GetNodeName() ] = item.GetAttributeAsString( "String" );
    }
  }

  configChanged = false;
}

void SaveConfig()
{
  CXMLDocument doc;
  CXMLNode& root = doc.GetDocumentNode();
  root = root.AddChild( "TacOConfig" );
  for ( const auto& kdp: ConfigNums )
  {
    root.AddChild( kdp.first ).SetAttributeFromInteger( "Data", kdp.second );
  }
  for ( const auto& kdp: ConfigStrings )
  {
    root.AddChild( kdp.first ).SetAttribute( "String", kdp.second );
  }
  doc.SaveToFile( "TacOConfig.xml" );
}

void ToggleConfigValue(std::string_view key) {
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  std::string k(key);
  if ( ConfigNums.find( k ) != ConfigNums.end() )
  {
    int32_t v = ConfigNums[ k ];
    ConfigNums[ k ] = !v;
  }
  else
    ConfigNums[ k ] = 0;
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
  ConfigNums[ value.data() ] = val;
}

TBOOL HasConfigValue(std::string_view value) {
  return ConfigNums.find(std::string(value)) != ConfigNums.end();
}

TBOOL HasConfigString(std::string_view value) {
  return ConfigStrings.find(std::string(value)) != ConfigStrings.end();
}

void SetConfigString(std::string_view value,
                     std::string_view val) {
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  ConfigStrings[ std::string(value) ] = val;
}

std::string GetConfigString(std::string_view value) {
  std::string v(value);
  auto f = ConfigStrings.find(v);
  if ( f != ConfigStrings.end() )
    return f->second;
  return "";
}

TBOOL IsWindowOpen(std::string_view windowname) {
  std::string s( windowname );
  return GetConfigValue( ( s + "_open" ) );
}

void SetWindowOpenState(std::string_view windowname, TBOOL Open) {
  std::string s(windowname);
  SetConfigValue((s + "_open"), (int)Open);
}

CRect GetWindowPosition(std::string_view windowname) {
  CRect r;
  std::string s(windowname);
  r.x1 = GetConfigValue( ( s + "_x1" ) );
  r.y1 = GetConfigValue( ( s + "_y1" ) );
  r.x2 = GetConfigValue( ( s + "_x2" ) );
  r.y2 = GetConfigValue( ( s + "_y2" ) );
  return r;
}

void SetWindowPosition(std::string_view windowname, CRect Pos) {
  std::string s(windowname);
  SetConfigValue( ( s + "_x1" ), Pos.x1 );
  SetConfigValue( ( s + "_y1" ), Pos.y1 );
  SetConfigValue( ( s + "_x2" ), Pos.x2 );
  SetConfigValue( ( s + "_y2" ), Pos.y2 );
}

TBOOL HasWindowData(std::string_view windowname) {
  std::string s(windowname);
  return HasConfigValue( ( s + "_open" ) ) &&
    HasConfigValue( ( s + "_x1" ) ) &&
    HasConfigValue( ( s + "_y1" ) ) &&
    HasConfigValue( ( s + "_x2" ) ) &&
    HasConfigValue( ( s + "_y2" ) );
}

void GetKeyBindings( std::unordered_map<int32_t, TacOKeyAction> &KeyBindings )
{
  KeyBindings.clear();

  for ( const auto& kdp: ConfigNums )
  {
    if ( kdp.first.find( "KeyboardKey_" ) != 0 )
      continue;
    int32_t key;
    if ( std::sscanf(kdp.first.c_str(), "KeyboardKey_%d", &key ) != 1 )
      continue;
    if ( kdp.second == (int32_t)TacOKeyAction::NoAction )
      continue;
    KeyBindings[ key ] = (TacOKeyAction)kdp.second;
  }

  if ( KeyBindings.empty() )
  {
    SetKeyBinding( TacOKeyAction::AddPOI, '+' );
    SetKeyBinding( TacOKeyAction::RemovePOI, '-' );
    SetKeyBinding( TacOKeyAction::ActivatePOI, 'F' );
    SetKeyBinding( TacOKeyAction::ActivatePOI, 'f' );
    SetKeyBinding( TacOKeyAction::EditNotepad, ']' );

    KeyBindings[ '+' ] = TacOKeyAction::AddPOI;
    KeyBindings[ '-' ] = TacOKeyAction::RemovePOI;
    KeyBindings[ 'F' ] = TacOKeyAction::ActivatePOI;
    KeyBindings[ 'f' ] = TacOKeyAction::ActivatePOI;
    KeyBindings[ ']' ] = TacOKeyAction::EditNotepad;
  }
}

void SetKeyBinding( TacOKeyAction action, int32_t key )
{
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  ConfigNums[ FormatString( "KeyboardKey_%d", key ) ] = (int32_t)action;
}

void DeleteKeyBinding( int32_t keyToDelete )
{
  auto elm = ConfigNums.begin();
  while ( elm != ConfigNums.end() )
  {
    if (elm->first.find("KeyboardKey_") != 0) {
      ++elm;
      continue;
    }
    int32_t key;
    if (std::sscanf(elm->first.c_str(), "KeyboardKey_%d", &key) != 1) {
      ++elm;
      continue;
    }

    if ( key == keyToDelete ) {
      elm = ConfigNums.erase( elm );
    } else {
      ++elm;
    }
  }
}

void GetScriptKeyBindings( std::unordered_map<int32_t, std::string> &ScriptKeyBindings )
{
  ScriptKeyBindings.clear();

  for ( const auto& kdp: ConfigNums )
  {
    if ( kdp.first.find( "ScriptKey_" ) != 0 )
      continue;
    auto key = kdp.first.substr( 10 );
    if ( key.empty() )
      continue;
    ScriptKeyBindings[ kdp.second ] = key;
  }
}

void SetScriptKeyBinding( std::string_view scriptEvent, int32_t key )
{
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  ConfigNums[ FormatString( "ScriptKey_%s", std::string(scriptEvent).c_str()) ] = (int32_t)key;
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

GW2TacticalCategory *GetCategory( std::string_view s );

void LoadMarkerCategoryVisibilityInfo()
{
  for ( auto& kdp: ConfigNums )
  {
    if ( kdp.first.find( "CategoryVisible_" ) != 0 )
      continue;

    auto str = kdp.first.substr( 16 );
    auto cat = GetCategory( str );
    if ( cat )
      cat->IsDisplayed = kdp.second != 0;
  }
}

void AutoSaveConfig()
{
  if ( !configChanged )
    return;

  if ( globalTimer.GetTime() - lastConfigChangeTime > 10000 )
  {
    configChanged = false;
    SaveConfig();
  }
}

void RemoveConfigEntry(std::string_view name) {
  ConfigStrings.erase(std::string(name));
  ConfigNums.erase(std::string(name));
}
