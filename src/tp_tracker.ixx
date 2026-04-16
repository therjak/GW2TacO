module;
#include <atomic>
#include <cassert>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "src/util/jsonxx.h"

export module taco.tp_tracker;

import math;
import whiteboard;
import xml;

export struct TransactionItem {
  int32_t transactionID = 0;
  int32_t itemID = 0;
  int32_t price = 0;
  int32_t quantity = 0;
};

export struct GW2ItemData {
  int32_t itemID = 0;
  std::string name;
  gui::WBATLASHANDLE icon = 0;
  int32_t buyPrice = 0;
  int32_t sellPrice = 0;
};

export class TPTracker : public gui::CWBGuiType<"tptracker", gui::CWBItem> {
 public:
  TPTracker();
  ~TPTracker() override;
  static inline TPTracker* Create(gui::CWBItem* Parent, math::Rect Position) {
    auto p = std::make_unique<TPTracker>();
    p->Initialize(Parent, Position);
    TPTracker* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* Root, CXMLNode& node,
                               math::Rect& Pos);

  bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                          gui::WBMESSAGE MessageType) override;

 private:
  void OnDraw(gui::CWBDrawAPI* API) override;
  static bool ParseTransaction(jsonxx::Object& object, TransactionItem& output);

  int32_t lastFetchTime = 0;

  std::vector<TransactionItem> buys;
  std::vector<TransactionItem> sells;

  std::mutex transaction_mtx;
  std::future<void> fetchTask;
};
