#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Bedrock/BaseLib/base_lib.h"
#include "Bedrock/Whiteboard/draw_api.h"

namespace GW2 {
class APIKey {
 public:
  std::string apiKey;
  std::unordered_map<std::string, bool> caps;
  std::string keyName;
  std::string accountName;
  std::vector<std::string> charNames;
  int worldId = 0;

  APIKey() = default;
  APIKey(std::string_view key);
  virtual ~APIKey();

  void FetchData();
  bool HasCaps(std::string_view cap);

  std::string QueryAPI(std::string_view path);

  bool initialized = false;
  bool valid = true;
  bool beingInitialized = false;

  std::thread fetcherThread;

  void SetKey(std::string_view key);
};

class APIKeyManager {
  bool initialized = false;
  std::mutex keyMutex;
  std::vector<std::unique_ptr<APIKey>> keys;

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
};

extern APIKeyManager apiKeyManager;

}  // namespace GW2
