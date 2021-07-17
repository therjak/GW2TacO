﻿#include "SpecialGUIItems.h"

ClickThroughButton::ClickThroughButton(CWBItem *Parent, const CRect &Pos,
                                       std::string_view txt /*= _T( "" ) */)
    : CWBButton(Parent, Pos, txt) {}

ClickThroughButton::~ClickThroughButton() {}

TBOOL ClickThroughButton::Initialize(CWBItem *Parent, const CRect &Position,
                                     std::string_view txt /*= _T( "" ) */) {
  return CWBButton::Initialize(Parent, Position, txt);
}

CWBItem *ClickThroughButton::Factory(CWBItem *Root, CXMLNode &node,
                                     CRect &Pos) {
  auto button = ClickThroughButton::Create(Root, Pos);
  if (node.HasAttribute(_T( "text" )))
    button->SetText(node.GetAttribute(_T( "text" )));

  if (node.HasAttribute(_T( "hidden" ))) {
    int32_t x = 0;
    node.GetAttributeAsInteger(_T( "hidden" ), &x);
    button->Hide(x);
  }

  return button.get();
}
