module;
#include <atomic>
#include <cassert>
#include <future>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/base/color.h"
#include "src/base/lock_free_queue.h"

export module taco.map_timer;

import math;
import whiteboard;
import xml;

export class GW2MapTimer : public gui::CWBGuiType<"maptimer", gui::CWBItem> {
 public:
  struct Event {
    std::string name;
    std::string waypoint;
    std::string world_boss_id;
    int length = 0;
    int start = 0;
    CColor color;
  };

  struct Map {
    std::string name;
    std::string chest_id;
    std::string category;
    int length = 0;
    int start = 0;
    std::string id;
    bool display = true;
    std::vector<Event> events;
  };

  struct Category {
    std::string id;
    std::string name;
    CColor color;
  };

 public:
  GW2MapTimer();
  ~GW2MapTimer() override;
  static inline GW2MapTimer* Create(gui::CWBItem* parent, math::Rect position) {
    auto p = std::make_unique<GW2MapTimer>();
    p->Initialize(parent, position);
    GW2MapTimer* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* root, const CXMLNode& node,
                               math::Rect& pos);

  std::vector<Map> maps_;
  std::unordered_map<std::string, Category> categories_;

 private:
  bool IsScrollbarVisible();
  void OnResize(const math::Size& s) override;
  int32_t GetScrollbarStep() override;
  CWBItem* GetItemUnderMouse(math::Point& point, math::Rect& crop_rect,
                             gui::WBMESSAGE message_type) override;
  void OnDraw(gui::CWBDrawAPI* api) override;
  void SetLayout(const CXMLNode& node);
  void UpdateScrollbarData(int ypos, const math::Rect& cl);

  int32_t last_ypos_ = -1;

  LockFreeQueue<std::unordered_set<std::string>> boss_queue_;
  LockFreeQueue<std::unordered_set<std::string>> mapchest_queue_;

  int32_t last_fetch_time_ = 0;

  std::unordered_set<std::string> world_bosses_;
  std::unordered_set<std::string> mapchests_;

  std::future<void> fetch_task_;
};
