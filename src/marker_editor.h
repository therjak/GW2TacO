#pragma once
#include <vector>

#include "src/gw2_tactical.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

class GW2MarkerEditor : public CWBItem {
  bool MessageProc(const CWBMessage& Message) override;
  void OnDraw(CWBDrawAPI* API) override;
  bool Hidden = false;
  GUID CurrentPOI;

  std::vector<GW2TacticalCategory*> CategoryList;
  bool ChangeDefault = false;

 public:
  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  GW2MarkerEditor(CWBItem* Parent, math::CRect Position);
  static inline std::shared_ptr<GW2MarkerEditor> Create(CWBItem* Parent,
                                                        math::CRect Position) {
    auto p = std::make_shared<GW2MarkerEditor>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~GW2MarkerEditor() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, math::CRect& Pos);
  WB_DECLARE_GUIITEM("markereditor", CWBItem);
};
