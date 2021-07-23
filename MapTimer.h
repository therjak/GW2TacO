#pragma once
#include <thread>
#include <vector>

#include "Bedrock/Whiteboard/GuiItem.h"

class GW2MapTimer : public CWBItem {
  struct Event {
    std::string name;
    std::string waypoint;
    std::string worldBossId;
    int length;
    int start;
    CColor color;
  };

  struct Map {
    std::string name;
    std::string chestId;
    int Length;
    int Start;
    std::string id;
    bool display = true;
    std::vector<Event> events;
  };

  CPoint lastpos;
  void OnDraw(CWBDrawAPI* API) override;
  void SetLayout(CXMLNode& node);

  bool beingFetched = false;
  int32_t lastFetchTime = 0;

  bool hasWorldBossInfo = false;

  std::thread fetchThread;

  std::vector<std::string> worldBosses;
  std::vector<std::string> mapchests;

  LIGHTWEIGHT_CRITICALSECTION critSec;

 public:
  std::vector<Map> maps;

  GW2MapTimer(CWBItem* Parent, CRect Position);
  static inline std::shared_ptr<GW2MapTimer> Create(CWBItem* Parent,
                                                    CRect Position) {
    auto p = std::make_shared<GW2MapTimer>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~GW2MapTimer() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "maptimer" ), CWBItem);

  bool IsMouseTransparent(CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
};
