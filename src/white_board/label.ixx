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

class CWBLabel : public CWBGuiType<"label", CWBItem> {
 public:
  CWBLabel(std::string_view txt);
  static inline CWBLabel* Create(CWBItem* Parent, const math::Rect& Pos,
                                 std::string_view txt = "") {
    auto p = std::make_unique<CWBLabel>(txt);
    p->Initialize(Parent, Pos);
    CWBLabel* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~CWBLabel() override;

  bool Initialize(CWBItem* Parent, const math::Rect& Position) override;

  [[nodiscard]] std::string GetText() const { return Text; }
  void SetText(std::string_view val);

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node, math::Rect& Pos);

  bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                          WBMESSAGE MessageType) override {
    return true;
  }
  math::Size GetContentSize() override;

 private:
  void OnDraw(CWBDrawAPI* API) override;
  std::string Text;
};

}  // namespace gui
