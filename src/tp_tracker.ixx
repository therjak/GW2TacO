module;
#include <atomic>
#include <cassert>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

export module taco.tp_tracker;

import math;
import taco.web;
import whiteboard;
import xml;

export class TPTracker : public gui::GuiType<"tptracker", gui::Widget> {
 public:
  TPTracker();
  ~TPTracker() override;
  static inline TPTracker* Create(gui::Widget* parent, math::Rect position) {
    auto p = std::make_unique<TPTracker>();
    p->Initialize(parent, position);
    TPTracker* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }

  static gui::Widget* Factory(gui::Widget* root, const CXMLNode& node,
                              const math::Rect& pos);

  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;

 private:
  void OnDraw(gui::DrawAPI* api) override;

  int32_t last_fetch_time_ = 0;

  std::vector<TransactionItem> buys_;
  std::vector<TransactionItem> sells_;

  std::mutex transaction_mtx_;
  std::future<void> fetch_task_;
};
