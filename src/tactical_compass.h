#pragma once
#include <memory>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

class GW2TacticalCompass : public CWBItem {
 public:
  GW2TacticalCompass(CWBItem* Parent, math::CRect Position);
  ~GW2TacticalCompass() override;
  static inline GW2TacticalCompass* Create(CWBItem* Parent,
                                           math::CRect Position) {
    auto p = std::make_unique<GW2TacticalCompass>(Parent, Position);
    GW2TacticalCompass* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("gw2rangecircles", CWBItem);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

 private:
  void OnDraw(CWBDrawAPI* API) override;
  void DrawTacticalCompass(CWBDrawAPI* API);
};
