﻿#include "GW2API.h"
#include "MumbleLink.h"
#include "OverlayConfig.h"
#include "Language.h"

#include <string>
#include <algorithm>

#include "Bedrock/UtilLib/jsonxx.h"
using namespace jsonxx;

std::string FetchHTTPS(std::string_view url,
                       std::string_view path);

std::string FetchAPIData(std::string_view path,
                     std::string_view apiKey) {
  bool hasquestionmark = false;

  if (std::find(path.begin(), path.end(), '?') != path.end()) {
    hasquestionmark = true;
  }

  std::string ask(path);
  if ( hasquestionmark )
    ask += "&access_token=";
  else
    ask += "?access_token=";
  ask += apiKey;

  return FetchHTTPS( "api.guildwars2.com", ask );
}

namespace GW2 {

APIKeyManager apiKeyManager;

APIKey::APIKey(std::string_view key) : apiKey(key) {
  }

  APIKey::~APIKey()
  {
    if ( fetcherThread.joinable() )
      fetcherThread.join();
  }

  void APIKey::FetchData()
  {
    if ( beingInitialized )
      return;

    valid = true;
    initialized = false;
    beingInitialized = true;

    fetcherThread = std::thread( [ this ]()
    {
      valid = true;

      auto keyData = QueryAPI( "/v2/tokeninfo" );

      Object json;
      json.parse( keyData );

      if ( json.has<String>( "name" ) )
        keyName = json.get<String>( "name" );
      else
        valid = false;

      if ( json.has<Array>( "permissions" ) )
      {
        auto& values = json.get<Array>( "permissions" ).values();
        for ( auto v : values )
        {
          if ( v->is<String>() )
            caps[ v->get<String>() ] = true;
        }
      }
      else
        valid = false;

      if ( HasCaps( "account" ) )
      {
        auto accountData = QueryAPI( "/v2/account" );
        json.parse( accountData );

        if ( json.has<String>( "name" ) )
          accountName = json.get<String>( "name" );

        if ( json.has<Number>( "world" ) )
          worldId = static_cast<int32_t>(json.get<Number>("world"));
      }

      initialized = true;
      beingInitialized = false;
    } );
  }

  bool APIKey::HasCaps(std::string_view cap)
  {
    if ( caps.find( std::string(cap) ) != caps.end() )
      return caps[ cap.data() ];

    return false;
  }

  std::string APIKey::QueryAPI(std::string_view path)
  {
    LOG_NFO( "[GW2TacO] Querying the API: %s", std::string(path).c_str() );

    return FetchAPIData( path, apiKey );
  }

  void APIKey::SetKey(std::string_view key)
  {
    if ( fetcherThread.joinable() )
      fetcherThread.join();
    apiKey = key;
    caps.clear();
    initialized = false;
    valid = true;
  }

  APIKey* APIKeyManager::GetIdentifiedAPIKey() {
    std::lock_guard<std::mutex> guard(keyMutex);
    if (mumbleLink.IsValid() || mumbleLink.charName.empty() || keys.empty())
      return nullptr;

    if (!initialized) {
      Initialize();
    }

    for (auto& key : keys) {
      if (!key->initialized) {
        continue;
      }

      if (key->fetcherThread.joinable()) {
        key->fetcherThread.join();
      }

      auto& cn = key->charNames;
      if (std::find(cn.begin(), cn.end(), mumbleLink.charName) != cn.end()) {
        return key.get();
      }
    }

    return nullptr;
  }

  APIKeyManager::Status APIKeyManager::GetStatus() {
    if (!initialized) {
      Initialize();
    }

    {
      std::lock_guard<std::mutex> guard(keyMutex);
      if (keys.empty()) {
        return Status::KeyNotSet;
      }
    }

    APIKey* key = GetIdentifiedAPIKey();
    if (!key) {
      std::lock_guard<std::mutex> guard(keyMutex);
      for (const auto& key : keys) {
        if (!key->initialized) {
          return Status::Loading;
        }
      }
      if (mumbleLink.charName.empty()) {
        return Status::WaitingForMumbleCharacterName;
      }
      return Status::CouldNotIdentifyAccount;
    }

    if (!key->initialized) {
      return Status::Loading;
    }

    return Status::OK;
  }

  APIKeyManager::Status APIKeyManager::DisplayStatusText(CWBDrawAPI* API, CWBFont* font)
  {
    APIKeyManager::Status status = GetStatus();

    switch (status) 
    {
    case Status::Loading:
      font->Write(API, DICT("waitingforapi"), CPoint(0, 0));
      break;
    case Status::KeyNotSet:
      font->Write(API, DICT("apikeynotset1"), CPoint(0, 0), CColor(0xff,0x40,0x40,0xff));
      font->Write(API, DICT("apikeynotset2"), CPoint(0, font->GetLineHeight()), CColor(0xff, 0x40, 0x40, 0xff));
      break;
    case Status::CouldNotIdentifyAccount:
      font->Write(API, DICT("couldntidentifyaccount1"), CPoint(0, 0), CColor(0xff, 0x40, 0x40, 0xff));
      font->Write(API, DICT("couldntidentifyaccount2"), CPoint(0, font->GetLineHeight()), CColor(0xff, 0x40, 0x40, 0xff));
      break;
    case Status::WaitingForMumbleCharacterName:
      font->Write(API, DICT("waitingforcharactername1"), CPoint(0, 0));
      break;
    case Status::AllKeysInvalid:
      font->Write(API, DICT("apierror1"), CPoint(0, 0), CColor(0xff, 0x40, 0x40, 0xff));
      font->Write(API, DICT("apierror2"), CPoint(0, font->GetLineHeight()), CColor(0xff, 0x40, 0x40, 0xff));
      break;
    }
    return status;
  }

  void APIKeyManager::Initialize()
  {
    if (initialized)
      return;

    if (HasConfigString("GW2APIKey"))
    {
      auto key = std::make_unique<APIKey>(GetConfigString("GW2APIKey"));
      RemoveConfigEntry("GW2APIKey");
      std::lock_guard<std::mutex> guard(keyMutex);
      keys.emplace_back(std::move(key));
    }

    int x = 0;
    while (true) 
    {
      std::string cfgName = "GW2APIKey" + std::to_string(x++);
      if (HasConfigString(cfgName))
      {
        auto key = std::make_unique<APIKey>(GetConfigString(cfgName));
        std::lock_guard<std::mutex> guard(keyMutex);
        keys.emplace_back(std::move(key));
      }
      else
        break;
    }

    RebuildConfigValues();

    std::lock_guard<std::mutex> guard(keyMutex);
    for (auto& key : keys) {
      key->FetchData();
    }

    initialized = true;
  }

  bool APIKeyManager::empty() {
    std::lock_guard<std::mutex> guard(keyMutex);
    return keys.empty();
  }

  APIKey* APIKeyManager::GetKey(int idx) {
    std::lock_guard<std::mutex> guard(keyMutex);
    return keys[idx].get();
  }

  void APIKeyManager::RemoveKey(int idx) {
    std::lock_guard<std::mutex> guard(keyMutex);
    keys.erase(keys.begin() + idx);
  }

  size_t APIKeyManager::size() { 
      std::lock_guard<std::mutex> guard(keyMutex);
    return keys.size();
  }

  void APIKeyManager::AddKey(std::unique_ptr<APIKey>&& key) {
    std::lock_guard<std::mutex> guard(keyMutex);
    keys.emplace_back(std::move(key));
  }

  void APIKeyManager::RebuildConfigValues() {
    int x = 0;
    while (true)
    {
      std::string cfgName = "GW2APIKey" + std::to_string(x++);
      if (HasConfigString(cfgName))
        RemoveConfigEntry(cfgName.c_str());
      else
        break;
    }

    std::lock_guard<std::mutex> guard(keyMutex);
    for (size_t x = 0; x < keys.size(); x++) {
      std::string cfgName = "GW2APIKey" + std::to_string(x);
      SetConfigString(cfgName, keys[x]->apiKey);
    }
    
    SaveConfig();
  }
}
