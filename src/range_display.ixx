module;
#include <memory>
#include <cassert>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"
#include "src/util/xml_node.h"

export module taco.range_display;

export class GW2RangeDisplay : public CWBGuiType<"gw2rangecircles", CWBItem> {
 public:
  GW2RangeDisplay(CWBItem* Parent, math::CRect Position);
  ~GW2RangeDisplay() override;
  static inline GW2RangeDisplay* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_unique<GW2RangeDisplay>(Parent, Position);
    GW2RangeDisplay* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

 private:
  void OnDraw(CWBDrawAPI* API) override;
  void DrawRangeCircle(CWBDrawAPI* API, float range, float alpha);
};
