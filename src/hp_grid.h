#pragma once

#include <memory>
#include <vector>

#include "src/base/rectangle.h"
#include "src/base/sphere.h"
#include "src/base/vector.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

class GW2HPGrid : public CWBItem {
  math::CPoint lastpos;
  void OnDraw(CWBDrawAPI* API) override;

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

 public:
  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  virtual void LoadGrids();

  GW2HPGrid(CWBItem* Parent, math::CRect Position);
  static inline std::shared_ptr<GW2HPGrid> Create(CWBItem* Parent,
                                                  math::CRect Position) {
    auto p = std::make_shared<GW2HPGrid>(Parent, Position);
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~GW2HPGrid() override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("hpgrid", CWBItem);
};
