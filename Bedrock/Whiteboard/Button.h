#pragma once

#include <string>
#include <string_view>

#include "Application.h"

class CWBButton : public CWBItem {
  std::string Text;
  bool Pushed = false;

  void OnDraw(CWBDrawAPI* API) override;
  bool MessageProc(const CWBMessage& Message) override;

 public:
  CWBButton(CWBItem* Parent, const CRect& Pos, std::string_view txt);
  static inline std::shared_ptr<CWBButton> Create(
      CWBItem* Parent, const CRect& Pos, std::string_view txt = _T( "" )) {
    auto p = std::make_shared<CWBButton>(Parent, Pos, txt);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~CWBButton() override;

  virtual bool Initialize(CWBItem* Parent, const CRect& Position,
                          std::string_view txt = _T( "" ));

  std::string GetText() const;
  void SetText(std::string_view val);

  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "button" ), CWBItem);

  CSize GetContentSize() override;

  virtual void Push(bool pushed);
  virtual bool IsPushed();

  WBITEMSTATE GetState() override;
};
