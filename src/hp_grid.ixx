module;

#include <cassert>
#include <memory>
#include <vector>

#include "src/base/rectangle.h"
#include "src/base/sphere.h"
#include "src/base/vector.h"
#include "src/util/xml_node.h"

export module taco.hp_grid;

import whiteboard;

export class GW2HPGrid : public CWBGuiType<"hpgrid", CWBItem> {
 public:
  GW2HPGrid(CWBItem* Parent, math::CRect Position);
  static inline GW2HPGrid* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_unique<GW2HPGrid>(Parent, Position);
    GW2HPGrid* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~GW2HPGrid() override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  virtual void LoadGrids();

 private:
  void OnDraw(CWBDrawAPI* API) override;

  math::CPoint lastpos;

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
