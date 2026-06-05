module;
#include <cassert>
#include <memory>
#include <utility>

export module taco.ts3_control;

import math;
import whiteboard;
import xml;

export class Ts3Control : public gui::GuiType<"ts3control", gui::Widget> {
 public:
  Ts3Control();
  ~Ts3Control() override;
  static inline Ts3Control* Create(gui::Widget* parent, math::Rect position) {
    auto p = std::make_unique<Ts3Control>();
    p->Initialize(parent, position);
    Ts3Control* r = p.get();
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
};
