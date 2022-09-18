#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/white_board/draw_api.h"

namespace GW2 {
class APIKey {
 public:
  APIKey() = default;
  explicit APIKey(std::string_view key);
  virtual ~APIKey();

  void FetchData();
  bool HasCaps(std::string_view cap);
  std::string QueryAPI(std::string_view path) const;
  void SetKey(std::string_view key);
  std::unordered_set<std::string> Dungeons() const;
  std::unordered_set<int32_t> DungeonAchievements() const;
  std::unordered_set<std::string> Raids() const;
  std::unordered_set<std::string> WorldBosses() const;
  std::unordered_set<std::string> Mapchests() const;

  std::string apiKey;
  std::unordered_map<std::string, bool> caps;
  std::string keyName;
  std::string accountName;
  std::vector<std::string> charNames;
  int worldId = 0;
  bool initialized = false;
  bool valid = true;
  bool beingInitialized = false;
  std::thread fetcherThread;

 private:
   std::unordered_set<std::string> QuerySet(std::string_view path) const;
   std::unordered_set<int32_t> QueryAchievementBits(int id) const;
};

class APIKeyManager {
 public:
  enum class Status {
    OK,
    Loading,
    KeyNotSet,
    CouldNotIdentifyAccount,
    WaitingForMumbleCharacterName,
    AllKeysInvalid
  };

  bool empty();
  APIKey* GetKey(int idx);
  void AddKey(std::unique_ptr<APIKey>&& key);
  void RemoveKey(int idx);
  size_t size();
  APIKey* GetIdentifiedAPIKey();
  Status GetStatus();
  Status DisplayStatusText(CWBDrawAPI* API, CWBFont* font);
  void Initialize();
  void RebuildConfigValues();

 private:
  bool initialized = false;
  std::mutex keyMutex;
  std::vector<std::unique_ptr<APIKey>> keys;
};

extern APIKeyManager apiKeyManager;
}  // namespace GW2
