#pragma once
#include "Bedrock/BaseLib/BaseLib.h"

class TS3Connection
{
  CSocket connection;

  struct CommandResponse
  {
    CStringArray Lines;
    int32_t ErrorCode = -1;
    CString Message;
  };

  int32_t currentHandlerID = 1;

  void ProcessNotification( CString &s );
  void ProcessChannelList( CString &channeldata, int32_t handler );
  void ProcessClientList( CString &clientdata, int32_t handler );
  CString ReadLine();

  int32_t LastPingTime = 0;

public:

  class TS3Client
  {
  public:
    int32_t clientid = 0;
    int32_t channelid = 0;
    CString name;
    int32_t talkStatus = 0;
    int32_t inputmuted = 0;
    int32_t outputmuted = 0;

    TU64 lastTalkTime = 0;
  };

  class TS3Channel
  {
  public:
    int32_t id = 0;
    int32_t parentid = 0;
    int32_t order = 0;
    CString name;
  };

  class TS3Schandler
  {
  public:
    int32_t id = 0;
    TBOOL Connected = false;
    int32_t myclientid = 0;
    TBOOL clientIDInvalid = true;
    CDictionaryEnumerable<int32_t, TS3Channel> Channels;
    CDictionaryEnumerable<int32_t, TS3Client> Clients;
    CString name;
  };

  CDictionary<int32_t, TS3Schandler> handlers;

  TS3Connection();
  virtual ~TS3Connection();

  TBOOL TryConnect();
  void TryValidateClientID();

  void Tick();
  void InitConnection();

  CommandResponse SendCommand( CString &message );
  CommandResponse SendCommand( TCHAR *message );

  void ProcessNotifications();

  void WaitForResponse();

  TBOOL IsConnected();

  CString unescape( CString string );

  bool authenticated = true;

};

extern TS3Connection teamSpeakConnection;
