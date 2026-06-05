module;

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module whiteboard:box;

import :application;
import :css_item;
import :gui_item;

export namespace gui {

enum class WBBOXAXIS : uint8_t {
  WB_HORIZONTAL = 0,
  WB_VERTICAL,
};

enum class WBBOXARRANGEMENT : uint8_t {
  WB_ARRANGE_NONE = 0,
  WB_ARRANGE_HORIZONTAL,
  WB_ARRANGE_VERTICAL
};

enum class WBBOXSIZING : uint8_t {
  WB_SIZING_KEEP = 0,
  WB_SIZING_FILL,
};

class Box : public GuiType<"box", Widget> {
 public:
  Box();
  static inline Box* Create(Widget* Parent, const math::Rect& Pos) {
    auto p = std::make_unique<Box>();
    p->Initialize(Parent, Pos);
    Box* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~Box() override;
  static Widget* Factory(Widget* Root, const CXMLNode& node,
                         const math::Rect& Pos);

  virtual void SetArrangement(WBBOXARRANGEMENT a);
  WBBOXARRANGEMENT GetArrangement();
  virtual void SetSpacing(int32_t s);
  virtual void SetAlignment(WBBOXAXIS axis, WBALIGNMENT align);
  virtual void SetSizing(WBBOXAXIS axis, WBBOXSIZING siz);
  bool ApplyStyle(std::string_view prop, std::string_view value,
                  const std::vector<std::string>& pseudo) override;
  bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

 protected:
  void AddChild(std::unique_ptr<Widget>&& Item) override;
  bool MessageProc(const CWBMessage& Message) override;
  virtual void RearrangeChildren();

  void OnDraw(DrawAPI* API) override;
  void RearrangeHorizontal();
  void RearrangeVertical();
  void UpdateScrollbarData();

  int32_t spacing_{};
  WBBOXARRANGEMENT arrangement_;
  WBALIGNMENT alignment_x_, alignment_y_;
  WBBOXSIZING sizing_x_, sizing_y_;
  bool click_through_ = false;
};

}  // namespace gui
