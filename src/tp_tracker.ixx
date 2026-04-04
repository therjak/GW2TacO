module;
#include <atomic>
#include <cassert>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/util/jsonxx.h"
#include "src/util/xml_node.h"
#include "src/white_board/atlas.h"

export module taco.tp_tracker;

import whiteboard;

export struct TransactionItem {
  int32_t transactionID = 0;
  int32_t itemID = 0;
  int32_t price = 0;
  int32_t quantity = 0;
};

export struct GW2ItemData {
  int32_t itemID = 0;
  std::string name;
  WBATLASHANDLE icon = 0;
  int32_t buyPrice = 0;
  int32_t sellPrice = 0;
};

export class TPTracker : public CWBGuiType<"tptracker", CWBItem> {
 public:
  TPTracker(CWBItem* Parent, math::CRect Position);
  ~TPTracker() override;
  static inline TPTracker* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_unique<TPTracker>(Parent, Position);
    TPTracker* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, math::CRect& Pos);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

 private:
  void OnDraw(CWBDrawAPI* API) override;
  static bool ParseTransaction(jsonxx::Object& object, TransactionItem& output);

  std::atomic<bool> being_fetched = false;
  int32_t lastFetchTime = 0;

  std::vector<TransactionItem> buys;
  std::vector<TransactionItem> sells;

  std::mutex transaction_mtx;
  std::thread fetchThread;
};
