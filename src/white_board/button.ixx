module;

#include <cassert>
#include <memory>
#include <string>
#include <string_view>

export module whiteboard:button;

import :application;
import :css_item;
import :gui_item;

export namespace gui {

class CWBButton : public CWBGuiType<"button", CWBItem> {
 public:
  CWBButton(std::string_view txt);
  static inline CWBButton* Create(CWBItem* Parent, const math::CRect& Pos,
                                  std::string_view txt = "") {
    auto p = std::make_unique<CWBButton>(txt);
    p->Initialize(Parent, Pos);
    CWBButton* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~CWBButton() override;

  bool Initialize(CWBItem* Parent, const math::CRect& Position) override;

  [[nodiscard]] std::string GetText() const;
  void SetText(std::string_view val);

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);

  math::CSize GetContentSize() override;

  virtual void Push(bool pushed);
  virtual bool IsPushed();

  WBITEMSTATE GetState() override;

 private:
  void OnDraw(CWBDrawAPI* API) override;
  bool MessageProc(const CWBMessage& Message) override;

  std::string Text;
  bool Pushed = false;
};

}  // namespace gui
