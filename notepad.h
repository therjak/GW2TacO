#pragma once
#include <memory>

#include "gw2_tactical.h"
#include "src/base/rectangle.h"
#include "src/base/vector.h"
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
  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  GW2Notepad(CWBItem* Parent, math::CRect Position);
  static inline std::shared_ptr<GW2Notepad> Create(CWBItem* Parent,
                                                   math::CRect Position) {
    auto p = std::make_shared<GW2Notepad>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~GW2Notepad() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, math::CRect& Pos);
  WB_DECLARE_GUIITEM("notepad", CWBItem);

  void StartEdit();
};
