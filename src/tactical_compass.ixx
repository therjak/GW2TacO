module;
#include <cassert>
#include <memory>
#include <utility>

export module taco.tactical_compass;

import math;
import whiteboard;
import xml;

export class GW2TacticalCompass
    : public gui::GuiType<"gw2rangecircles", gui::Widget> {
 public:
  GW2TacticalCompass();
  ~GW2TacticalCompass() override;
  static inline GW2TacticalCompass* Create(gui::Widget* parent,
                                           math::Rect position) {
    auto p = std::make_unique<GW2TacticalCompass>();
    p->Initialize(parent, position);
    GW2TacticalCompass* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }

  static gui::Widget* Factory(gui::Widget* root, const CXMLNode& node,
                              const math::Rect& pos);

  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;

 private:
  void OnDraw(gui::DrawAPI* api) override;
  void DrawTacticalCompass(gui::DrawAPI* api);
};
