#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

class RaidEvent {
 public:
  enum class Type : char { Boss, Checkpoint };
  RaidEvent(std::string_view name, Type type)
      : name(name), type(std::move(type)) {}
  RaidEvent(const RaidEvent& e) : name(e.name), type(e.type) {}
  std::string_view name;
  Type type;
  std::atomic<bool> finished = false;
};

class Wing {
 public:
  std::string_view name;
  std::vector<RaidEvent> events;
};

class Raid {
 public:
  std::string_view name;
  std::string_view shortName;
  std::string_view configName;
  std::vector<Wing> wings;
};

class RaidProgress : public CWBItem {
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
  WB_DECLARE_GUIITEM("raidprogress", CWBItem);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
  std::vector<Raid>& GetRaids();

 private:
  void OnDraw(CWBDrawAPI* API) override;

  math::CPoint lastpos;
  bool beingFetched = false;
  int32_t lastFetchTime = 0;
  std::vector<Raid> raids;
  std::thread fetchThread;
};
