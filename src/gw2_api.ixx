module;
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <vector>

#include "src/base/lock_free_queue.h"
#include "src/white_board/draw_api.h"

export module taco.gw2;

export namespace GW2 {

struct KeyData {
  std::unordered_set<std::string> caps;
  std::string key_name;
  std::string account_name;
  std::vector<std::string> char_names;
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

  std::string apiKey;

 private:
  friend class APIKeyManager;
  std::atomic<bool> initialized = false;
  std::thread fetcherThread;
  KeyData key_data;
  LockFreeQueue<KeyData> key_data_queue;
  std::atomic<bool> beingInitialized = false;
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

export extern APIKeyManager apiKeyManager;
}  // namespace GW2
