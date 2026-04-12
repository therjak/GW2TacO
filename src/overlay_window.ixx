module;
#include <cassert>
#include <memory>

#include "src/base/rectangle.h"
#include "src/base/vector.h"

export module taco.overlay_window;

import whiteboard;

export class OverlayWindow
    : public gui::CWBGuiType<"OverlayWindow", gui::CWBWindow> {
 public:
  OverlayWindow();
  ~OverlayWindow() override;
  static inline OverlayWindow* Create(gui::CWBItem* Parent,
                                      math::CRect Position) {
    auto p = std::make_unique<OverlayWindow>();
    p->Initialize(Parent, Position);
    OverlayWindow* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* Root, const CXMLNode& node,
                               math::CRect& Pos);

  bool MessageProc(const gui::CWBMessage& Message) override;
  void OnDraw(gui::CWBDrawAPI* API) override;
  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          gui::WBMESSAGE MessageType) override;
};
