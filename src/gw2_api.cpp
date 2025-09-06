#include "src/gw2_api.h"

#include <algorithm>
#include <string>

#include "src/base/logger.h"
#include "src/language.h"
#include "src/mumble_link.h"
#include "src/overlay_config.h"
#include "src/util/jsonxx.h"
using namespace jsonxx;

using math::CPoint;

std::string FetchHTTPS(std::string_view url, std::string_view path);

std::string FetchAPIData(std::string_view path, std::string_view apiKey) {
  bool hasquestionmark = false;

  if (std::find(path.begin(), path.end(), '?') != path.end()) {
    hasquestionmark = true;
  }

  std::string ask(path);
  if (hasquestionmark) {
    ask += "&access_token=";
  } else {
    ask += "?access_token=";
  }
  ask += apiKey;

  return FetchHTTPS("api.guildwars2.com", ask);
}

namespace GW2 {

APIKeyManager apiKeyManager;

APIKey::APIKey(std::string_view key) : apiKey(key) {}

APIKey::~APIKey() {
  if (fetcherThread.joinable()) {
    fetcherThread.join();
  }
}

void APIKey::FetchData() {
  if (beingInitialized) return;
  beingInitialized = true;

  // keyName = "";
  // accountName = "";
  // charNames.clear();
  // caps.clear();
  // worldId = 0;

  initialized = false;

  fetcherThread = std::thread([this]() {
    KeyData new_key_data;

    auto keyData = QueryAPI("/v2/tokeninfo");

    Object json;
    json.parse(keyData);

    if (json.has<String>("name")) {
      new_key_data.key_name = json.get<String>("name");
    } else {
      new_key_data.valid = false;
    }

    if (json.has<Array>("permissions")) {
      auto& values = json.get<Array>("permissions").values();
      for (auto v : values) {
        if (v->is<String>()) new_key_data.caps.insert(v->get<String>());
      }
    } else {
      new_key_data.valid = false;
    }

    if (new_key_data.caps.contains("account")) {
      auto accountData = QueryAPI("/v2/account");
      json.parse(accountData);

      if (json.has<String>("name")) {
        new_key_data.account_name = json.get<String>("name");
      }

      if (json.has<Number>("world")) {
        new_key_data.world_id = static_cast<int32_t>(json.get<Number>("world"));
      }
    }
    if (new_key_data.caps.contains("characters")) {
      auto characterData =
          "{\"characters\":" + QueryAPI("/v2/characters") + "}";
      json.parse(characterData);
      if (!json.has<Array>("characters")) {
        Log_Err("[GW2TacO] Unexpected result from API characters endpoint: %s",
                characterData);
        Log_Err("[GW2TacO] CHARACTERS WON'T BE RECOGNIZED FOR API KEY NAMED %s",
                new_key_data.key_name);
      } else {
        auto m = json.get<Array>("characters").values();
        for (auto& x : m) {
          if (x->is<String>()) {
            auto name = x->get<String>();
            new_key_data.char_names.emplace_back(name);
          }
        }
      }
    } else {
      Log_Err(
          "[GW2TacO] API error: API key '%s - %s (%s)' doesn't have the "
          "'characters' permission - account identification through Mumble "
          "Link will not be possible.",
          new_key_data.account_name, new_key_data.key_name, apiKey);
    }
    key_data_queue.push(new_key_data);

    initialized = true;
    beingInitialized = false;
  });
}

bool APIKey::HasCaps(std::string_view cap) {
  return key_data.caps.contains(std::string(cap));
}

std::string APIKey::QueryAPI(std::string_view path) const {
  Log_Nfo("[GW2TacO] Querying the API: {:s}", path);

  return FetchAPIData(path, apiKey);
}

void APIKey::SetKey(std::string_view key) {
  if (fetcherThread.joinable()) {
    fetcherThread.join();
  }
  apiKey = key;
  initialized = false;
  beingInitialized = false;
  FetchData();
}

bool APIKey::Valid() {
  if (!initialized) {
    return false;
  }
  const auto& new_data = key_data_queue.pop();
  if (new_data.has_value()) {
    auto data = new_data.value();
    std::swap(key_data, data);
  }
  return key_data.valid;
}

std::unordered_set<std::string> APIKey::QuerySet(std::string_view path) const {
  const auto q = QueryAPI(path);
  Array json;
  json.parse(q);
  std::unordered_set<std::string> ret;
  for (auto& x : json.values()) {
    if (!x->is<String>()) continue;
    ret.emplace(x->get<String>());
  }
  return ret;
}

std::unordered_set<int32_t> APIKey::QueryAchievementBits(int id) const {
  const auto q = QueryAPI("/v2/account/achievements?ids=" + std::to_string(id));
  Array json;
  json.parse(q);
  std::unordered_set<int32_t> ret;
  const auto& data = json.values();
  if (!data.empty() && data[0]->is<Object>()) {
    Object obj = data[0]->get<Object>();
    if (obj.has<Array>("bits")) {
      auto bits = obj.get<Array>("bits").values();
      if (bits.size() > 0) {
        for (auto& bit : bits) {
          if (bit->is<Number>()) {
            ret.emplace(static_cast<int32_t>(bit->get<Number>()));
          }
        }
      }
    }
  }
  return ret;
}

APIKey* APIKeyManager::GetIdentifiedAPIKey() {
  std::scoped_lock l(keyMutex);
  if (!mumbleLink.IsValid()) {
    return nullptr;
  }
  if (mumbleLink.charName.empty()) {
    return nullptr;
  }
  if (keys.empty()) {
    return nullptr;
  }

  if (!initialized) {
    Initialize();
  }

  for (auto& key : keys) {
    if (!key->Valid()) {
      continue;
    }

    if (key->fetcherThread.joinable()) {
      key->fetcherThread.join();
    }

    auto& cn = key->key_data.char_names;
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
    std::scoped_lock l(keyMutex);
    if (keys.empty()) {
      return Status::KeyNotSet;
    }
  }

  APIKey* key = GetIdentifiedAPIKey();
  if (!key) {
    std::scoped_lock l(keyMutex);
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

APIKeyManager::Status APIKeyManager::DisplayStatusText(CWBDrawAPI* API,
                                                       CWBFont* font) {
  APIKeyManager::Status status = GetStatus();

  switch (status) {
    case Status::Loading:
      font->Write(API, DICT("waitingforapi"), CPoint(0, 0));
      break;
    case Status::KeyNotSet:
      font->Write(API, DICT("apikeynotset1"), CPoint(0, 0),
                  CColor(0xff, 0x40, 0x40, 0xff));
      font->Write(API, DICT("apikeynotset2"), CPoint(0, font->GetLineHeight()),
                  CColor(0xff, 0x40, 0x40, 0xff));
      break;
    case Status::CouldNotIdentifyAccount:
      font->Write(API, DICT("couldntidentifyaccount1"), CPoint(0, 0),
                  CColor(0xff, 0x40, 0x40, 0xff));
      font->Write(API, DICT("couldntidentifyaccount2"),
                  CPoint(0, font->GetLineHeight()),
                  CColor(0xff, 0x40, 0x40, 0xff));
      break;
    case Status::WaitingForMumbleCharacterName:
      font->Write(API, DICT("waitingforcharactername1"), CPoint(0, 0));
      break;
    case Status::AllKeysInvalid:
      font->Write(API, DICT("apierror1"), CPoint(0, 0),
                  CColor(0xff, 0x40, 0x40, 0xff));
      font->Write(API, DICT("apierror2"), CPoint(0, font->GetLineHeight()),
                  CColor(0xff, 0x40, 0x40, 0xff));
      break;
  }
  return status;
}

void APIKeyManager::Initialize() {
  if (initialized) return;

  if (HasConfigString("GW2APIKey")) {
    auto key = std::make_unique<APIKey>(GetConfigString("GW2APIKey"));
    RemoveConfigEntry("GW2APIKey");
    std::scoped_lock l(keyMutex);
    keys.emplace_back(std::move(key));
  }

  int x = 0;
  while (true) {
    std::string cfgName = "GW2APIKey" + std::to_string(x++);
    if (HasConfigString(cfgName)) {
      auto key = std::make_unique<APIKey>(GetConfigString(cfgName));
      std::scoped_lock l(keyMutex);
      keys.emplace_back(std::move(key));
    } else {
      break;
    }
  }

  RebuildConfigValues();

  std::scoped_lock l(keyMutex);
  for (auto& key : keys) {
    key->FetchData();
  }

  initialized = true;
}

bool APIKeyManager::empty() {
  std::scoped_lock l(keyMutex);
  return keys.empty();
}

APIKey* APIKeyManager::GetKey(int idx) {
  std::scoped_lock l(keyMutex);
  return keys[idx].get();
}

void APIKeyManager::RemoveKey(int idx) {
  std::scoped_lock l(keyMutex);
  keys.erase(keys.begin() + idx);
}

size_t APIKeyManager::size() {
  std::scoped_lock l(keyMutex);
  return keys.size();
}

void APIKeyManager::AddKey(std::unique_ptr<APIKey>&& key) {
  std::scoped_lock l(keyMutex);
  keys.emplace_back(std::move(key));
}

void APIKeyManager::RebuildConfigValues() {
  int x = 0;
  while (true) {
    std::string cfgName = "GW2APIKey" + std::to_string(x++);
    if (HasConfigString(cfgName)) {
      RemoveConfigEntry(cfgName);
    } else {
      break;
    }
  }

  std::scoped_lock l(keyMutex);
  for (size_t x = 0; x < keys.size(); x++) {
    std::string cfgName = "GW2APIKey" + std::to_string(x);
    SetConfigString(cfgName, keys[x]->apiKey);
  }

  SaveConfig();
}
}  // namespace GW2
