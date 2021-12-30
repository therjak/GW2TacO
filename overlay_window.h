#pragma once

#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"
#include "src/white_board/window.h"

class OverlayWindow : public CWBWindow {
 public:
  bool MessageProc(const CWBMessage& Message) override;
  void OnDraw(CWBDrawAPI* API) override;
  bool IsMouseTransparent(const CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  OverlayWindow(CWBItem* Parent, CRect Position);
  static inline std::shared_ptr<OverlayWindow> Create(CWBItem* Parent,
                                                      CRect Position) {
    auto p = std::make_shared<OverlayWindow>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }

  ~OverlayWindow() override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM("OverlayWindow", CWBItem);
};
