module;
#include <cassert>
#include <memory>
#include <string_view>

#include "src/base/rectangle.h"
#include "src/base/vector.h"

export module taco.special_gui_items;

import whiteboard;

export class ClickThroughButton
    : public CWBGuiType<"clickthroughbutton", CWBButton> {
 public:
  ClickThroughButton(CWBItem* Parent, const math::CRect& Pos,
                     std::string_view txt);
  ~ClickThroughButton() override;
  static inline ClickThroughButton* Create(CWBItem* Parent,
                                           const math::CRect& Pos,
                                           std::string_view txt = "") {
    auto p = std::make_unique<ClickThroughButton>(Parent, Pos, txt);
    ClickThroughButton* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  bool Initialize(CWBItem* Parent, const math::CRect& Position) override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
};
