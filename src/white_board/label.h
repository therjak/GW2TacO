#pragma once

#include <string>
#include <string_view>

#include "src/white_board/application.h"

class CWBLabel : public CWBItem {
 public:
  CWBLabel(CWBItem* Parent, const math::CRect& Pos, std::string_view txt);
  static inline CWBLabel* Create(CWBItem* Parent, const math::CRect& Pos,
                                 std::string_view txt = "") {
    auto p = std::make_unique<CWBLabel>(Parent, Pos, txt);
    CWBLabel* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~CWBLabel() override;

  bool Initialize(CWBItem* Parent, const math::CRect& Position) override;

  [[nodiscard]] std::string GetText() const { return Text; }
  void SetText(std::string_view val);

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("label", CWBItem);

  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override {
    return true;
  }
  math::CSize GetContentSize() override;

 private:
  void OnDraw(CWBDrawAPI* API) override;
  std::string Text;
};
