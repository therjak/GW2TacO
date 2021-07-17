#pragma once
#include "Bedrock/WhiteBoard/whiteboard.h"

class GW2RangeDisplay : public CWBItem {
  virtual void OnDraw(CWBDrawAPI *API);
  void DrawRangeCircle(CWBDrawAPI *API, float range, float alpha);

 public:
  GW2RangeDisplay(CWBItem *Parent, CRect Position);
  static inline std::shared_ptr<GW2RangeDisplay> Create(CWBItem *Parent,
                                                        CRect Position) {
    auto p = std::make_shared<GW2RangeDisplay>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  virtual ~GW2RangeDisplay();

  static CWBItem *Factory(CWBItem *Root, CXMLNode &node, CRect &Pos);
  WB_DECLARE_GUIITEM(_T( "gw2rangecircles" ), CWBItem);

  virtual TBOOL IsMouseTransparent(CPoint &ClientSpacePoint,
                                   WBMESSAGE MessageType);
};
