#pragma once
#include <memory>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

class GW2RangeDisplay : public CWBItem {
  void OnDraw(CWBDrawAPI* API) override;
  void DrawRangeCircle(CWBDrawAPI* API, float range, float alpha);

 public:
  GW2RangeDisplay(CWBItem* Parent, math::CRect Position);
  static inline GW2RangeDisplay* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_shared<GW2RangeDisplay>(Parent, Position);
    GW2RangeDisplay* r = p.get();
    assert(Parent);
    Parent->AddChild(p);
    return r;
  }
  ~GW2RangeDisplay() override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("gw2rangecircles", CWBItem);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
};
