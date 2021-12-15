#pragma once

#include <string>
#include <string_view>

#include "application.h"

class CWBLabel : public CWBItem {
  std::string Text;
  void OnDraw(CWBDrawAPI* API) override;

 public:
  CWBLabel(CWBItem* Parent, const CRect& Pos, std::string_view txt);
  static inline std::shared_ptr<CWBLabel> Create(
      CWBItem* Parent, const CRect& Pos, std::string_view txt = _T("")) {
    auto p = std::make_shared<CWBLabel>(Parent, Pos, txt);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~CWBLabel() override;

  virtual bool Initialize(CWBItem* Parent, const CRect& Position,
                          std::string_view txt = _T( "" ));

  std::string GetText() const { return Text; }
  void SetText(std::string_view val);

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "label" ), CWBItem);

  bool IsMouseTransparent(const CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override {
    return true;
  }
  CSize GetContentSize() override;
};
