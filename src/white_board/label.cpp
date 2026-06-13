module;

#include <cassert>
#include <string>
#include <string_view>

#include "src/base/color.h"

module whiteboard;

import :application;
import :font;
import :label;

using math::Point;
using math::Rect;
using math::Size;

namespace gui {

void Label::OnDraw(DrawAPI* API) {
  DrawBackground(API);

  const ItemState i = GetState();
  Font* Font = GetFont(i);
  const auto text_transform = static_cast<TextTransform>(
      GetDisplayDescriptor().GetValue(i, WB_ITEM_TEXTTRANSFORM));

  if (Font) {
    const CColor TextColor =
        GetDisplayDescriptor().GetColor(i, WB_ITEM_FONTCOLOR);
    const Point TextPos = Font->GetTextPosition(
        text_, GetClientRect(), GetCSSProperties().text_align_x_,
        GetCSSProperties().text_align_y_, text_transform);
    Font->Write(API, text_, TextPos, TextColor, text_transform);
  }

  DrawBorder(API);
}

Label::Label(std::string_view Txt) : GuiType(), text_(Txt) {}

Label::~Label() = default;

bool Label::Initialize(Widget* Parent, const Rect& Position) {
  if (!Widget::Initialize(Parent, Position)) return false;
  ContentChanged();
  return true;
}

Widget* Label::Factory(Widget* Root, const XMLNode& node, const Rect& Pos) {
  auto label = Label::Create(Root, Pos);
  if (node.HasAttribute("text")) label->SetText(node.GetAttribute("text"));
  return label;
}

void Label::SetText(std::string_view val) {
  text_ = val;
  ContentChanged();
}

Size Label::GetContentSize() {
  const ItemState i = GetState();
  Font* Font = GetFont(i);
  const auto text_transform = static_cast<TextTransform>(
      GetDisplayDescriptor().GetValue(i, WB_ITEM_TEXTTRANSFORM));

  if (!Font) return Size(0, 0);
  return Size(Font->GetWidth(text_, false, text_transform),
              Font->GetLineHeight());
}

}  // namespace gui
