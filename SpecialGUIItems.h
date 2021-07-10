#pragma once
#include <string_view>

#include "Bedrock/WhiteBoard/WhiteBoard.h"

class ClickThroughButton : public CWBButton {
 public:
  ClickThroughButton(CWBItem *Parent, const CRect &Pos,
                     std::string_view txt = _T( "" ));
  virtual ~ClickThroughButton();

  virtual TBOOL Initialize(CWBItem *Parent, const CRect &Position,
                           std::string_view txt = _T( "" ));

  static CWBItem *Factory(CWBItem *Root, CXMLNode &node, CRect &Pos);
  WB_DECLARE_GUIITEM(_T( "clickthroughbutton" ), CWBItem);
};
