#include "TS3Connection.h"

#include "Bedrock/BaseLib/string_format.h"
#include "OverlayConfig.h"

TS3Connection teamSpeakConnection;

TS3Connection::TS3Connection() { InitWinsock(); }

TS3Connection::~TS3Connection() { DeinitWinsock(); }

bool TS3Connection::TryConnect() {
  if (connection.IsConnected()) return true;

  handlers.Flush();

  bool connected = connection.Connect("localhost", 25639);
  if (!connected) return false;

  ReadLine();
  ReadLine();
  ReadLine();

  InitConnection();

  return true;
}

void TS3Connection::TryValidateClientID() {
  for (int32_t x = 0; x < handlers.NumItems(); x++)
    if (handlers[x].Connected && handlers[x].clientIDInvalid) {
      currentHandlerID = handlers[x].id;
      CommandResponse use =
          SendCommand(FormatString("use %d", currentHandlerID));
      if (use.ErrorCode) continue;
      CommandResponse whoami = SendCommand("whoami");
      handlers[currentHandlerID].Connected = whoami.ErrorCode != 1794;

      if (whoami.ErrorCode == 512)
        handlers[currentHandlerID].clientIDInvalid = true;

      if (!whoami.ErrorCode) {
        handlers[currentHandlerID].clientIDInvalid = false;
        currentHandlerID = currentHandlerID;
        int32_t clid = 0, cid = 0;
        std::sscanf(whoami.Lines[0].c_str(), "clid=%d cid=%d", &clid, &cid);
        handlers[currentHandlerID].Clients[clid].clientid = clid;
        handlers[currentHandlerID].Clients[clid].channelid = cid;
        handlers[currentHandlerID].myclientid = clid;

        CommandResponse serverName =
            SendCommand("servervariable virtualserver_name");
        if (!serverName.ErrorCode)
          if (serverName.Lines[0].find("virtualserver_name=") == 0)
            handlers[currentHandlerID].name =
                unescape(serverName.Lines[0].substr(19));

        CommandResponse channelList = SendCommand("channellist");
        if (!channelList.ErrorCode)
          ProcessChannelList(channelList.Lines[0], currentHandlerID);

        CommandResponse clientList = SendCommand("clientlist -voice");
        if (!clientList.ErrorCode)
          ProcessClientList(clientList.Lines[0], currentHandlerID);
      }
    }
}

void TS3Connection::Tick() {
  if (!connection.IsConnected()) {
    if (FindWindow(NULL, "TeamSpeak 3")) {
      if (!TryConnect()) return;
    } else
      return;
  } else {
    TryValidateClientID();
  }

  if (!authenticated) {
    if (GetTime() - LastPingTime > 1000) {
      InitConnection();
      LastPingTime = GetTime();
    }

    if (!authenticated) return;
  }

  ProcessNotifications();

  if (GetTime() - LastPingTime > 5000) {
    CommandResponse res = SendCommand("whoami");
    LastPingTime = GetTime();
  }
}

void TS3Connection::InitConnection() {
  if (HasConfigString("TS3APIKey")) {
    std::string apiKey = GetConfigString("TS3APIKey");
    auto response =
        SendCommand("auth apikey=" + apiKey);  // 3P9O-GWJ8-1TKI-OY1F-AX0T-BPQK
    if (response.ErrorCode)
      authenticated = false;
    else
      authenticated = true;
  }

  if (!authenticated) return;

  auto notifyresponse =
      SendCommand("clientnotifyregister schandlerid=0 event=any");

  currentHandlerID = 1;

  CommandResponse response = SendCommand("serverconnectionhandlerlist");
  if (!response.ErrorCode && !response.Lines.empty()) {
    auto schandlers = Split(response.Lines[0], "|");
    for (int32_t x = 0; x < schandlers.size(); x++)
      if (schandlers[x].find("schandlerid=") == 0) {
        TS3Schandler handler;
        std::sscanf(schandlers[x].c_str(), "schandlerid=%d", &handler.id);
        handlers[handler.id] = handler;
        CommandResponse use = SendCommand(FormatString("use %d", handler.id));
        if (use.ErrorCode) continue;
        CommandResponse whoami = SendCommand(FormatString("whoami"));
        handlers[handler.id].Connected = whoami.ErrorCode != 1794;

        if (whoami.ErrorCode == 512)
          handlers[handler.id].clientIDInvalid = true;

        if (!whoami.ErrorCode) {
          handlers[handler.id].clientIDInvalid = false;
          currentHandlerID = handler.id;
          int32_t clid = 0, cid = 0;
          std::sscanf(whoami.Lines[0].c_str(), "clid=%d cid=%d", &clid, &cid);
          handlers[handler.id].Clients[clid].clientid = clid;
          handlers[handler.id].Clients[clid].channelid = cid;
          handlers[handler.id].myclientid = clid;

          CommandResponse serverName =
              SendCommand("servervariable virtualserver_name");
          if (!serverName.ErrorCode)
            if (serverName.Lines[0].find("virtualserver_name=") == 0)
              handlers[handler.id].name =
                  unescape(serverName.Lines[0].substr(19));

          CommandResponse channelList = SendCommand("channellist");
          if (!channelList.ErrorCode)
            ProcessChannelList(channelList.Lines[0], handler.id);

          CommandResponse clientList = SendCommand("clientlist -voice");
          if (!clientList.ErrorCode)
            ProcessClientList(clientList.Lines[0], handler.id);
        }
      }
  }

  SendCommand(FormatString("use %d", currentHandlerID));
}

TS3Connection::CommandResponse TS3Connection::SendCommand(
    std::string_view message) {
  CommandResponse response;

  if (!connection.IsConnected()) return response;

  ProcessNotifications();

  connection.Write(message);
  connection.Write("\n");

  while (1) {
    if (!connection.IsConnected()) {
      response.ErrorCode = -1;
      response.Message = "Disconnected";
      return response;
    }

    auto nextline = ReadLine();
    response.Lines.push_back(nextline);
    if (nextline.find("error") == 0) {
      auto msg = SplitByWhitespace(nextline);
      if (msg.size() >= 2)
        std::sscanf(msg[1].c_str(), "id=%d", &response.ErrorCode);

      if (response.ErrorCode == 1796) authenticated = false;

      if (msg.size() >= 3) response.Message = msg[2].substr(4);

      if (response.ErrorCode)
        LOG_DBG("[GW2TacO] command %s response: %d %s",
                std::string(message).c_str(), response.ErrorCode,
                response.Message.c_str());
      break;
    }
  }

  return response;
}

void TS3Connection::ProcessNotifications() {
  while (connection.GetLength()) ProcessNotification(ReadLine());
}

int ClientTalkTimeSorter(const TS3Connection::TS3Client& a,
                         const TS3Connection::TS3Client& b) {
  return (int)(b.lastTalkTime - a.lastTalkTime);
}

void TS3Connection::ProcessNotification(std::string_view s) {
  auto cmd = SplitByWhitespace(s);

  int32_t schandlerid = 0;
  for (size_t x = 1; x < cmd.size(); x++)
    if (cmd[x].find("schandlerid=") == 0)
      std::sscanf(cmd[x].c_str(), "schandlerid=%d", &schandlerid);

  if (cmd[0] == "notifytalkstatuschange") {
    int32_t clientid = -1;
    int32_t status = -1;
    for (int32_t x = 1; x < cmd.size(); x++) {
      if (cmd[x].find("status=") == 0)
        std::sscanf(cmd[x].c_str(), "status=%d", &status);
      if (cmd[x].find("clid=") == 0)
        std::sscanf(cmd[x].c_str(), "clid=%d", &clientid);
    }

    if (clientid >= 0 && status >= 0) {
      if (handlers[schandlerid].Clients[clientid].talkStatus != status &&
          status > 0) {
        handlers[schandlerid].Clients[clientid].lastTalkTime = GetTime();
        handlers[schandlerid].Clients.SortByValue(ClientTalkTimeSorter);
      }

      handlers[schandlerid].Clients[clientid].talkStatus = status;
    }
    return;
  }

  if (cmd[0] == "notifyclientids") {
    return;
  }

  if (cmd[0] == "notifyclientmoved") {
    int32_t clientid = -1;
    int32_t channelid = -1;
    for (int32_t x = 1; x < cmd.size(); x++) {
      if (cmd[x].find("ctid=") == 0)
        std::sscanf(cmd[x].c_str(), "ctid=%d", &channelid);
      if (cmd[x].find("clid=") == 0)
        std::sscanf(cmd[x].c_str(), "clid=%d", &clientid);
    }

    if (clientid >= 0 && channelid >= 0) {
      handlers[schandlerid].Clients[clientid].channelid = channelid;
      if (channelid == handlers[schandlerid]
                           .Clients[handlers[schandlerid].myclientid]
                           .channelid) {
        handlers[schandlerid].Clients[clientid].lastTalkTime = GetTime();
        handlers[schandlerid].Clients.SortByValue(ClientTalkTimeSorter);
      }
    }
    return;
  }

  if (cmd[0] == "notifycurrentserverconnectionchanged") {
    currentHandlerID = schandlerid;
    return;
  }

  if (cmd[0] == "notifyconnectstatuschange") {
    for (int32_t x = 1; x < cmd.size(); x++) {
      if (cmd[x].find("status=disconnected") == 0) {
        handlers[schandlerid].Connected = false;
        handlers[schandlerid].Channels.Flush();
        handlers[schandlerid].Clients.Flush();
      }

      if (cmd[x].find("status=connecting") == 0) {
        handlers[schandlerid].Connected = false;
        handlers[schandlerid].Channels.Flush();
        handlers[schandlerid].Clients.Flush();
      }

      if (cmd[x].find("status=connected") == 0) {
        handlers[schandlerid].Connected = true;
        CommandResponse use = SendCommand(FormatString("use %d", schandlerid));
        if (!use.ErrorCode) {
          currentHandlerID = schandlerid;
          CommandResponse whoami = SendCommand("whoami");
          if (!whoami.ErrorCode) {
            int32_t clid = 0, cid = 0;
            std::sscanf(whoami.Lines[0].c_str(), "clid=%d cid=%d", &clid, &cid);
            handlers[schandlerid].Clients[clid].clientid = clid;
            handlers[schandlerid].Clients[clid].channelid = cid;
            handlers[schandlerid].myclientid = clid;
          }
          CommandResponse serverName =
              SendCommand("servervariable virtualserver_name");
          if (!serverName.ErrorCode)
            if (serverName.Lines[0].find("virtualserver_name=") == 0)
              handlers[schandlerid].name =
                  unescape(serverName.Lines[0].substr(19));
        }
      }
    }
    return;
  }

  if (cmd[0] == "channellist") {
    ProcessChannelList(s, schandlerid);
    return;
  }

  if (cmd[0] == "channellistfinished") {
    return;
  }

  if (cmd[0] == "notifycliententerview") {
    ProcessClientList(s, schandlerid);
    return;
  }

  if (cmd[0] == "notifyclientleftview") {
    int32_t clientid = -1;
    for (int32_t x = 1; x < cmd.size(); x++) {
      if (cmd[x].find("clid=") == 0)
        std::sscanf(cmd[x].c_str(), "clid=%d", &clientid);
    }
    if (clientid >= 0) handlers[schandlerid].Clients.Delete(clientid);
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
    int32_t clientid = -1;
    for (int32_t x = 1; x < cmd.size(); x++) {
      if (cmd[x].find("clid=") == 0)
        std::sscanf(cmd[x].c_str(), "clid=%d", &clientid);
    }

    if (clientid >= 0) {
      for (int32_t x = 1; x < cmd.size(); x++) {
        if (cmd[x].find("client_input_muted=") == 0)
          std::sscanf(cmd[x].c_str(), "client_input_muted=%d",
                      &handlers[schandlerid].Clients[clientid].inputmuted);
        if (cmd[x].find("client_output_muted=") == 0)
          std::sscanf(cmd[x].c_str(), "client_output_muted=%d",
                      &handlers[schandlerid].Clients[clientid].outputmuted);
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
  if (!connection.IsConnected()) return "";

  std::string lne = connection.ReadLine();
  if (connection.GetLength()) {
    char c;
    if (connection.Peek(&c, 1))
      if (c == '\r') c = connection.ReadByte();
  }
  return lne;
}

void TS3Connection::ProcessChannelList(std::string_view channeldata,
                                       int32_t handler) {
  auto channels = Split(channeldata, "|");
  for (int32_t x = 0; x < channels.size(); x++) {
    auto channelData = SplitByWhitespace(channels[x]);
    TS3Channel channel;
    for (const auto& cd : channelData) {
      if (cd.find("cid=") == 0) {
        std::sscanf(cd.c_str(), "cid=%d", &channel.id);
        continue;
      }
      if (cd.find("pid=") == 0) {
        std::sscanf(cd.c_str(), "pid=%d", &channel.parentid);
        continue;
      }
      if (cd.find("cpid=") == 0) {
        std::sscanf(cd.c_str(), "cpid=%d", &channel.parentid);
        continue;
      }
      if (cd.find("channel_order=") == 0) {
        std::sscanf(cd.c_str(), "channel_order=%d", &channel.order);
        continue;
      }
      if (cd.find("channel_name=") == 0) {
        channel.name = unescape(cd.substr(13));
        continue;
      }
    }
    handlers[handler].Channels[channel.id] = channel;
  }
}

void TS3Connection::ProcessClientList(std::string_view clientdata,
                                      int32_t handler) {
  bool needsSort = false;

  auto channels = Split(clientdata, "|");
  for (int32_t x = 0; x < channels.size(); x++) {
    auto clientData = SplitByWhitespace(channels[x]);
    TS3Client client;
    for (const auto& cd : clientData) {
      if (cd.find("cid=") == 0) {
        std::sscanf(cd.c_str(), "cid=%d", &client.channelid);
        continue;
      }
      if (cd.find("ctid=") == 0) {
        std::sscanf(cd.c_str(), "ctid=%d", &client.channelid);
        continue;
      }
      if (cd.find("client_channel_group_inherited_channel_id=") == 0) {
        std::sscanf(cd.c_str(), "client_channel_group_inherited_channel_id=%d",
                    &client.channelid);
        continue;
      }
      if (cd.find("clid=") == 0) {
        std::sscanf(cd.c_str(), "clid=%d", &client.clientid);
        continue;
      }
      if (cd.find("client_input_muted=") == 0) {
        std::sscanf(cd.c_str(), "client_input_muted=%d", &client.inputmuted);
        continue;
      }
      if (cd.find("client_output_muted=") == 0) {
        std::sscanf(cd.c_str(), "client_output_muted=%d", &client.outputmuted);
        continue;
      }
      if (cd.find("client_nickname=") == 0) {
        client.name = unescape(cd.substr(16));
        continue;
      }
    }
    handlers[handler].Clients[client.clientid] = client;
    handlers[handler].Clients[client.clientid].lastTalkTime = GetTime();
    if (handlers[handler].myclientid &&
        handlers[handler].Clients[client.clientid].channelid ==
            handlers[handler].Clients[handlers[handler].myclientid].channelid)
      needsSort = true;
  }

  if (needsSort) handlers[handler].Clients.SortByValue(ClientTalkTimeSorter);
}

std::string TS3Connection::unescape(std::string_view string) {
  std::string result;
  for (uint32_t x = 0; x < string.size(); x++) {
    if (string[x] == '\\') {
      if (x == string.size() - 1) break;

      if (x) result += string.substr(0, x);

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

bool TS3Connection::IsConnected() { return connection.IsConnected(); }

