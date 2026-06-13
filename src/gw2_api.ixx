module;
#include <atomic>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "src/base/lock_free_queue.h"

export module taco.gw2;

import whiteboard;

export namespace GW2 {
struct KeyData {
  std::unordered_set<std::string> caps;
  std::string key_name;
  std::string account_name;
  std::unordered_set<std::string> char_names;
  int world_id = 0;
  bool valid = true;
};

class APIKeyManager;

class APIKey {
 public:
  APIKey() = default;
  explicit APIKey(std::string_view key);
  virtual ~APIKey();

  void FetchData();
  bool HasCaps(std::string_view cap);
  std::string QueryAPI(std::string_view path) const;
  void SetKey(std::string_view key);
  bool Valid();
  std::string_view AccountName() const { return key_data.account_name; }
  int WorldID() const { return key_data.world_id; }

  std::unordered_set<std::string> QuerySet(std::string_view path) const;
  std::unordered_set<int32_t> QueryAchievementBits(int id) const;
  std::string Key() const { return api_key_; }

 private:
  friend class APIKeyManager;
  std::string api_key_;
  std::atomic<bool> initialized_ = false;
  std::future<void> fetch_task_;
  KeyData key_data;
  LockFreeQueue<KeyData> key_data_queue;
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
  Status DisplayStatusText(gui::DrawAPI* API, gui::Font* font);
  void Initialize();
  void RebuildConfigValues();

 private:
  bool initialized_ = false;
  std::mutex key_mutex_;
  std::vector<std::unique_ptr<APIKey>> keys_;
};

APIKeyManager api_key_manager;
}  // namespace GW2
