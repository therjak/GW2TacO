module;
#include <cassert>
#include <memory>

export module taco.ts3_control;

import math;
import whiteboard;
import xml;

export class TS3Control : public gui::CWBGuiType<"ts3control", gui::CWBItem> {
 public:
  TS3Control();
  ~TS3Control() override;
  static inline TS3Control* Create(gui::CWBItem* Parent, math::Rect Position) {
    auto p = std::make_unique<TS3Control>();
    p->Initialize(Parent, Position);
    TS3Control* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* Root, CXMLNode& node,
                               math::Rect& Pos);

  bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                          gui::WBMESSAGE MessageType) override;

 private:
  void OnDraw(gui::CWBDrawAPI* API) override;
  math::Point lastpos;
};
