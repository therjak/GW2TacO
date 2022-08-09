#pragma once

#include <memory>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"
#include "src/white_board/window.h"

class OverlayWindow : public CWBWindow {
 public:
  bool MessageProc(const CWBMessage& Message) override;
  void OnDraw(CWBDrawAPI* API) override;
  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  OverlayWindow(CWBItem* Parent, math::CRect Position);
  static inline std::shared_ptr<OverlayWindow> Create(CWBItem* Parent,
                                                      math::CRect Position) {
    auto p = std::make_shared<OverlayWindow>(Parent, Position);
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }

  ~OverlayWindow() override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("OverlayWindow", CWBItem);
};
