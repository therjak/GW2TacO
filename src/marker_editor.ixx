module;
#include <cassert>
#include <memory>
#include <vector>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/gw2_tactical.h"

export module taco.marker_editor;

import xml;
import whiteboard;

export class GW2MarkerEditor
    : public gui::CWBGuiType<"markereditor", gui::CWBItem> {
 public:
  GW2MarkerEditor();
  static inline GW2MarkerEditor* Create(gui::CWBItem* Parent,
                                        math::CRect Position) {
    auto p = std::make_unique<GW2MarkerEditor>();
    p->Initialize(Parent, Position);
    GW2MarkerEditor* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~GW2MarkerEditor() override;

  static gui::CWBItem* Factory(gui::CWBItem* Root, CXMLNode& node,
                               math::CRect& Pos);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          gui::WBMESSAGE MessageType) override;

 private:
  bool MessageProc(const gui::CWBMessage& Message) override;
  void OnDraw(gui::CWBDrawAPI* API) override;

  bool Hidden = false;
  GUID CurrentPOI{};

  std::vector<GW2TacticalCategory*> CategoryList;
  bool ChangeDefault = false;
};
