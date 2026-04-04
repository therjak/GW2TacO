module;
#include <cassert>
#include <memory>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/util/xml_node.h"

export module taco.ts3_control;

import whiteboard;

export class TS3Control : public CWBGuiType<"ts3control", CWBItem> {
 public:
  TS3Control(CWBItem* Parent, math::CRect Position);
  ~TS3Control() override;
  static inline TS3Control* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_unique<TS3Control>(Parent, Position);
    TS3Control* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, math::CRect& Pos);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

 private:
  void OnDraw(CWBDrawAPI* API) override;
  math::CPoint lastpos;
};
