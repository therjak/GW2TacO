#pragma once
#include "gw2_tactical.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

class GW2Notepad : public CWBItem {
  bool MessageProc(const CWBMessage& Message) override;
  void OnDraw(CWBDrawAPI* API) override;
  bool Hidden = false;
  GUID CurrentPOI;

  bool ChangeDefault = false;
  bool canSetFocus = false;

 public:
  bool IsMouseTransparent(const CPoint& ClientSpacePoint,
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
  WB_DECLARE_GUIITEM("notepad", CWBItem);

  void StartEdit();
};
