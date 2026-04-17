module;

#include <string_view>

module taco.special_gui_items;

import whiteboard;

using math::Rect;

ClickThroughButton::ClickThroughButton(std::string_view txt /*=  ""  */)
    : CWBGuiType(txt) {}
ClickThroughButton::~ClickThroughButton() = default;

gui::CWBItem* ClickThroughButton::Factory(gui::CWBItem* root,
                                          const CXMLNode& node, Rect& pos) {
  auto button = ClickThroughButton::Create(root, pos);
  if (node.HasAttribute("text")) button->SetText(node.GetAttribute("text"));

  if (node.HasAttribute("hidden")) {
    int32_t x = 0;
    node.GetAttributeAsInteger("hidden", &x);
    button->Hide(x);
  }

  return button;
}
