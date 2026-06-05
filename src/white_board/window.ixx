module;
#include <tchar.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

export module whiteboard:window;

import :application;
import :css_item;
import :gui_item;

export namespace gui {

constexpr uint32_t WB_WINDOW_CLOSEABLE = 0x00000001;
constexpr uint32_t WB_WINDOW_MOVEABLE = 0x00000002;
constexpr uint32_t WB_WINDOW_RESIZABLE = 0x00000004;
constexpr uint32_t WB_WINDOW_ALWAYSONTOP = 0x00000008;
constexpr uint32_t WB_WINDOW_TITLE = 0x00000010;

constexpr uint32_t WB_WINDOW_DEFAULT =
    (WB_WINDOW_CLOSEABLE | WB_WINDOW_RESIZABLE | WB_WINDOW_MOVEABLE |
     WB_WINDOW_TITLE);

constexpr uint32_t WB_DRAGMODE_TOP = 0x00000001;
constexpr uint32_t WB_DRAGMODE_BOTTOM = 0x00000002;
constexpr uint32_t WB_DRAGMODE_LEFT = 0x00000004;
constexpr uint32_t WB_DRAGMODE_RIGHT = 0x00000008;

constexpr uint32_t WB_DRAGMASK = 0x0000000F;

constexpr uint32_t WB_DRAGMODE_CLOSEBUTTON = 0x00000010;
constexpr uint32_t WB_DRAGMODE_MOVE = 0x00000020;

enum class WBWINDOWELEMENT : char {
  WB_WINELEMENT_CLOSE = 0,
  WB_WINELEMENT_MINIMIZE,
  WB_WINELEMENT_INFO,
  WB_WINELEMENT_TITLE,
};

class Window : public GuiType<"window", Widget> {
 public:
  Window(const TCHAR* txt = "", uint32_t style = WB_WINDOW_DEFAULT);
  static inline Window* Create(Widget* Parent, const math::Rect& Pos,
                               const TCHAR* txt = "",
                               uint32_t style = WB_WINDOW_DEFAULT) {
    auto p = std::make_unique<Window>(txt, style);
    p->Initialize(Parent, Pos);
    Window* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~Window() override;

  bool Initialize(Widget* Parent, const math::Rect& Position) override;
  bool ApplyStyle(std::string_view prop, std::string_view value,
                  const std::vector<std::string>& pseudo) override;

  [[nodiscard]] std::string GetTitle() const { return window_title_; }
  void SetTitle(std::string_view val) { window_title_ = val; }

  uint32_t GetDragMode();
  static Widget* Factory(Widget* Root, const CXMLNode& node,
                         const math::Rect& Pos);

  math::Rect GetElementPos(WBWINDOWELEMENT Element);
  uint32_t GetBorderSelectionArea(const math::Point& mousepos);

 protected:
  void OnDraw(DrawAPI* API) override;
  bool MessageProc(const CWBMessage& Message) override;

  uint32_t style_;
  uint32_t drag_mode_{0};

 private:
  int32_t border_width_{3};
  int32_t title_bar_height_;
  int32_t corner_selection_size_{15};

  math::Size min_size_;

  std::string window_title_;

  std::unordered_map<WBWINDOWELEMENT, CssPropertyBatch> elements_;
};

}  // namespace gui
