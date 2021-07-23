#pragma once

#include "Bedrock/Whiteboard/Window.h"

class OverlayWindow : public CWBWindow {
 public:
  bool MessageProc(CWBMessage& Message) override;
  void OnDraw(CWBDrawAPI* API) override;
  bool IsMouseTransparent(CPoint& ClientSpacePoint,
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

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "OverlayWindow" ), CWBItem);
};
