#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"
#include "Bedrock/UtilLib/jsonxx.h"
#include <thread>

struct TransactionItem
{
  int32_t transactionID = 0;
  int32_t itemID = 0;
  int32_t price = 0;
  int32_t quantity = 0;
};

struct GW2ItemData
{
  int32_t itemID = 0;
  CString name;
  WBATLASHANDLE icon = 0;
  int32_t buyPrice = 0;
  int32_t sellPrice = 0;
};

extern CDictionary<int32_t, GW2ItemData> itemDataCache;

class TPTracker : public CWBItem
{
  //CPoint lastpos;
  virtual void OnDraw( CWBDrawAPI *API );

  bool beingFetched = false;
  int32_t lastFetchTime = 0;

  //bool hasFullDungeonInfo = false;

  std::thread fetchThread;

  //CArray<Dungeon> dungeons;

  CArray<TransactionItem> buys;
  CArray<TransactionItem> sells;
  static TBOOL ParseTransaction( jsonxx::Object& object, TransactionItem& output );

  LIGHTWEIGHT_CRITICALSECTION dataWriteCritSec;

public:

  TPTracker( CWBItem *Parent, CRect Position );
  virtual ~TPTracker();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "tptracker" ), CWBItem );

  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );
};


