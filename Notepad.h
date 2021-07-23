#pragma once
#include "Bedrock/Whiteboard/Application.h"
#include "gw2tactical.h"

class GW2Notepad : public CWBItem {
  bool MessageProc(CWBMessage& Message) override;
  void OnDraw(CWBDrawAPI* API) override;
  bool Hidden = false;
  GUID CurrentPOI;

  bool ChangeDefault = false;
  bool canSetFocus = false;

 public:
  bool IsMouseTransparent(CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  GW2Notepad(CWBItem* Parent, CRect Position);
  static inline std::shared_ptr<GW2Notepad> Create(CWBItem* Parent,
                                                   CRect Position) {
    auto p = std::make_shared<GW2Notepad>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~GW2Notepad() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "notepad" ), CWBItem);

  void StartEdit();
};
