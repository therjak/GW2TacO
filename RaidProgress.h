#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"
#include <thread>
#include <string>

class RaidEvent
{
public:
  std::string name;
  std::string type;
  bool finished = false;
};

class Wing
{
public:
  std::string name;
  std::vector<RaidEvent> events;
};

class Raid
{
public:
  std::string name;
  std::string shortName;
  std::string configName;
  std::vector<Wing> wings;
};

class RaidProgress : public CWBItem
{
  CPoint lastpos;
  virtual void OnDraw( CWBDrawAPI *API );

  bool beingFetched = false;
  int32_t lastFetchTime = 0;

  bool hasFullRaidInfo = false;

  std::thread fetchThread;

  std::vector<Raid> raids;

public:

  RaidProgress( CWBItem *Parent, CRect Position );
  virtual ~RaidProgress();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "raidprogress" ), CWBItem );

  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );
  std::vector<Raid>& GetRaids();
};


