module;
#include <algorithm>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/base/color.h"
#include "src/base/logger.h"

module taco.gw2;

import math;
import taco.language;
import taco.mumble_link;
import taco.overlay_config;
import taco.web;
import whiteboard;

using math::Point;

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

APIKey::APIKey(std::string_view key) : api_key_(key) {}

APIKey::~APIKey() {}

void APIKey::FetchData() {
  if (fetch_task_.valid() && fetch_task_.wait_for(std::chrono::seconds(0)) !=
                                 std::future_status::ready)
    return;

  initialized_ = false;

  fetch_task_ = std::async(std::launch::async, [this]() {
    KeyData new_key_data;

    auto keyData = QueryAPI("/v2/tokeninfo");

    auto token_info = ParseTokenInfo(keyData);

    if (token_info.name.has_value()) {
      new_key_data.key_name = token_info.name.value();
    } else {
      new_key_data.valid = false;
    }

    if (token_info.permissions.has_value()) {
      for (const auto& cap : token_info.permissions.value()) {
        new_key_data.caps.insert(cap);
      }
    } else {
      new_key_data.valid = false;
    }

    if (new_key_data.caps.contains("account")) {
      auto accountData = QueryAPI("/v2/account");
      auto account_info = ParseAccountInfo(accountData);

      if (account_info.name.has_value()) {
        new_key_data.account_name = account_info.name.value();
      }

      if (account_info.world.has_value()) {
        new_key_data.world_id = account_info.world.value();
      }
    }
    if (new_key_data.caps.contains("characters")) {
      auto characterData = QueryAPI("/v2/characters");
      auto characters = ParseArray(characterData);
      if (characters.empty()) {
        Log_Err(
            "[GW2TacO] Unexpected result from API characters endpoint: {:s}",
            characterData);
        Log_Err(
            "[GW2TacO] CHARACTERS WON'T BE RECOGNIZED FOR API KEY NAMED {:s}",
            new_key_data.key_name);
      } else {
        new_key_data.char_names = characters;
      }
    } else {
      Log_Err(
          "[GW2TacO] API error: API key '{:s} - {:s} ({:s})' doesn't have the "
          "'characters' permission - account identification through Mumble "
          "Link will not be possible.",
          new_key_data.account_name, new_key_data.key_name, api_key_);
    }
    key_data_queue.push(new_key_data);

    initialized_ = true;
  });
}

bool APIKey::HasCaps(std::string_view cap) {
  return key_data.caps.contains(std::string(cap));
}

std::string APIKey::QueryAPI(std::string_view path) const {
  Log_Nfo("[GW2TacO] Querying the API: {:s}", path);

  return FetchAPIData(path, api_key_);
}

void APIKey::SetKey(std::string_view key) {
  if (fetch_task_.valid()) {
    fetch_task_.wait();
  }
  api_key_ = key;
  initialized_ = false;
  FetchData();
}

bool APIKey::Valid() {
  if (!initialized_) {
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
  return ParseArray(q);
}

std::unordered_set<int32_t> APIKey::QueryAchievementBits(int id) const {
  const auto q = QueryAPI("/v2/account/achievements?ids=" + std::to_string(id));
  std::unordered_set<int32_t> ret;
  auto achievement = ParseAccountAchievement(q);
  for (int32_t bit : achievement.bits) {
    ret.emplace(bit);
  }
  return ret;
}

APIKey* APIKeyManager::GetIdentifiedAPIKey() {
  std::scoped_lock l(key_mutex_);
  if (!mumbleLink.IsValid()) {
    return nullptr;
  }
  if (mumbleLink.char_name.empty()) {
    return nullptr;
  }
  if (keys_.empty()) {
    return nullptr;
  }

  if (!initialized_) {
    Initialize();
  }

  for (auto& key : keys_) {
    if (!key->Valid()) {
      continue;
    }

    if (key->fetch_task_.valid()) {
      key->fetch_task_.wait();
    }

    auto& cn = key->key_data.char_names;
    if (std::find(cn.begin(), cn.end(), mumbleLink.char_name) != cn.end()) {
      return key.get();
    }
  }

  return nullptr;
}

APIKeyManager::Status APIKeyManager::GetStatus() {
  if (!initialized_) {
    Initialize();
  }

  {
    std::scoped_lock l(key_mutex_);
    if (keys_.empty()) {
      return Status::KeyNotSet;
    }
  }

  APIKey* key = GetIdentifiedAPIKey();
  if (!key) {
    std::scoped_lock l(key_mutex_);
    for (const auto& key : keys_) {
      if (!key->initialized_) {
        return Status::Loading;
      }
    }
    if (mumbleLink.char_name.empty()) {
      return Status::WaitingForMumbleCharacterName;
    }
    return Status::CouldNotIdentifyAccount;
  }

  if (!key->initialized_) {
    return Status::Loading;
  }

  return Status::OK;
}

APIKeyManager::Status APIKeyManager::DisplayStatusText(gui::DrawAPI* API,
                                                       gui::Font* font) {
  APIKeyManager::Status status = GetStatus();

  switch (status) {
    case Status::Loading:
      font->Write(API, DICT("waitingforapi"), Point(0, 0));
      break;
    case Status::KeyNotSet:
      font->Write(API, DICT("apikeynotset1"), Point(0, 0),
                  CColor(0xff, 0x40, 0x40, 0xff));
      font->Write(API, DICT("apikeynotset2"), Point(0, font->GetLineHeight()),
                  CColor(0xff, 0x40, 0x40, 0xff));
      break;
    case Status::CouldNotIdentifyAccount:
      font->Write(API, DICT("couldntidentifyaccount1"), Point(0, 0),
                  CColor(0xff, 0x40, 0x40, 0xff));
      font->Write(API, DICT("couldntidentifyaccount2"),
                  Point(0, font->GetLineHeight()),
                  CColor(0xff, 0x40, 0x40, 0xff));
      break;
    case Status::WaitingForMumbleCharacterName:
      font->Write(API, DICT("waitingforcharactername1"), Point(0, 0));
      break;
    case Status::AllKeysInvalid:
      font->Write(API, DICT("apierror1"), Point(0, 0),
                  CColor(0xff, 0x40, 0x40, 0xff));
      font->Write(API, DICT("apierror2"), Point(0, font->GetLineHeight()),
                  CColor(0xff, 0x40, 0x40, 0xff));
      break;
  }
  return status;
}

void APIKeyManager::Initialize() {
  if (initialized_) return;

  if (HasConfigString("GW2APIKey")) {
    auto key = std::make_unique<APIKey>(GetConfigString("GW2APIKey"));
    RemoveConfigEntry("GW2APIKey");
    std::scoped_lock l(key_mutex_);
    keys_.emplace_back(std::move(key));
  }

  int x = 0;
  while (true) {
    std::string cfgName = "GW2APIKey" + std::to_string(x++);
    if (HasConfigString(cfgName)) {
      auto key = std::make_unique<APIKey>(GetConfigString(cfgName));
      std::scoped_lock l(key_mutex_);
      keys_.emplace_back(std::move(key));
    } else {
      break;
    }
  }

  RebuildConfigValues();

  std::scoped_lock l(key_mutex_);
  for (auto& key : keys_) {
    key->FetchData();
  }

  initialized_ = true;
}

bool APIKeyManager::empty() {
  std::scoped_lock l(key_mutex_);
  return keys_.empty();
}

APIKey* APIKeyManager::GetKey(int idx) {
  std::scoped_lock l(key_mutex_);
  return keys_[idx].get();
}

void APIKeyManager::RemoveKey(int idx) {
  std::scoped_lock l(key_mutex_);
  keys_.erase(keys_.begin() + idx);
}

size_t APIKeyManager::size() {
  std::scoped_lock l(key_mutex_);
  return keys_.size();
}

void APIKeyManager::AddKey(std::unique_ptr<APIKey>&& key) {
  std::scoped_lock l(key_mutex_);
  keys_.emplace_back(std::move(key));
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

  std::scoped_lock l(key_mutex_);
  for (size_t x = 0; x < keys_.size(); x++) {
    std::string cfgName = "GW2APIKey" + std::to_string(x);
    SetConfigString(cfgName, keys_[x]->api_key_);
  }

  SaveConfig();
}
}  // namespace GW2
