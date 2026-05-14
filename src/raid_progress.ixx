module;
#include <atomic>
#include <cassert>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/base/lock_free_queue.h"

export module taco.raid_progress;

import math;
import whiteboard;
import xml;

export class RaidEvent {
 public:
  enum class Type : char { Boss, Checkpoint };
  RaidEvent(std::string_view name, Type type)
      : name(name), type(std::move(type)) {}
  RaidEvent(const RaidEvent& e) : name(e.name), type(e.type) {}
  std::string_view name;
  Type type;
  std::atomic<bool> finished = false;
};

export class Wing {
 public:
  std::string_view name;
  std::vector<RaidEvent> events;
};

export class Raid {
 public:
  std::string_view name;
  std::string_view short_name;
  std::string_view config_name;
  std::vector<Wing> wings;
};

export class RaidProgress
    : public gui::CWBGuiType<"raidprogress", gui::CWBItem> {
 public:
  RaidProgress();
  ~RaidProgress() override;
  static inline RaidProgress* Create(gui::CWBItem* parent,
                                     math::Rect position) {
    auto p = std::make_unique<RaidProgress>();
    p->Initialize(parent, position);
    RaidProgress* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* root, const CXMLNode& node,
                               const math::Rect& pos);

  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;
  std::vector<Raid>& GetRaids();

 private:
  void OnDraw(gui::CWBDrawAPI* api) override;

  math::Point last_pos_;
  LockFreeQueue<std::unordered_set<std::string>> raid_queue;
  int32_t last_fetch_time_ = 0;
  std::vector<Raid> raids_;
  std::future<void> fetch_task_;
};
