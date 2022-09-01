#pragma once
#include <string>
#include <string_view>
#include <unordered_map>

#include "src/base/socket.h"

class TS3Connection {
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
    bool Connected = false;
    int32_t myclientid = 0;
    bool clientIDInvalid = true;
    std::unordered_map<int32_t, TS3Channel> Channels;
    std::unordered_map<int32_t, TS3Client> Clients;
    std::string name;
  };

  struct CommandResponse {
    std::vector<std::string> Lines;
    int32_t ErrorCode = -1;
    std::string Message;
  };

  TS3Connection();
  virtual ~TS3Connection();

  bool TryConnect();
  void TryValidateClientID();
  void Tick();
  void InitConnection();
  void ProcessNotifications();
  bool IsConnected();
  std::string unescape(std::string_view string);

  std::unordered_map<int32_t, TS3Schandler> handlers;
  bool authenticated = false;

 private:
  CommandResponse SendCommand(std::string_view message);
  void ProcessNotification(std::string_view s);
  void ProcessChannelList(std::string_view channeldata, int32_t handler);
  void ProcessClientList(std::string_view clientdata, int32_t handler);
  std::string ReadLine();

  CSocket connection;
  int32_t currentHandlerID = 1;
  int32_t LastPingTime = 0;
};

extern TS3Connection teamSpeakConnection;
