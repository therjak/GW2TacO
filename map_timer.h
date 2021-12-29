#pragma once
#include <mutex>
#include <thread>
#include <vector>

#include "src/white_board/gui_item.h"

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
    std::string category;
    int Length;
    int Start;
    std::string id;
    bool display = true;
    std::vector<Event> events;
  };

  struct Category {
    std::string id;
    std::string name;
    CColor color;
  };

  CPoint lastpos;
  int32_t lastypos = -1;

  bool beingFetched = false;
  int32_t lastFetchTime = 0;

  bool hasWorldBossInfo = false;

  std::thread fetchThread;

  std::vector<std::string> worldBosses;
  std::vector<std::string> mapchests;

  std::mutex mtx;

  bool IsScrollbarVisible();
  void OnResize(const CSize& s) override;
  int32_t GetScrollbarStep() override;
  CWBItem* GetItemUnderMouse(CPoint& Point, CRect& CropRect,
                             WBMESSAGE MessageType) override;
  void OnDraw(CWBDrawAPI* API) override;
  void SetLayout(CXMLNode& node);
  void UpdateScrollbarData(int ypos, const CRect& cl);

 public:
  std::vector<Map> maps;
  std::unordered_map<std::string, Category> categories;

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

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "maptimer" ), CWBItem);
};
