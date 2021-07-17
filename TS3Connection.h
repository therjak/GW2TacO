#pragma once
#include <string>
#include <string_view>

#include "Bedrock/BaseLib/BaseLib.h"
#include "Bedrock/BaseLib/Dictionary.h"

class TS3Connection {
  CSocket connection;

  struct CommandResponse {
    std::vector<std::string> Lines;
    int32_t ErrorCode = -1;
    std::string Message;
  };

  int32_t currentHandlerID = 1;

  void ProcessNotification(std::string_view s);
  void ProcessChannelList(std::string_view channeldata, int32_t handler);
  void ProcessClientList(std::string_view clientdata, int32_t handler);
  std::string ReadLine();

  int32_t LastPingTime = 0;

 public:
  class TS3Client {
   public:
    int32_t clientid = 0;
    int32_t channelid = 0;
    std::string name;
    int32_t talkStatus = 0;
    int32_t inputmuted = 0;
    int32_t outputmuted = 0;

    uint64_t lastTalkTime = 0;
  };

  class TS3Channel {
   public:
    int32_t id = 0;
    int32_t parentid = 0;
    int32_t order = 0;
    std::string name;
  };

  class TS3Schandler {
   public:
    int32_t id = 0;
    TBOOL Connected = false;
    int32_t myclientid = 0;
    TBOOL clientIDInvalid = true;
    CDictionaryEnumerable<int32_t, TS3Channel> Channels;
    CDictionaryEnumerable<int32_t, TS3Client> Clients;
    std::string name;
  };

  CDictionary<int32_t, TS3Schandler> handlers;

  TS3Connection();
  virtual ~TS3Connection();

  bool TryConnect();
  void TryValidateClientID();

  void Tick();
  void InitConnection();

  CommandResponse SendCommand(std::string_view message);

  void ProcessNotifications();

  bool IsConnected();

  std::string unescape(std::string_view string);

  bool authenticated = false;
};

extern TS3Connection teamSpeakConnection;
