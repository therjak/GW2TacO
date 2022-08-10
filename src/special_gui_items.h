#pragma once
#include <memory>
#include <string_view>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/white_board/button.h"
#include "src/white_board/gui_item.h"

class ClickThroughButton : public CWBButton {
 public:
  ClickThroughButton(CWBItem* Parent, const math::CRect& Pos,
                     std::string_view txt);
  static inline ClickThroughButton* Create(CWBItem* Parent,
                                           const math::CRect& Pos,
                                           std::string_view txt = "") {
    auto p = std::make_unique<ClickThroughButton>(Parent, Pos, txt);
    ClickThroughButton* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~ClickThroughButton() override;

  bool Initialize(CWBItem* Parent, const math::CRect& Position) override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("clickthroughbutton", CWBItem);
};
