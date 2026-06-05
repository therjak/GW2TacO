module;
#include <cassert>
#include <memory>
#include <utility>

export module taco.overlay_window;

import math;
import whiteboard;
import xml;

export class OverlayWindow : public gui::GuiType<"OverlayWindow", gui::Window> {
 public:
  OverlayWindow();
  ~OverlayWindow() override;
  static inline OverlayWindow* Create(gui::Widget* parent,
                                      math::Rect position) {
    auto p = std::make_unique<OverlayWindow>();
    p->Initialize(parent, position);
    OverlayWindow* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }

  static gui::Widget* Factory(gui::Widget* root, const CXMLNode& node,
                              const math::Rect& pos);

  bool MessageProc(const gui::CWBMessage& message) override;
  void OnDraw(gui::DrawAPI* api) override;
  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;
};
