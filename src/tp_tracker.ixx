module;
#include <atomic>
#include <cassert>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

export module taco.tp_tracker;

import math;
import whiteboard;
import xml;

export struct TransactionItem {
  int32_t transaction_id = 0;
  int32_t item_id = 0;
  int32_t price = 0;
  int32_t quantity = 0;
  std::string created;
};

export struct GW2ItemData {
  int32_t item_id = 0;
  std::string name;
  gui::WBATLASHANDLE icon = 0;
  int32_t buy_price = 0;
  int32_t sell_price = 0;
};

export class TPTracker : public gui::CWBGuiType<"tptracker", gui::CWBItem> {
 public:
  TPTracker();
  ~TPTracker() override;
  static inline TPTracker* Create(gui::CWBItem* parent, math::Rect position) {
    auto p = std::make_unique<TPTracker>();
    p->Initialize(parent, position);
    TPTracker* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* root, CXMLNode& node,
                               math::Rect& pos);

  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;

 private:
  void OnDraw(gui::CWBDrawAPI* api) override;

  int32_t last_fetch_time_ = 0;

  std::vector<TransactionItem> buys_;
  std::vector<TransactionItem> sells_;

  std::mutex transaction_mtx_;
  std::future<void> fetch_task_;
};
