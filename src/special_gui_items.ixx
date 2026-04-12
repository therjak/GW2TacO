module;
#include <cassert>
#include <memory>
#include <string_view>

#include "src/base/rectangle.h"
#include "src/base/vector.h"

export module taco.special_gui_items;

import whiteboard;

export class ClickThroughButton
    : public gui::CWBGuiType<"clickthroughbutton", gui::CWBButton> {
 public:
  ClickThroughButton(std::string_view txt);
  ~ClickThroughButton() override;
  static inline ClickThroughButton* Create(gui::CWBItem* Parent,
                                           const math::CRect& Pos,
                                           std::string_view txt = "") {
    auto p = std::make_unique<ClickThroughButton>(txt);
    p->Initialize(Parent, Pos);
    ClickThroughButton* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* Root, const CXMLNode& node,
                               math::CRect& Pos);
};
