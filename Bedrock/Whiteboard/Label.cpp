#include "Label.h"

void CWBLabel::OnDraw(CWBDrawAPI *API) {
  DrawBackground(API);

  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont(i);
  WBTEXTTRANSFORM TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));

  if (Font) {
    CColor TextColor =
        CSSProperties.DisplayDescriptor.GetColor(i, WB_ITEM_FONTCOLOR);
    CPoint TextPos =
        Font->GetTextPosition(Text, GetClientRect(), CSSProperties.TextAlignX,
                              CSSProperties.TextAlignY, TextTransform);
    Font->Write(API, Text, TextPos, TextColor, TextTransform);
  }

  DrawBorder(API);
}

CWBLabel::CWBLabel(CWBItem *Parent, const CRect &Pos, std::string_view Txt)
    : CWBItem() {
  Initialize(Parent, Pos, Txt);
}

CWBLabel::~CWBLabel() = default;

TBOOL CWBLabel::Initialize(CWBItem *Parent, const CRect &Position,
                           std::string_view Txt) {
  Text = Txt;

  if (!CWBItem::Initialize(Parent, Position)) return false;
  ContentChanged();
  return true;
}

CWBItem *CWBLabel::Factory(CWBItem *Root, CXMLNode &node, CRect &Pos) {
  auto label = CWBLabel::Create(Root, Pos);
  if (node.HasAttribute(_T( "text" )))
    label->SetText(node.GetAttribute(_T( "text" )));
  return label.get();
}

void CWBLabel::SetText(std::string_view val) {
  Text = val;
  ContentChanged();
}

CSize CWBLabel::GetContentSize() {
  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont(i);
  WBTEXTTRANSFORM TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));

  if (!Font) return CSize(0, 0);
  return CSize(Font->GetWidth(Text, false, TextTransform),
               Font->GetLineHeight());
}
