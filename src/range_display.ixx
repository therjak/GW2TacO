module;
#include <cassert>
#include <memory>
#include <utility>

export module taco.range_display;

import math;
import whiteboard;
import xml;

export class GW2RangeDisplay
    : public gui::GuiType<"gw2rangecircles", gui::Widget> {
 public:
  GW2RangeDisplay();
  ~GW2RangeDisplay() override;
  static inline GW2RangeDisplay* Create(gui::Widget* parent,
                                        math::Rect position) {
    auto p = std::make_unique<GW2RangeDisplay>();
    p->Initialize(parent, position);
    GW2RangeDisplay* r = p.get();
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
  void DrawRangeCircle(gui::DrawAPI* api, float range, float alpha);
};
