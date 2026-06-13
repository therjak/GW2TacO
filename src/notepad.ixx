module;
#include <cassert>
#include <memory>
#include <utility>

#include "src/gw2_tactical.h"

export module taco.notepad;

import math;
import whiteboard;
import xml;

export class Notepad : public gui::GuiType<"notepad", gui::Widget> {
 public:
  Notepad();
  ~Notepad() override;
  static inline Notepad* Create(gui::Widget* parent, math::Rect position) {
    auto p = std::make_unique<Notepad>();
    p->Initialize(parent, position);
    Notepad* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }

  static gui::Widget* Factory(gui::Widget* root, const XMLNode& node,
                              const math::Rect& pos);

  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::MessageType type) override;

  void StartEdit();

 private:
  bool Initialize(gui::Widget* parent, const math::Rect& position) override;
  bool MessageProc(const gui::Message& message) override;
  void OnDraw(gui::DrawAPI* api) override;
  bool hidden_ = false;
  GUID current_poi_{};

  bool change_default_ = false;
  bool can_set_focus_ = false;
};
