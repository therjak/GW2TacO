#pragma once
#include <memory>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/gw2_tactical.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

class GW2Notepad : public CWBItem {
  bool MessageProc(const CWBMessage& Message) override;
  void OnDraw(CWBDrawAPI* API) override;
  bool Hidden = false;
  GUID CurrentPOI{};

  bool ChangeDefault = false;
  bool canSetFocus = false;

 public:
  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  GW2Notepad(CWBItem* Parent, math::CRect Position);
  static inline GW2Notepad* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_unique<GW2Notepad>(Parent, Position);
    GW2Notepad* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~GW2Notepad() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, math::CRect& Pos);
  WB_DECLARE_GUIITEM("notepad", CWBItem);

  void StartEdit();
};
