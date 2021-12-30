#pragma once

#include <string>
#include <string_view>

#include "src/white_board/application.h"

class CWBLabel : public CWBItem {
  std::string Text;
  void OnDraw(CWBDrawAPI* API) override;

 public:
  CWBLabel(CWBItem* Parent, const math::CRect& Pos, std::string_view txt);
  static inline std::shared_ptr<CWBLabel> Create(CWBItem* Parent,
                                                 const math::CRect& Pos,
                                                 std::string_view txt = "") {
    auto p = std::make_shared<CWBLabel>(Parent, Pos, txt);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~CWBLabel() override;

  bool Initialize(CWBItem* Parent, const math::CRect& Position) override;

  std::string GetText() const { return Text; }
  void SetText(std::string_view val);

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("label", CWBItem);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override {
    return true;
  }
  math::CSize GetContentSize() override;
};
