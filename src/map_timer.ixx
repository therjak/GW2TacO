module;
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>
#include <unordered_map>
#include <cassert>

#include "src/base/lock_free_queue.h"
#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/white_board/gui_item.h"
#include "src/util/xml_node.h"

export module taco.map_timer;

export class GW2MapTimer : public CWBItem {
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
  GW2MapTimer(CWBItem* Parent, math::CRect Position);
  ~GW2MapTimer() override;
  static inline GW2MapTimer* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_unique<GW2MapTimer>(Parent, Position);
    GW2MapTimer* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("maptimer", CWBItem);

  std::vector<Map> maps;
  std::unordered_map<std::string, Category> categories;

 private:
  bool IsScrollbarVisible();
  void OnResize(const math::CSize& s) override;
  int32_t GetScrollbarStep() override;
  CWBItem* GetItemUnderMouse(math::CPoint& Point, math::CRect& CropRect,
                             WBMESSAGE MessageType) override;
  void OnDraw(CWBDrawAPI* API) override;
  void SetLayout(const CXMLNode& node);
  void UpdateScrollbarData(int ypos, const math::CRect& cl);

  int32_t lastypos = -1;

  LockFreeQueue<std::unordered_set<std::string>> boss_queue;
  LockFreeQueue<std::unordered_set<std::string>> mapchest_queue;

  std::atomic<bool> being_fetched = false;
  int32_t lastFetchTime = 0;

  std::unordered_set<std::string> world_bosses;
  std::unordered_set<std::string> mapchests;

  std::thread fetchThread;
};
