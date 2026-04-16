module;
#include <cassert>
#include <memory>

export module taco.range_display;

import math;
import whiteboard;
import xml;

export class GW2RangeDisplay
    : public gui::CWBGuiType<"gw2rangecircles", gui::CWBItem> {
 public:
  GW2RangeDisplay();
  ~GW2RangeDisplay() override;
  static inline GW2RangeDisplay* Create(gui::CWBItem* Parent,
                                        math::Rect Position) {
    auto p = std::make_unique<GW2RangeDisplay>();
    p->Initialize(Parent, Position);
    GW2RangeDisplay* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* Root, const CXMLNode& node,
                               math::Rect& Pos);

  bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                          gui::WBMESSAGE MessageType) override;

 private:
  void OnDraw(gui::CWBDrawAPI* API) override;
  void DrawRangeCircle(gui::CWBDrawAPI* API, float range, float alpha);
};
