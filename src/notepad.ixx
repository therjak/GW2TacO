module;
#include <cassert>
#include <memory>

#include "src/gw2_tactical.h"

export module taco.notepad;

import math;
import whiteboard;
import xml;

export class GW2Notepad : public gui::CWBGuiType<"notepad", gui::CWBItem> {
 public:
  GW2Notepad();
  ~GW2Notepad() override;
  static inline GW2Notepad* Create(gui::CWBItem* Parent, math::Rect Position) {
    auto p = std::make_unique<GW2Notepad>();
    p->Initialize(Parent, Position);
    GW2Notepad* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* Root, CXMLNode& node,
                               math::Rect& Pos);

  bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                          gui::WBMESSAGE MessageType) override;

  void StartEdit();

 private:
  bool MessageProc(const gui::CWBMessage& Message) override;
  void OnDraw(gui::CWBDrawAPI* API) override;
  bool Hidden = false;
  GUID CurrentPOI{};

  bool ChangeDefault = false;
  bool canSetFocus = false;
};
