#pragma once

#include <string_view>
#include <string>

#include "Application.h"

class CWBButton : public CWBItem {
  std::string Text;
  TBOOL Pushed;

  virtual void OnDraw(CWBDrawAPI *API);
  virtual TBOOL MessageProc(CWBMessage &Message);

 public:
  CWBButton(CWBItem *Parent, const CRect &Pos, std::string_view txt = _T( "" ));
  virtual ~CWBButton();

  virtual TBOOL Initialize(CWBItem *Parent, const CRect &Position,
                           std::string_view txt = _T( "" ));

  std::string GetText() const;
  void SetText(std::string_view val);

  static CWBItem *Factory(CWBItem *Root, CXMLNode &node, CRect &Pos);
  WB_DECLARE_GUIITEM(_T( "button" ), CWBItem);

  virtual CSize GetContentSize();

  virtual void Push(TBOOL pushed);
  virtual TBOOL IsPushed();

  virtual WBITEMSTATE GetState();
};
