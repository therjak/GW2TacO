module;
#include <cassert>
#include <memory>

#include "src/base/rectangle.h"
#include "src/base/vector.h"

export module taco.tactical_compass;

import xml;
import whiteboard;

export class GW2TacticalCompass
    : public CWBGuiType<"gw2rangecircles", CWBItem> {
 public:
  GW2TacticalCompass();
  ~GW2TacticalCompass() override;
  static inline GW2TacticalCompass* Create(CWBItem* Parent,
                                           math::CRect Position) {
    auto p = std::make_unique<GW2TacticalCompass>();
    p->Initialize(Parent, Position);
    GW2TacticalCompass* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

 private:
  void OnDraw(CWBDrawAPI* API) override;
  void DrawTacticalCompass(CWBDrawAPI* API);
};
