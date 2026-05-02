module;

#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include "src/base/color.h"

export module taco.hp_grid;

import math;
import whiteboard;
import xml;

export class GW2HPGrid : public gui::CWBGuiType<"hpgrid", gui::CWBItem> {
 public:
  GW2HPGrid();
  static inline GW2HPGrid* Create(gui::CWBItem* parent, math::Rect position) {
    auto p = std::make_unique<GW2HPGrid>();
    p->Initialize(parent, position);
    GW2HPGrid* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }
  ~GW2HPGrid() override;

  static gui::CWBItem* Factory(gui::CWBItem* root, const CXMLNode& node,
                               math::Rect& pos);

  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;

  virtual void LoadGrids();

 private:
  void OnDraw(gui::CWBDrawAPI* api) override;

  math::Point last_pos_;

  struct GridLine {
    float percentage = 0;
    CColor color;
  };

  struct GridData {
    int map_id = 0;
    math::CSphere b_sphere;
    std::vector<GridLine> displayed_percentages;
  };

  std::vector<GridData> grids_;
};
