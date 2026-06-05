module;
#include <cassert>
#include <memory>
#include <string_view>
#include <utility>

export module taco.special_gui_items;

import math;
import whiteboard;
import xml;

export class ClickThroughButton
    : public gui::GuiType<"clickthroughbutton", gui::Button> {
 public:
  ClickThroughButton(std::string_view txt);
  ~ClickThroughButton() override;
  static inline ClickThroughButton* Create(gui::Widget* parent,
                                           const math::Rect& pos,
                                           std::string_view txt = "") {
    auto p = std::make_unique<ClickThroughButton>(txt);
    p->Initialize(parent, pos);
    ClickThroughButton* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }

  static gui::Widget* Factory(gui::Widget* root, const CXMLNode& node,
                              const math::Rect& pos);
};
