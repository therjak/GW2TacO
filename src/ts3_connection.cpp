module;
#include <cstdio>
#include <format>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "src/base/logger.h"
#include "src/base/socket.h"
#include "src/base/string_format.h"
#include "src/gw2_tactical.h"

module taco.ts3connection;

import taco.overlay_config;
import time;

TS3Connection teamSpeakConnection;

TS3Connection::TS3Connection() { InitWinsock(); }

TS3Connection::~TS3Connection() { DeinitWinsock(); }

bool TS3Connection::TryConnect() {
  if (connection_.IsConnected()) {
    return true;
  }

  handlers_.clear();

  bool connected = connection_.Connect("localhost", 25639);
  if (!connected) {
    return false;
  }

  ReadLine();
  ReadLine();
  ReadLine();

  InitConnection();

  return true;
}

void TS3Connection::TryValidateClientID() {
  for (const auto& handler : handlers_) {
    if (handler.second.connected && handler.second.client_id_invalid) {
      current_handler_id_ = handler.second.id;
      CommandResponse use =
          SendCommand(std::format("use {:d}", current_handler_id_));
      if (use.error_code) {
        continue;
      }
      CommandResponse whoami = SendCommand("whoami");
      handlers_[current_handler_id_].connected = whoami.error_code != 1794;

      if (whoami.error_code == 512) {
        handlers_[current_handler_id_].client_id_invalid = true;
      }

      if (!whoami.error_code) {
        handlers_[current_handler_id_].client_id_invalid = false;
        current_handler_id_ = current_handler_id_;
        int32_t clid = 0, cid = 0;
        std::sscanf(whoami.lines[0].c_str(), "clid=%d cid=%d", &clid, &cid);
        handlers_[current_handler_id_].clients[clid].client_id = clid;
        handlers_[current_handler_id_].clients[clid].channel_id = cid;
        handlers_[current_handler_id_].my_client_id = clid;

        CommandResponse server_name =
            SendCommand("servervariable virtualserver_name");
        if (!server_name.error_code) {
          if (server_name.lines[0].find("virtualserver_name=") == 0) {
            handlers_[current_handler_id_].name =
                Unescape(server_name.lines[0].substr(19));
          }
        }

        CommandResponse channel_list = SendCommand("channellist");
        if (!channel_list.error_code) {
          ProcessChannelList(channel_list.lines[0], current_handler_id_);
        }

        CommandResponse client_list = SendCommand("clientlist -voice");
        if (!client_list.error_code) {
          ProcessClientList(client_list.lines[0], current_handler_id_);
        }
      }
    }
  }
}

void TS3Connection::Tick() {
  if (!connection_.IsConnected()) {
    if (GetTime() - last_ping_time_ > 1000) {
      if (FindWindow(nullptr, "TeamSpeak 3")) {
        if (!TryConnect()) {
          return;
        }
      } else {
        last_ping_time_ = GetTime();
        return;
      }
    }
  } else {
    TryValidateClientID();
  }

  if (!authenticated_) {
    if (GetTime() - last_ping_time_ > 1000) {
      InitConnection();
      last_ping_time_ = GetTime();
    }

    if (!authenticated_) {
      return;
    }
  }

  ProcessNotifications();

  if (GetTime() - last_ping_time_ > 5000) {
    CommandResponse res = SendCommand("whoami");
    last_ping_time_ = GetTime();
  }
}

void TS3Connection::InitConnection() {
  if (HasConfigString("TS3APIKey")) {
    std::string api_key = GetConfigString("TS3APIKey");
    auto response =
        SendCommand("auth apikey=" + api_key);  // 3P9O-GWJ8-1TKI-OY1F-AX0T-BPQK
    if (response.error_code) {
      authenticated_ = false;
    } else {
      authenticated_ = true;
    }
  }

  if (!authenticated_) {
    return;
  }

  auto notify_response =
      SendCommand("clientnotifyregister schandlerid=0 event=any");

  current_handler_id_ = 1;

  CommandResponse response = SendCommand("serverconnectionhandlerlist");
  if (!response.error_code && !response.lines.empty()) {
    auto schandlers = Split(response.lines[0], "|");
    for (auto& schandler : schandlers) {
      if (schandler.find("schandlerid=") == 0) {
        TS3Schandler handler;
        std::sscanf(schandler.c_str(), "schandlerid=%d", &handler.id);
        handlers_[handler.id] = handler;
        CommandResponse use = SendCommand(std::format("use {:d}", handler.id));
        if (use.error_code) {
          continue;
        }
        CommandResponse whoami = SendCommand("whoami");
        handlers_[handler.id].connected = whoami.error_code != 1794;

        if (whoami.error_code == 512) {
          handlers_[handler.id].client_id_invalid = true;
        }

        if (!whoami.error_code) {
          handlers_[handler.id].client_id_invalid = false;
          current_handler_id_ = handler.id;
          int32_t clid = 0, cid = 0;
          std::sscanf(whoami.lines[0].c_str(), "clid=%d cid=%d", &clid, &cid);
          handlers_[handler.id].clients[clid].client_id = clid;
          handlers_[handler.id].clients[clid].channel_id = cid;
          handlers_[handler.id].my_client_id = clid;

          CommandResponse server_name =
              SendCommand("servervariable virtualserver_name");
          if (!server_name.error_code) {
            if (server_name.lines[0].find("virtualserver_name=") == 0) {
              handlers_[handler.id].name =
                  Unescape(server_name.lines[0].substr(19));
            }
          }

          CommandResponse channel_list = SendCommand("channellist");
          if (!channel_list.error_code) {
            ProcessChannelList(channel_list.lines[0], handler.id);
          }

          CommandResponse client_list = SendCommand("clientlist -voice");
          if (!client_list.error_code) {
            ProcessClientList(client_list.lines[0], handler.id);
          }
        }
      }
    }
  }

  SendCommand(std::format("use {:d}", current_handler_id_));
}

TS3Connection::CommandResponse TS3Connection::SendCommand(
    std::string_view message) {
  CommandResponse response;

  if (!connection_.IsConnected()) {
    return response;
  }

  ProcessNotifications();

  connection_.Write(message);
  connection_.Write("\n");

  while (true) {
    if (!connection_.IsConnected()) {
      response.error_code = -1;
      response.message = "Disconnected";
      return response;
    }

    auto next_line = ReadLine();
    response.lines.push_back(next_line);
    if (next_line.find("error") == 0) {
      auto msg = SplitByWhitespace(next_line);
      if (msg.size() >= 2) {
        std::sscanf(msg[1].c_str(), "id=%d", &response.error_code);
      }

      if (response.error_code == 1796) {
        authenticated_ = false;
      }

      if (msg.size() >= 3) {
        response.message = msg[2].substr(4);
      }

      if (response.error_code) {
        Log_Dbg("[GW2TacO] command {:s} response: {:d} {:s}", message,
                response.error_code, response.message);
      }
      break;
    }
  }

  return response;
}

void TS3Connection::ProcessNotifications() {
  while (connection_.GetLength()) {
    ProcessNotification(ReadLine());
  }
}

int ClientTalkTimeSorter(const TS3Connection::TS3Client& a,
                         const TS3Connection::TS3Client& b) {
  return static_cast<int>(b.last_talk_time - a.last_talk_time);
}

void TS3Connection::ProcessNotification(std::string_view s) {
  auto cmd = SplitByWhitespace(s);

  int32_t schandler_id = 0;
  for (size_t x = 1; x < cmd.size(); x++) {
    if (cmd[x].find("schandlerid=") == 0) {
      std::sscanf(cmd[x].c_str(), "schandlerid=%d", &schandler_id);
    }
  }

  if (cmd[0] == "notifytalkstatuschange") {
    int32_t client_id = -1;
    int32_t status = -1;
    for (size_t x = 1; x < cmd.size(); x++) {
      if (cmd[x].find("status=") == 0) {
        std::sscanf(cmd[x].c_str(), "status=%d", &status);
      }
      if (cmd[x].find("clid=") == 0) {
        std::sscanf(cmd[x].c_str(), "clid=%d", &client_id);
      }
    }

    if (client_id >= 0 && status >= 0) {
      if (handlers_[schandler_id].clients[client_id].talk_status != status &&
          status > 0) {
        handlers_[schandler_id].clients[client_id].last_talk_time = GetTime();
      }

      handlers_[schandler_id].clients[client_id].talk_status = status;
    }
    return;
  }

  if (cmd[0] == "notifyclientids") {
    return;
  }

  if (cmd[0] == "notifyclientmoved") {
    int32_t client_id = -1;
    int32_t channel_id = -1;
    for (size_t x = 1; x < cmd.size(); x++) {
      if (cmd[x].find("ctid=") == 0) {
        std::sscanf(cmd[x].c_str(), "ctid=%d", &channel_id);
      }
      if (cmd[x].find("clid=") == 0) {
        std::sscanf(cmd[x].c_str(), "clid=%d", &client_id);
      }
    }

    if (client_id >= 0 && channel_id >= 0) {
      handlers_[schandler_id].clients[client_id].channel_id = channel_id;
      if (channel_id == handlers_[schandler_id]
                            .clients[handlers_[schandler_id].my_client_id]
                            .channel_id) {
        handlers_[schandler_id].clients[client_id].last_talk_time = GetTime();
      }
    }
    return;
  }

  if (cmd[0] == "notifycurrentserverconnectionchanged") {
    current_handler_id_ = schandler_id;
    return;
  }

  if (cmd[0] == "notifyconnectstatuschange") {
    for (size_t x = 1; x < cmd.size(); x++) {
      if (cmd[x].find("status=disconnected") == 0) {
        handlers_[schandler_id].connected = false;
        handlers_[schandler_id].channels.clear();
        handlers_[schandler_id].clients.clear();
      }

      if (cmd[x].find("status=connecting") == 0) {
        handlers_[schandler_id].connected = false;
        handlers_[schandler_id].channels.clear();
        handlers_[schandler_id].clients.clear();
      }

      if (cmd[x].find("status=connected") == 0) {
        handlers_[schandler_id].connected = true;
        CommandResponse use =
            SendCommand(std::format("use {:d}", schandler_id));
        if (!use.error_code) {
          current_handler_id_ = schandler_id;
          CommandResponse whoami = SendCommand("whoami");
          if (!whoami.error_code) {
            int32_t clid = 0, cid = 0;
            std::sscanf(whoami.lines[0].c_str(), "clid=%d cid=%d", &clid, &cid);
            handlers_[schandler_id].clients[clid].client_id = clid;
            handlers_[schandler_id].clients[clid].channel_id = cid;
            handlers_[schandler_id].my_client_id = clid;
          }
          CommandResponse server_name =
              SendCommand("servervariable virtualserver_name");
          if (!server_name.error_code) {
            if (server_name.lines[0].find("virtualserver_name=") == 0) {
              handlers_[schandler_id].name =
                  Unescape(server_name.lines[0].substr(19));
            }
          }
        }
      }
    }
    return;
  }

  if (cmd[0] == "channellist") {
    ProcessChannelList(s, schandler_id);
    return;
  }

  if (cmd[0] == "channellistfinished") {
    return;
  }

  if (cmd[0] == "notifycliententerview") {
    ProcessClientList(s, schandler_id);
    return;
  }

  if (cmd[0] == "notifyclientleftview") {
    int32_t client_id = -1;
    for (size_t x = 1; x < cmd.size(); x++) {
      if (cmd[x].find("clid=") == 0) {
        std::sscanf(cmd[x].c_str(), "clid=%d", &client_id);
      }
    }
    if (client_id >= 0) {
      handlers_[schandler_id].clients.erase(client_id);
    }
    return;
  }

  if (cmd[0] == "notifychannelgrouplist") {
    return;
  }

  if (cmd[0] == "notifyservergrouplist") {
    return;
  }

  if (cmd[0] == "notifyclientneededpermissions") {
    return;
  }

  if (cmd[0] == "notifyclientupdated") {
    int32_t client_id = -1;
    for (size_t x = 1; x < cmd.size(); x++) {
      if (cmd[x].find("clid=") == 0) {
        std::sscanf(cmd[x].c_str(), "clid=%d", &client_id);
      }
    }

    if (client_id >= 0) {
      for (size_t x = 1; x < cmd.size(); x++) {
        if (cmd[x].find("client_input_muted=") == 0) {
          std::sscanf(cmd[x].c_str(), "client_input_muted=%d",
                      &handlers_[schandler_id].clients[client_id].input_muted);
        }
        if (cmd[x].find("client_output_muted=") == 0) {
          std::sscanf(cmd[x].c_str(), "client_output_muted=%d",
                      &handlers_[schandler_id].clients[client_id].output_muted);
        }
      }
    }
    return;
  }

  if (cmd[0] == "notifychannelsubscribed") {
    return;
  }

  if (cmd[0] == "notifychanneledited") {
    return;
  }

  if (cmd[0] == "notifyclientchannelgroupchanged") {
    return;
  }
}

std::string TS3Connection::ReadLine() {
  if (!connection_.IsConnected()) {
    return "";
  }

  std::string lne = connection_.ReadLine();
  if (connection_.GetLength()) {
    char c = 0;
    if (connection_.Peek(&c, 1)) {
      if (c == '\r') {
        c = connection_.ReadByte();
      }
    }
  }
  return lne;
}

void TS3Connection::ProcessChannelList(std::string_view channeldata,
                                       int32_t handler) {
  auto channels = Split(channeldata, "|");
  for (auto& x : channels) {
    auto channel_data = SplitByWhitespace(x);
    TS3Channel channel;
    for (const auto& cd : channel_data) {
      if (cd.find("cid=") == 0) {
        std::sscanf(cd.c_str(), "cid=%d", &channel.id);
        continue;
      }
      if (cd.find("pid=") == 0) {
        std::sscanf(cd.c_str(), "pid=%d", &channel.parent_id);
        continue;
      }
      if (cd.find("cpid=") == 0) {
        std::sscanf(cd.c_str(), "cpid=%d", &channel.parent_id);
        continue;
      }
      if (cd.find("channel_order=") == 0) {
        std::sscanf(cd.c_str(), "channel_order=%d", &channel.order);
        continue;
      }
      if (cd.find("channel_name=") == 0) {
        channel.name = Unescape(cd.substr(13));
        continue;
      }
    }
    handlers_[handler].channels[channel.id] = channel;
  }
}

void TS3Connection::ProcessClientList(std::string_view clientdata,
                                      int32_t handler) {
  auto channels = Split(clientdata, "|");
  for (auto& channel : channels) {
    auto client_data = SplitByWhitespace(channel);
    TS3Client client;
    for (const auto& cd : client_data) {
      if (cd.find("cid=") == 0) {
        std::sscanf(cd.c_str(), "cid=%d", &client.channel_id);
        continue;
      }
      if (cd.find("ctid=") == 0) {
        std::sscanf(cd.c_str(), "ctid=%d", &client.channel_id);
        continue;
      }
      if (cd.find("client_channel_group_inherited_channel_id=") == 0) {
        std::sscanf(cd.c_str(), "client_channel_group_inherited_channel_id=%d",
                    &client.channel_id);
        continue;
      }
      if (cd.find("clid=") == 0) {
        std::sscanf(cd.c_str(), "clid=%d", &client.client_id);
        continue;
      }
      if (cd.find("client_input_muted=") == 0) {
        std::sscanf(cd.c_str(), "client_input_muted=%d", &client.input_muted);
        continue;
      }
      if (cd.find("client_output_muted=") == 0) {
        std::sscanf(cd.c_str(), "client_output_muted=%d", &client.output_muted);
        continue;
      }
      if (cd.find("client_nickname=") == 0) {
        client.name = Unescape(cd.substr(16));
        continue;
      }
    }
    handlers_[handler].clients[client.client_id] = client;
    handlers_[handler].clients[client.client_id].last_talk_time = GetTime();
  }
}

std::string TS3Connection::Unescape(std::string_view string) {
  std::string result;
  for (uint32_t x = 0; x < string.size(); x++) {
    if (string[x] == '\\') {
      if (x == string.size() - 1) {
        break;
      }

      if (x) {
        result += string.substr(0, x);
      }

      switch (string[x + 1]) {
        case '\\':
          result += "\\";
        case '/':
          result += "/";
        case 's':
          result += " ";
          break;
        case 'p':
          result += "|";
          break;
        case 'a':
          result += "\a";
          break;
        case 'b':
          result += "\b";
          break;
        case 'f':
          result += "\f";
          break;
        case 'n':
          result += "\n";
          break;
        case 'r':
          result += "\r";
          break;
        case 't':
          result += "\t";
          break;
        case 'v':
          result += "\v";
          break;
        default:
          break;
      }

      string = string.substr(x + 2);
      x = -1;
    }
  }

  return result + std::string(string);
}

bool TS3Connection::IsConnected() { return connection_.IsConnected(); }
