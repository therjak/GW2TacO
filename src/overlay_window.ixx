module;
#include <cassert>
#include <memory>

export module taco.overlay_window;

import math;
import whiteboard;
import xml;

export class OverlayWindow
    : public gui::CWBGuiType<"OverlayWindow", gui::CWBWindow> {
 public:
  OverlayWindow();
  ~OverlayWindow() override;
  static inline OverlayWindow* Create(gui::CWBItem* parent,
                                      math::Rect position) {
    auto p = std::make_unique<OverlayWindow>();
    p->Initialize(parent, position);
    OverlayWindow* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* root, const CXMLNode& node,
                               math::Rect& pos);

  bool MessageProc(const gui::CWBMessage& message) override;
  void OnDraw(gui::CWBDrawAPI* api) override;
  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;
};
