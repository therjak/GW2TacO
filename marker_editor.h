#pragma once
#include <vector>

#include "Bedrock/Whiteboard/draw_api.h"
#include "Bedrock/Whiteboard/gui_item.h"
#include "gw2_tactical.h"

class GW2MarkerEditor : public CWBItem {
  bool MessageProc(const CWBMessage& Message) override;
  void OnDraw(CWBDrawAPI* API) override;
  bool Hidden = false;
  GUID CurrentPOI;

  std::vector<GW2TacticalCategory*> CategoryList;
  bool ChangeDefault = false;

 public:
  bool IsMouseTransparent(const CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  GW2MarkerEditor(CWBItem* Parent, CRect Position);
  static inline std::shared_ptr<GW2MarkerEditor> Create(CWBItem* Parent,
                                                        CRect Position) {
    auto p = std::make_shared<GW2MarkerEditor>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~GW2MarkerEditor() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "markereditor" ), CWBItem);
};
