#pragma once
#include "Bedrock/BaseLib/BaseLib.h"
#include "Bedrock/Whiteboard/WhiteBoard.h"
#include <thread>
#include <string>
#include <string_view>
#include <unordered_map>

namespace GW2
{
  class APIKey
  {
  public:

    std::string apiKey;
    std::unordered_map< std::string, bool > caps;
    std::string keyName;
    std::string accountName;
    CStringArray charNames;
    int worldId = 0;

    APIKey() = default;
    APIKey( std::string_view key );
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

  class APIKeyManager
  {
    bool initialized = false;

  public:

    enum class Status
    {
      OK,
      Loading,
      KeyNotSet,
      CouldNotIdentifyAccount,
      WaitingForMumbleCharacterName,
      AllKeysInvalid
    };

    CArrayThreadSafe<APIKey*> keys;

    APIKey* GetIdentifiedAPIKey();
    Status GetStatus();
    Status DisplayStatusText(CWBDrawAPI* API, CWBFont* font);
    void Initialize();
    void RebuildConfigValues();
  };

  extern APIKeyManager apiKeyManager;

}
