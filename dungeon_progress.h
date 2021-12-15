#pragma once
#include <string>
#include <thread>
#include <vector>

#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

class DungeonPath {
 public:
  std::string name;
  std::string type;
  bool finished = false;
  bool frequenter = false;
};

class Dungeon {
 public:
  std::string name;
  std::string shortName;
  std::vector<DungeonPath> paths;
};

class DungeonProgress : public CWBItem {
  CPoint lastpos;
  void OnDraw(CWBDrawAPI* API) override;

  bool beingFetched = false;
  int32_t lastFetchTime = 0;

  bool hasFullDungeonInfo = false;

  std::thread fetchThread;

  std::vector<Dungeon> dungeons;

 public:
  DungeonProgress(CWBItem* Parent, CRect Position);
  static inline std::shared_ptr<DungeonProgress> Create(CWBItem* Parent,
                                                        CRect Position) {
    auto p = std::make_shared<DungeonProgress>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~DungeonProgress() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "dungeonprogress" ), CWBItem);

  bool IsMouseTransparent(const CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
};
