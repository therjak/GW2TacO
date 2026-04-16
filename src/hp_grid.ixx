module;

#include <cassert>
#include <memory>
#include <vector>

export module taco.hp_grid;

import math;
import whiteboard;
import xml;

export class GW2HPGrid : public gui::CWBGuiType<"hpgrid", gui::CWBItem> {
 public:
  GW2HPGrid();
  static inline GW2HPGrid* Create(gui::CWBItem* Parent, math::Rect Position) {
    auto p = std::make_unique<GW2HPGrid>();
    p->Initialize(Parent, Position);
    GW2HPGrid* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~GW2HPGrid() override;

  static gui::CWBItem* Factory(gui::CWBItem* Root, const CXMLNode& node,
                               math::Rect& Pos);

  bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                          gui::WBMESSAGE MessageType) override;

  virtual void LoadGrids();

 private:
  void OnDraw(gui::CWBDrawAPI* API) override;

  math::Point lastpos;

  struct GridLine {
    float percentage = 0;
    CColor color;
  };

  struct GridData {
    int mapID = 0;
    math::CSphere bSphere;
    std::vector<GridLine> displayedPercentages;
  };

  std::vector<GridData> Grids;
};
