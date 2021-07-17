#pragma once

#include <vector>

#include "Bedrock/WhiteBoard/whiteboard.h"

class GW2HPGrid : public CWBItem {
  CPoint lastpos;
  virtual void OnDraw(CWBDrawAPI *API);

  struct GridLine {
    float percentage = 0;
    CColor color;
  };

  struct GridData {
    int mapID = 0;
    CSphere bSphere;
    std::vector<GridLine> displayedPercentages;
  };

  std::vector<GridData> Grids;

 public:
  virtual TBOOL IsMouseTransparent(CPoint &ClientSpacePoint,
                                   WBMESSAGE MessageType);

  virtual void LoadGrids();

  GW2HPGrid(CWBItem *Parent, CRect Position);
  static inline std::shared_ptr<GW2HPGrid> Create(CWBItem *Parent,
                                                  CRect Position) {
    auto p = std::make_shared<GW2HPGrid>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  virtual ~GW2HPGrid();

  static CWBItem *Factory(CWBItem *Root, CXMLNode &node, CRect &Pos);
  WB_DECLARE_GUIITEM(_T( "hpgrid" ), CWBItem);
};
