module;
#include <cassert>
#include <memory>

export module taco.tactical_compass;

import math;
import whiteboard;
import xml;

export class GW2TacticalCompass
    : public gui::CWBGuiType<"gw2rangecircles", gui::CWBItem> {
 public:
  GW2TacticalCompass();
  ~GW2TacticalCompass() override;
  static inline GW2TacticalCompass* Create(gui::CWBItem* Parent,
                                           math::Rect Position) {
    auto p = std::make_unique<GW2TacticalCompass>();
    p->Initialize(Parent, Position);
    GW2TacticalCompass* r = p.get();
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
  void DrawTacticalCompass(gui::CWBDrawAPI* API);
};
