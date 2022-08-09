#pragma once

#include <memory>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

class TS3Control : public CWBItem {
  math::CPoint lastpos;
  void OnDraw(CWBDrawAPI* API) override;

 public:
  TS3Control(CWBItem* Parent, math::CRect Position);
  static inline std::shared_ptr<TS3Control> Create(CWBItem* Parent,
                                                   math::CRect Position) {
    auto p = std::make_shared<TS3Control>(Parent, Position);
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~TS3Control() override;

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, math::CRect& Pos);
  WB_DECLARE_GUIITEM("ts3control", CWBItem);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;
};
