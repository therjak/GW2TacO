#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"
#include <thread>
#include <vector>
#include <string>

class DungeonPath
{
public:
  std::string name;
  std::string type;
  bool finished = false;
  bool frequenter = false;
};

class Dungeon
{
public:
  std::string name;
  std::string shortName;
  std::vector<DungeonPath> paths;
};

class DungeonProgress : public CWBItem
{
  CPoint lastpos;
  virtual void OnDraw( CWBDrawAPI *API );

  bool beingFetched = false;
  int32_t lastFetchTime = 0;

  bool hasFullDungeonInfo = false;

  std::thread fetchThread;

  std::vector<Dungeon> dungeons;

public:

  DungeonProgress( CWBItem *Parent, CRect Position );
  virtual ~DungeonProgress();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "dungeonprogress" ), CWBItem );

  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );
};


