#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"
#include "gw2tactical.h"

class GW2Notepad : public CWBItem {
  virtual TBOOL MessageProc(CWBMessage &Message);
  virtual void OnDraw(CWBDrawAPI *API);
  TBOOL Hidden = false;
  GUID CurrentPOI;

  TBOOL ChangeDefault = false;
  bool canSetFocus = false;

 public:
  virtual TBOOL IsMouseTransparent(CPoint &ClientSpacePoint,
                                   WBMESSAGE MessageType);

  GW2Notepad(CWBItem *Parent, CRect Position);
  static inline std::shared_ptr<GW2Notepad> Create(CWBItem *Parent,
                                                   CRect Position) {
    auto p = std::make_shared<GW2Notepad>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  virtual ~GW2Notepad();

  static CWBItem *Factory(CWBItem *Root, CXMLNode &node, CRect &Pos);
  WB_DECLARE_GUIITEM(_T( "notepad" ), CWBItem);

  void StartEdit();
};
