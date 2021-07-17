#pragma once
#include <vector>

#include "Bedrock/Whiteboard/WhiteBoard.h"
#include "gw2tactical.h"

class GW2MarkerEditor : public CWBItem {
  TBOOL MessageProc(CWBMessage &Message) override;
  void OnDraw(CWBDrawAPI *API) override;
  TBOOL Hidden = false;
  GUID CurrentPOI;

  std::vector<GW2TacticalCategory *> CategoryList;
  TBOOL ChangeDefault = false;

 public:
  TBOOL IsMouseTransparent(CPoint &ClientSpacePoint,
                           WBMESSAGE MessageType) override;

  GW2MarkerEditor(CWBItem *Parent, CRect Position);
  static inline std::shared_ptr<GW2MarkerEditor> Create(CWBItem *Parent,
                                                        CRect Position) {
    auto p = std::make_shared<GW2MarkerEditor>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~GW2MarkerEditor() override;

  static CWBItem *Factory(CWBItem *Root, CXMLNode &node, CRect &Pos);
  WB_DECLARE_GUIITEM(_T( "markereditor" ), CWBItem);
};
