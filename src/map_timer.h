﻿#pragma once
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <unordered_set>

#include "src/white_board/gui_item.h"

class GW2MapTimer : public CWBItem {
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
  void SetLayout(CXMLNode& node);
  void UpdateScrollbarData(int ypos, const math::CRect& cl);

  int32_t lastypos = -1;

  bool beingFetched = false;
  int32_t lastFetchTime = 0;

  std::mutex mtx;
  std::unordered_set<std::string> worldBosses;
  std::unordered_set<std::string> mapchests;

  std::thread fetchThread;
};
