#pragma once
#include <string>
#include <thread>

#include "Bedrock/Whiteboard/draw_api.h"
#include "Bedrock/Whiteboard/gui_item.h"

class RaidEvent {
 public:
  std::string name;
  std::string type;
  bool finished = false;
};

class Wing {
 public:
  std::string name;
  std::vector<RaidEvent> events;
};

class Raid {
 public:
  std::string name;
  std::string shortName;
  std::string configName;
  std::vector<Wing> wings;
};

class RaidProgress : public CWBItem {
  CPoint lastpos;
  void OnDraw(CWBDrawAPI* API) override;

  bool beingFetched = false;
  int32_t lastFetchTime = 0;

  bool hasFullRaidInfo = false;

  std::thread fetchThread;

  std::vector<Raid> raids;

 public:
  RaidProgress(CWBItem* Parent, CRect Position);
  static inline std::shared_ptr<RaidProgress> Create(CWBItem* Parent,
                                                     CRect Position) {
    auto p = std::make_shared<RaidProgress>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~RaidProgress() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "raidprogress" ), CWBItem);

  bool IsMouseTransparent(const CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
  std::vector<Raid>& GetRaids();
};
