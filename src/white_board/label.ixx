module;

#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

export module whiteboard:label;

import :application;
import :css_item;
import :gui_item;

export namespace gui {

class Label : public GuiType<"label", Widget> {
 public:
  Label(std::string_view txt);
  static inline Label* Create(Widget* Parent, const math::Rect& Pos,
                              std::string_view txt = "") {
    auto p = std::make_unique<Label>(txt);
    p->Initialize(Parent, Pos);
    Label* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~Label() override;

  bool Initialize(Widget* Parent, const math::Rect& Position) override;

  [[nodiscard]] std::string GetText() const { return text_; }
  void SetText(std::string_view val);

  static Widget* Factory(Widget* Root, const XMLNode& node,
                         const math::Rect& Pos);

  bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                          MessageType message_type) override {
    return true;
  }
  math::Size GetContentSize() override;

 private:
  void OnDraw(DrawAPI* API) override;
  std::string text_;
};

}  // namespace gui
