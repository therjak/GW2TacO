#pragma once
#include <thread>
#include <vector>

#include "Bedrock/UtilLib/jsonxx.h"
#include "Bedrock/WhiteBoard/WhiteBoard.h"

struct TransactionItem {
  int32_t transactionID = 0;
  int32_t itemID = 0;
  int32_t price = 0;
  int32_t quantity = 0;
};

struct GW2ItemData {
  int32_t itemID = 0;
  std::string name;
  WBATLASHANDLE icon = 0;
  int32_t buyPrice = 0;
  int32_t sellPrice = 0;
};

extern CDictionary<int32_t, GW2ItemData> itemDataCache;

class TPTracker : public CWBItem {
  virtual void OnDraw(CWBDrawAPI *API);

  bool beingFetched = false;
  int32_t lastFetchTime = 0;

  std::thread fetchThread;

  std::vector<TransactionItem> buys;
  std::vector<TransactionItem> sells;
  static TBOOL ParseTransaction(jsonxx::Object &object,
                                TransactionItem &output);

  LIGHTWEIGHT_CRITICALSECTION dataWriteCritSec;

 public:
  TPTracker(CWBItem *Parent, CRect Position);
  static inline std::shared_ptr<TPTracker> Create(CWBItem *Parent,
                                                  CRect Position) {
    auto p = std::make_shared<TPTracker>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  virtual ~TPTracker();

  static CWBItem *Factory(CWBItem *Root, CXMLNode &node, CRect &Pos);
  WB_DECLARE_GUIITEM(_T( "tptracker" ), CWBItem);

  virtual TBOOL IsMouseTransparent(CPoint &ClientSpacePoint,
                                   WBMESSAGE MessageType);
};
