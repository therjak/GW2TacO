#include "src/white_board/label.h"

using math::CPoint;
using math::CRect;
using math::CSize;

void CWBLabel::OnDraw(CWBDrawAPI* API) {
  DrawBackground(API);

  const WBITEMSTATE i = GetState();
  CWBFont* Font = GetFont(i);
  const WBTEXTTRANSFORM TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));

  if (Font) {
    const CColor TextColor =
        CSSProperties.DisplayDescriptor.GetColor(i, WB_ITEM_FONTCOLOR);
    const CPoint TextPos =
        Font->GetTextPosition(Text, GetClientRect(), CSSProperties.TextAlignX,
                              CSSProperties.TextAlignY, TextTransform);
    Font->Write(API, Text, TextPos, TextColor, TextTransform);
  }

  DrawBorder(API);
}

CWBLabel::CWBLabel(CWBItem* Parent, const CRect& Pos, std::string_view Txt)
    : CWBItem(), Text(Txt) {
  Initialize(Parent, Pos);
}

CWBLabel::~CWBLabel() = default;

bool CWBLabel::Initialize(CWBItem* Parent, const CRect& Position) {
  if (!CWBItem::Initialize(Parent, Position)) return false;
  ContentChanged();
  return true;
}

CWBItem* CWBLabel::Factory(CWBItem* Root, const CXMLNode& node, CRect& Pos) {
  auto label = CWBLabel::Create(Root, Pos);
  if (node.HasAttribute("text")) label->SetText(node.GetAttribute("text"));
  return label.get();
}

void CWBLabel::SetText(std::string_view val) {
  Text = val;
  ContentChanged();
}

CSize CWBLabel::GetContentSize() {
  const WBITEMSTATE i = GetState();
  CWBFont* Font = GetFont(i);
  const WBTEXTTRANSFORM TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));

  if (!Font) return CSize(0, 0);
  return CSize(Font->GetWidth(Text, false, TextTransform),
               Font->GetLineHeight());
}
