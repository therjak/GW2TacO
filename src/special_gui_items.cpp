module;

#include <string_view>

module taco.special_gui_items;

import whiteboard;

using math::CRect;

ClickThroughButton::ClickThroughButton(std::string_view txt /*=  ""  */)
    : CWBGuiType(txt) {}
ClickThroughButton::~ClickThroughButton() = default;

gui::CWBItem* ClickThroughButton::Factory(gui::CWBItem* Root,
                                          const CXMLNode& node, CRect& Pos) {
  auto button = ClickThroughButton::Create(Root, Pos);
  if (node.HasAttribute("text")) button->SetText(node.GetAttribute("text"));

  if (node.HasAttribute("hidden")) {
    int32_t x = 0;
    node.GetAttributeAsInteger("hidden", &x);
    button->Hide(x);
  }

  return button;
}
