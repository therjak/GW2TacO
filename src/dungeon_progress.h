#pragma once
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

class DungeonPath {
 public:
  DungeonPath(std::string name, std::string type, int32_t id)
      : name(std::move(name)), type(std::move(type)), id(id) {}
  DungeonPath(const DungeonPath& p) : name(p.name), type(p.type), id(p.id) {}
  const std::string_view name;
  const std::string_view type;
  const int32_t id;
  std::mutex mtx;
  bool finished = false;
  bool frequenter = false;
};

class Dungeon {
 public:
  const std::string_view name;
  const std::string_view shortName;
  std::vector<DungeonPath> paths;
};

class DungeonProgress : public CWBItem {
  math::CPoint lastpos;
  void OnDraw(CWBDrawAPI* API) override;

  bool beingFetched = false;
  int32_t lastFetchTime = 0;

  std::thread fetchThread;

  std::vector<Dungeon> dungeons;

 public:
  DungeonProgress(CWBItem* Parent, math::CRect Position);
  static inline DungeonProgress* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_shared<DungeonProgress>(Parent, Position);
    DungeonProgress* r = p.get();
    assert(Parent);
    Parent->AddChild(p);
    return r;
  }
  ~DungeonProgress() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, math::CRect& Pos);
  WB_DECLARE_GUIITEM("dungeonprogress", CWBItem);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
};
