module;
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <vector>
#include <cassert>

#include "src/base/lock_free_queue.h"
#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"
#include "src/util/xml_node.h"

export module taco.raid_progress;

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

export class RaidProgress : public CWBGuiType<"raidprogress", CWBItem> {
 public:
  RaidProgress(CWBItem* Parent, math::CRect Position);
  ~RaidProgress() override;
  static inline RaidProgress* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_unique<RaidProgress>(Parent, Position);
    RaidProgress* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, math::CRect& Pos);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
  std::vector<Raid>& GetRaids();

 private:
  void OnDraw(CWBDrawAPI* API) override;

  math::CPoint lastpos;
  LockFreeQueue<std::unordered_set<std::string>> raid_queue;
  std::atomic<bool> being_fetched = false;
  int32_t lastFetchTime = 0;
  std::vector<Raid> raids;
  std::thread fetchThread;
};
