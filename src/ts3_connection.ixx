module;
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/base/socket.h"

export module taco.ts3connection;

export class TS3Connection {
 public:
  class TS3Client {
   public:
    int32_t client_id = 0;
    int32_t channel_id = 0;
    std::string name;
    int32_t talk_status = 0;
    int32_t input_muted = 0;
    int32_t output_muted = 0;

    uint64_t last_talk_time = 0;
  };

  class TS3Channel {
   public:
    int32_t id = 0;
    int32_t parent_id = 0;
    int32_t order = 0;
    std::string name;
  };

  class TS3Schandler {
   public:
    int32_t id = 0;
    bool connected = false;
    int32_t my_client_id = 0;
    bool client_id_invalid = true;
    std::unordered_map<int32_t, TS3Channel> channels;
    std::unordered_map<int32_t, TS3Client> clients;
    std::string name;
  };

  struct CommandResponse {
    std::vector<std::string> lines;
    int32_t error_code = -1;
    std::string message;
  };

  TS3Connection();
  virtual ~TS3Connection();

  bool TryConnect();
  void TryValidateClientID();
  void Tick();
  void InitConnection();
  void ProcessNotifications();
  bool IsConnected();
  std::string Unescape(std::string_view string);

  std::unordered_map<int32_t, TS3Schandler> handlers_;
  bool authenticated_ = false;

 private:
  CommandResponse SendCommand(std::string_view message);
  void ProcessNotification(std::string_view s);
  void ProcessChannelList(std::string_view channeldata, int32_t handler);
  void ProcessClientList(std::string_view clientdata, int32_t handler);
  std::string ReadLine();

  CSocket connection_;
  int32_t current_handler_id_ = 1;
  int32_t last_ping_time_ = 0;
};

export extern TS3Connection teamSpeakConnection;
