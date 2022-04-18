#pragma once
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
  RaidEvent(std::string name, Type type)
      : name(std::move(name)), type(std::move(type)) {}
  RaidEvent(const RaidEvent& e) : name(e.name), type(e.type) {}
  std::string_view name;
  Type type;
  std::mutex mtx;
  bool finished = false;
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
  math::CPoint lastpos;
  void OnDraw(CWBDrawAPI* API) override;

  bool beingFetched = false;
  int32_t lastFetchTime = 0;

  std::thread fetchThread;

  std::vector<Raid> raids;

 public:
  RaidProgress(CWBItem* Parent, math::CRect Position);
  static inline std::shared_ptr<RaidProgress> Create(CWBItem* Parent,
                                                     math::CRect Position) {
    auto p = std::make_shared<RaidProgress>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~RaidProgress() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, math::CRect& Pos);
  WB_DECLARE_GUIITEM("raidprogress", CWBItem);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
  std::vector<Raid>& GetRaids();
};
