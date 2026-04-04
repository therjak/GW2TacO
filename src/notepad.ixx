module;
#include <cassert>
#include <memory>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/gw2_tactical.h"

export module taco.notepad;

import xml;
import whiteboard;

export class GW2Notepad : public CWBGuiType<"notepad", CWBItem> {
 public:
  GW2Notepad(CWBItem* Parent, math::CRect Position);
  ~GW2Notepad() override;
  static inline GW2Notepad* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_unique<GW2Notepad>(Parent, Position);
    GW2Notepad* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, math::CRect& Pos);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  void StartEdit();

 private:
  bool MessageProc(const CWBMessage& Message) override;
  void OnDraw(CWBDrawAPI* API) override;
  bool Hidden = false;
  GUID CurrentPOI{};

  bool ChangeDefault = false;
  bool canSetFocus = false;
};
