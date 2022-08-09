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
  static inline std::shared_ptr<ClickThroughButton> Create(
      CWBItem* Parent, const math::CRect& Pos, std::string_view txt = "") {
    auto p = std::make_shared<ClickThroughButton>(Parent, Pos, txt);
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~ClickThroughButton() override;

  bool Initialize(CWBItem* Parent, const math::CRect& Position) override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("clickthroughbutton", CWBItem);
};
