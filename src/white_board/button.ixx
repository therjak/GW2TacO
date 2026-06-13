module;

#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

export module whiteboard:button;

import :application;
import :css_item;
import :gui_item;

export namespace gui {

class Button : public GuiType<"button", Widget> {
 public:
  Button(std::string_view txt);
  static inline Button* Create(Widget* Parent, const math::Rect& Pos,
                               std::string_view txt = "") {
    auto p = std::make_unique<Button>(txt);
    p->Initialize(Parent, Pos);
    Button* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~Button() override;

  bool Initialize(Widget* Parent, const math::Rect& Position) override;

  [[nodiscard]] std::string GetText() const;
  void SetText(std::string_view val);

  static Widget* Factory(Widget* Root, const XMLNode& node,
                         const math::Rect& Pos);

  math::Size GetContentSize() override;

  virtual void Push(bool pushed);
  virtual bool IsPushed();

  ItemState GetState() override;

 private:
  void OnDraw(DrawAPI* API) override;
  bool MessageProc(const Message& message) override;

  std::string text_;
  bool pushed_ = false;
};

}  // namespace gui
