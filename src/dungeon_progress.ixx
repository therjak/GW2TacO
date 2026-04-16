module;
#include <atomic>
#include <cassert>
#include <future>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "src/base/lock_free_queue.h"

export module taco.dungeon_progress;

import math;
import whiteboard;
import xml;

export class DungeonPath {
 public:
  DungeonPath(const std::string_view& name, const std::string_view& type,
              int32_t id)
      : name(name), type(type), id(id) {}
  DungeonPath(const DungeonPath& p) : name(p.name), type(p.type), id(p.id) {}

  const std::string_view name;
  const std::string_view type;
  const int32_t id;
  std::atomic<bool> finished = false;
  std::atomic<bool> frequenter = false;
};

export class Dungeon {
 public:
  const std::string_view name;
  const std::string_view shortName;
  std::vector<DungeonPath> paths;
};

export class DungeonProgress
    : public gui::CWBGuiType<"dungeonprogress", gui::CWBItem> {
 public:
  DungeonProgress();
  static inline DungeonProgress* Create(gui::CWBItem* Parent,
                                        math::Rect Position) {
    auto p = std::make_unique<DungeonProgress>();
    p->Initialize(Parent, Position);
    DungeonProgress* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~DungeonProgress() override;

  static gui::CWBItem* Factory(gui::CWBItem* Root, CXMLNode& node,
                               math::Rect& Pos);

  bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                          gui::WBMESSAGE MessageType) override;

 private:
  void OnDraw(gui::CWBDrawAPI* API) override;

  math::Point lastpos;

  LockFreeQueue<std::unordered_set<std::string>> dungeon_queue;
  LockFreeQueue<std::unordered_set<int32_t>> dungeon_achievements_queue;

  std::atomic<bool> being_fetched = false;
  int32_t lastFetchTime = 0;

  std::vector<Dungeon> dungeons;
  std::future<void> fetchTask;
};
