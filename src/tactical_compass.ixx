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
  static inline GW2TacticalCompass* Create(gui::CWBItem* parent,
                                           math::Rect position) {
    auto p = std::make_unique<GW2TacticalCompass>();
    p->Initialize(parent, position);
    GW2TacticalCompass* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* root, const CXMLNode& node,
                               math::Rect& pos);

  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;

 private:
  void OnDraw(gui::CWBDrawAPI* api) override;
  void DrawTacticalCompass(gui::CWBDrawAPI* api);
};
