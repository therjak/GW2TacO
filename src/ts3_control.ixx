module;
#include <cassert>
#include <memory>

export module taco.ts3_control;

import math;
import whiteboard;
import xml;

export class Ts3Control : public gui::CWBGuiType<"ts3control", gui::CWBItem> {
 public:
  Ts3Control();
  ~Ts3Control() override;
  static inline Ts3Control* Create(gui::CWBItem* parent, math::Rect position) {
    auto p = std::make_unique<Ts3Control>();
    p->Initialize(parent, position);
    Ts3Control* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* root, CXMLNode& node,
                               math::Rect& pos);

  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;

 private:
  void OnDraw(gui::CWBDrawAPI* api) override;
};
