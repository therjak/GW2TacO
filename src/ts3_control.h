﻿#pragma once

#include <memory>

#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

class TS3Control : public CWBItem {
 public:
  TS3Control(CWBItem* Parent, math::CRect Position);
  ~TS3Control() override;
  static inline TS3Control* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_unique<TS3Control>(Parent, Position);
    TS3Control* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, math::CRect& Pos);
  WB_DECLARE_GUIITEM("ts3control", CWBItem);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

 private:
  void OnDraw(CWBDrawAPI* API) override;
  math::CPoint lastpos;
};
