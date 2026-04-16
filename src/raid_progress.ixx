module;
#include <atomic>
#include <cassert>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_set>
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
  std::string_view shortName;
  std::string_view configName;
  std::vector<Wing> wings;
};

export class RaidProgress
    : public gui::CWBGuiType<"raidprogress", gui::CWBItem> {
 public:
  RaidProgress();
  ~RaidProgress() override;
  static inline RaidProgress* Create(gui::CWBItem* Parent,
                                     math::Rect Position) {
    auto p = std::make_unique<RaidProgress>();
    p->Initialize(Parent, Position);
    RaidProgress* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* Root, CXMLNode& node,
                               math::Rect& Pos);

  bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                          gui::WBMESSAGE MessageType) override;
  std::vector<Raid>& GetRaids();

 private:
  void OnDraw(gui::CWBDrawAPI* API) override;

  math::Point last_pos;
  LockFreeQueue<std::unordered_set<std::string>> raid_queue;
  int32_t lastFetchTime = 0;
  std::vector<Raid> raids;
  std::future<void> fetchTask;
};
