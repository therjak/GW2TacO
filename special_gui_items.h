#pragma once
#include <string_view>

#include "src/white_board/button.h"
#include "src/white_board/gui_item.h"

class ClickThroughButton : public CWBButton {
 public:
  ClickThroughButton(CWBItem* Parent, const CRect& Pos, std::string_view txt);
  static inline std::shared_ptr<ClickThroughButton> Create(
      CWBItem* Parent, const CRect& Pos, std::string_view txt = "") {
    auto p = std::make_shared<ClickThroughButton>(Parent, Pos, txt);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~ClickThroughButton() override;

  bool Initialize(CWBItem* Parent, const CRect& Position) override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM("clickthroughbutton", CWBItem);
};
