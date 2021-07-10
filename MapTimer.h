#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"
#include <thread>
#include <vector>

class GW2MapTimer : public CWBItem
{
  struct Event
  {
    std::string name;
    std::string waypoint;
    std::string worldBossId;
    int length;
    int start;
    CColor color;
  };

  struct Map
  {
    std::string name;
    std::string chestId;
    int Length;
    int Start;
    std::string id;
    TBOOL display = true;
    std::vector<Event> events;
  };

  CPoint lastpos;
  virtual void OnDraw( CWBDrawAPI *API );
  void SetLayout( CXMLNode &node );

  bool beingFetched = false;
  int32_t lastFetchTime = 0;

  bool hasWorldBossInfo = false;

  std::thread fetchThread;

  std::vector<std::string> worldBosses;
  std::vector<std::string> mapchests;

  LIGHTWEIGHT_CRITICALSECTION critSec;

public:

  std::vector<Map> maps;

  GW2MapTimer( CWBItem *Parent, CRect Position );
  virtual ~GW2MapTimer();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "maptimer" ), CWBItem );

  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );
};


