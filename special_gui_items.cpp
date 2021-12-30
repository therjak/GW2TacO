#include "special_gui_items.h"

#include "src/white_board/button.h"

ClickThroughButton::ClickThroughButton(CWBItem* Parent, const CRect& Pos,
                                       std::string_view txt /*=  ""  */)
    : CWBButton(Parent, Pos, txt) {}

ClickThroughButton::~ClickThroughButton() = default;

bool ClickThroughButton::Initialize(CWBItem* Parent, const CRect& Position) {
  return CWBButton::Initialize(Parent, Position);
}

CWBItem* ClickThroughButton::Factory(CWBItem* Root, const CXMLNode& node,
                                     CRect& Pos) {
  auto button = ClickThroughButton::Create(Root, Pos);
  if (node.HasAttribute("text")) button->SetText(node.GetAttribute("text"));

  if (node.HasAttribute("hidden")) {
    int32_t x = 0;
    node.GetAttributeAsInteger("hidden", &x);
    button->Hide(x);
  }

  return button.get();
}
