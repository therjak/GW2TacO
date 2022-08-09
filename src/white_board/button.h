#pragma once

#include <string>
#include <string_view>

#include "src/white_board/application.h"

class CWBButton : public CWBItem {
  std::string Text;
  bool Pushed = false;

  void OnDraw(CWBDrawAPI* API) override;
  bool MessageProc(const CWBMessage& Message) override;

 public:
  CWBButton(CWBItem* Parent, const math::CRect& Pos, std::string_view txt);
  static inline std::shared_ptr<CWBButton> Create(CWBItem* Parent,
                                                  const math::CRect& Pos,
                                                  std::string_view txt = "") {
    auto p = std::make_shared<CWBButton>(Parent, Pos, txt);
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~CWBButton() override;

  bool Initialize(CWBItem* Parent, const math::CRect& Position) override;

  std::string GetText() const;
  void SetText(std::string_view val);

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("button", CWBItem);

  math::CSize GetContentSize() override;

  virtual void Push(bool pushed);
  virtual bool IsPushed();

  WBITEMSTATE GetState() override;
};
