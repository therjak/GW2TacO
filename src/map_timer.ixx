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
    std::string worldBossId;
    int length = 0;
    int start = 0;
    CColor color;
  };

  struct Map {
    std::string name;
    std::string chestId;
    std::string category;
    int Length = 0;
    int Start = 0;
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
  static inline GW2MapTimer* Create(gui::CWBItem* Parent, math::Rect Position) {
    auto p = std::make_unique<GW2MapTimer>();
    p->Initialize(Parent, Position);
    GW2MapTimer* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* Root, const CXMLNode& node,
                               math::Rect& Pos);

  std::vector<Map> maps;
  std::unordered_map<std::string, Category> categories;

 private:
  bool IsScrollbarVisible();
  void OnResize(const math::Size& s) override;
  int32_t GetScrollbarStep() override;
  CWBItem* GetItemUnderMouse(math::Point& Point, math::Rect& CropRect,
                             gui::WBMESSAGE MessageType) override;
  void OnDraw(gui::CWBDrawAPI* API) override;
  void SetLayout(const CXMLNode& node);
  void UpdateScrollbarData(int ypos, const math::Rect& cl);

  int32_t lastypos = -1;

  LockFreeQueue<std::unordered_set<std::string>> boss_queue;
  LockFreeQueue<std::unordered_set<std::string>> mapchest_queue;

  int32_t lastFetchTime = 0;

  std::unordered_set<std::string> world_bosses;
  std::unordered_set<std::string> mapchests;

  std::future<void> fetchTask;
};
