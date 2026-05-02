module;
#include <cassert>
#include <memory>
#include <utility>

#include "src/gw2_tactical.h"

export module taco.notepad;

import math;
import whiteboard;
import xml;

export class Notepad : public gui::CWBGuiType<"notepad", gui::CWBItem> {
 public:
  Notepad();
  ~Notepad() override;
  static inline Notepad* Create(gui::CWBItem* parent, math::Rect position) {
    auto p = std::make_unique<Notepad>();
    p->Initialize(parent, position);
    Notepad* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }

  static gui::CWBItem* Factory(gui::CWBItem* root, CXMLNode& node,
                               math::Rect& pos);

  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;

  void StartEdit();

 private:
  bool MessageProc(const gui::CWBMessage& message) override;
  void OnDraw(gui::CWBDrawAPI* api) override;
  bool hidden_ = false;
  GUID current_poi_{};

  bool change_default_ = false;
  bool can_set_focus_ = false;
};
