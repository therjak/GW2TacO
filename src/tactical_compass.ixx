module;
#include <cassert>
#include <memory>

#include "src/base/rectangle.h"
#include "src/base/vector.h"

export module taco.tactical_compass;

import xml;
import whiteboard;

export class GW2TacticalCompass
    : public gui::CWBGuiType<"gw2rangecircles", gui::CWBItem> {
 public:
  GW2TacticalCompass();
  ~GW2TacticalCompass() override;
  static inline GW2TacticalCompass* Create(gui::CWBItem* Parent,
                                           math::CRect Position) {
    auto p = std::make_unique<GW2TacticalCompass>();
    p->Initialize(Parent, Position);
    GW2TacticalCompass* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* Root, const CXMLNode& node,
                               math::CRect& Pos);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          gui::WBMESSAGE MessageType) override;

 private:
  void OnDraw(gui::CWBDrawAPI* API) override;
  void DrawTacticalCompass(gui::CWBDrawAPI* API);
};
