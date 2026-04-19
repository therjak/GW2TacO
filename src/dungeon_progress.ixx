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
      : name_(name), type_(type), id_(id) {}
  DungeonPath(const DungeonPath& p) : name_(p.name_), type_(p.type_), id_(p.id_) {}

  const std::string_view name_;
  const std::string_view type_;
  const int32_t id_;
  std::atomic<bool> finished_ = false;
  std::atomic<bool> frequenter_ = false;
};

export class Dungeon {
 public:
  const std::string_view name_;
  const std::string_view short_name_;
  std::vector<DungeonPath> paths_;
};

export class DungeonProgress
    : public gui::CWBGuiType<"dungeonprogress", gui::CWBItem> {
 public:
  DungeonProgress();
  static inline DungeonProgress* Create(gui::CWBItem* parent,
                                        math::Rect position) {
    auto p = std::make_unique<DungeonProgress>();
    p->Initialize(parent, position);
    DungeonProgress* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }
  ~DungeonProgress() override;

  static gui::CWBItem* Factory(gui::CWBItem* root, CXMLNode& node,
                               math::Rect& pos);

  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;

 private:
  void OnDraw(gui::CWBDrawAPI* api) override;

  math::Point last_pos_;

  LockFreeQueue<std::unordered_set<std::string>> dungeon_queue_;
  LockFreeQueue<std::unordered_set<int32_t>> dungeon_achievements_queue_;

  std::atomic<bool> being_fetched_ = false;
  int32_t last_fetch_time_ = 0;

  std::vector<Dungeon> dungeons_;
  std::future<void> fetch_task_;
};
