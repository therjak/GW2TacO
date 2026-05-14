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

void CWBLabel::OnDraw(CWBDrawAPI* API) {
  DrawBackground(API);

  const WBITEMSTATE i = GetState();
  CWBFont* Font = GetFont(i);
  const auto TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));

  if (Font) {
    const CColor TextColor =
        CSSProperties.DisplayDescriptor.GetColor(i, WB_ITEM_FONTCOLOR);
    const Point TextPos =
        Font->GetTextPosition(Text, GetClientRect(), CSSProperties.TextAlignX,
                              CSSProperties.TextAlignY, TextTransform);
    Font->Write(API, Text, TextPos, TextColor, TextTransform);
  }

  DrawBorder(API);
}

CWBLabel::CWBLabel(std::string_view Txt) : CWBGuiType(), Text(Txt) {}

CWBLabel::~CWBLabel() = default;

bool CWBLabel::Initialize(CWBItem* Parent, const Rect& Position) {
  if (!CWBItem::Initialize(Parent, Position)) return false;
  ContentChanged();
  return true;
}

CWBItem* CWBLabel::Factory(CWBItem* Root, const CXMLNode& node,
                           const Rect& Pos) {
  auto label = CWBLabel::Create(Root, Pos);
  if (node.HasAttribute("text")) label->SetText(node.GetAttribute("text"));
  return label;
}

void CWBLabel::SetText(std::string_view val) {
  Text = val;
  ContentChanged();
}

Size CWBLabel::GetContentSize() {
  const WBITEMSTATE i = GetState();
  CWBFont* Font = GetFont(i);
  const auto TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));

  if (!Font) return Size(0, 0);
  return Size(Font->GetWidth(Text, false, TextTransform),
              Font->GetLineHeight());
}

}  // namespace gui
