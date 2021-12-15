#pragma once
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

class GW2TacticalCompass : public CWBItem {
  void OnDraw(CWBDrawAPI* API) override;
  void DrawTacticalCompass(CWBDrawAPI* API);

 public:
  GW2TacticalCompass(CWBItem* Parent, CRect Position);
  static inline std::shared_ptr<GW2TacticalCompass> Create(CWBItem* Parent,
                                                           CRect Position) {
    auto p = std::make_shared<GW2TacticalCompass>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~GW2TacticalCompass() override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "gw2rangecircles" ), CWBItem);

  bool IsMouseTransparent(const CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
};
