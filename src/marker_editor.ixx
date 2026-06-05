module;
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "src/gw2_tactical.h"

export module taco.marker_editor;

import math;
import whiteboard;
import xml;

export class GW2MarkerEditor
    : public gui::GuiType<"markereditor", gui::Widget> {
 public:
  GW2MarkerEditor();
  static inline GW2MarkerEditor* Create(gui::Widget* parent,
                                        math::Rect position) {
    auto p = std::make_unique<GW2MarkerEditor>();
    p->Initialize(parent, position);
    GW2MarkerEditor* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }
  ~GW2MarkerEditor() override;

  static gui::Widget* Factory(gui::Widget* root, const CXMLNode& node,
                              const math::Rect& pos);

  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;

 private:
  bool Initialize(gui::Widget* parent, const math::Rect& position) override;
  bool MessageProc(const gui::CWBMessage& message) override;
  void OnDraw(gui::DrawAPI* api) override;

  bool hidden_ = false;
  GUID current_poi_{};

  std::vector<GW2TacticalCategory*> category_list_;
  bool change_default_ = false;
};

export std::string default_marker_category = "";
