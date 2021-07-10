#pragma once

#include <string>
#include <string_view>

#include "Application.h"

class CWBLabel : public CWBItem {
  std::string Text;
  virtual void OnDraw(CWBDrawAPI *API);

 public:
  CWBLabel();
  CWBLabel(CWBItem *Parent, const CRect &Pos, std::string_view txt = _T( "" ));
  virtual ~CWBLabel();

  virtual TBOOL Initialize(CWBItem *Parent, const CRect &Position,
                           std::string_view txt = _T( "" ));

  std::string GetText() const { return Text; }
  void SetText(std::string_view val);

  static CWBItem *Factory(CWBItem *Root, CXMLNode &node, CRect &Pos);
  WB_DECLARE_GUIITEM(_T( "label" ), CWBItem);

  TBOOL IsMouseTransparent(CPoint &ClientSpacePoint, WBMESSAGE MessageType) {
    return true;
  }
  virtual CSize GetContentSize();
};
