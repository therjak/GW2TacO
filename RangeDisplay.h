#pragma once
#include "Bedrock/Whiteboard/DrawAPI.h"
#include "Bedrock/Whiteboard/GuiItem.h"

class GW2RangeDisplay : public CWBItem {
  void OnDraw(CWBDrawAPI* API) override;
  void DrawRangeCircle(CWBDrawAPI* API, float range, float alpha);

 public:
  GW2RangeDisplay(CWBItem* Parent, CRect Position);
  static inline std::shared_ptr<GW2RangeDisplay> Create(CWBItem* Parent,
                                                        CRect Position) {
    auto p = std::make_shared<GW2RangeDisplay>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~GW2RangeDisplay() override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "gw2rangecircles" ), CWBItem);

  bool IsMouseTransparent(const CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
};
