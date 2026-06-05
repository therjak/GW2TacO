module;

#include <string_view>

module taco.special_gui_items;

import whiteboard;

using math::Rect;

ClickThroughButton::ClickThroughButton(std::string_view txt /*=  ""  */)
    : GuiType(txt) {}
ClickThroughButton::~ClickThroughButton() = default;

gui::Widget* ClickThroughButton::Factory(gui::Widget* root,
                                         const CXMLNode& node,
                                         const Rect& pos) {
  auto button = ClickThroughButton::Create(root, pos);
  if (node.HasAttribute("text")) button->SetText(node.GetAttribute("text"));

  if (node.HasAttribute("hidden")) {
    int32_t x = 0;
    node.GetAttributeAsInteger("hidden", &x);
    button->Hide(x);
  }

  return button;
}
