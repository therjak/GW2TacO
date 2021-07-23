#pragma once
#include <thread>
#include <vector>

#include "Bedrock/UtilLib/jsonxx.h"
#include "Bedrock/Whiteboard/DrawAPI.h"
#include "Bedrock/Whiteboard/GuiItem.h"

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

class TPTracker : public CWBItem {
  void OnDraw(CWBDrawAPI* API) override;

  bool beingFetched = false;
  int32_t lastFetchTime = 0;

  std::thread fetchThread;

  std::vector<TransactionItem> buys;
  std::vector<TransactionItem> sells;
  static bool ParseTransaction(jsonxx::Object& object, TransactionItem& output);

  LIGHTWEIGHT_CRITICALSECTION dataWriteCritSec;

 public:
  TPTracker(CWBItem* Parent, CRect Position);
  static inline std::shared_ptr<TPTracker> Create(CWBItem* Parent,
                                                  CRect Position) {
    auto p = std::make_shared<TPTracker>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~TPTracker() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "tptracker" ), CWBItem);

  bool IsMouseTransparent(CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
};
