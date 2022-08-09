#pragma once
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/util/jsonxx.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

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

  std::mutex transaction_mtx;
  std::thread fetchThread;

  std::vector<TransactionItem> buys;
  std::vector<TransactionItem> sells;
  static bool ParseTransaction(jsonxx::Object& object, TransactionItem& output);

 public:
  TPTracker(CWBItem* Parent, math::CRect Position);
  static inline TPTracker* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_shared<TPTracker>(Parent, Position);
    TPTracker* r = p.get();
    assert(Parent);
    Parent->AddChild(p);
    return r;
  }
  ~TPTracker() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, math::CRect& Pos);
  WB_DECLARE_GUIITEM("tptracker", CWBItem);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
};
