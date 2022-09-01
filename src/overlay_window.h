#pragma once

#include <memory>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"
#include "src/white_board/window.h"

class OverlayWindow : public CWBWindow {
 public:
  OverlayWindow(CWBItem* Parent, math::CRect Position);
  ~OverlayWindow() override;
  static inline OverlayWindow* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_unique<OverlayWindow>(Parent, Position);
    OverlayWindow* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("OverlayWindow", CWBItem);
  bool MessageProc(const CWBMessage& Message) override;
  void OnDraw(CWBDrawAPI* API) override;
  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
};
