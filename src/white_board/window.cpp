module;
#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "src/core2/window_handler.h"

module whiteboard;

import :application;
import :font;
import :window;

using math::Point;
using math::Rect;
using math::Size;

namespace gui {

uint32_t Window::GetBorderSelectionArea(const Point& mousepos) {
  if (GetMouseItem() != this) return 0;

  const Rect r = GetScreenRect();

  if ((r + Rect(-border_width_, -border_width_, -border_width_, -border_width_))
          .Contains(mousepos)) {
    return 0;
  }

  uint32_t result = 0;

  if (r.x1 + corner_selection_size_ > mousepos.x) result |= WB_DRAGMODE_LEFT;
  if (r.x2 - corner_selection_size_ < mousepos.x) result |= WB_DRAGMODE_RIGHT;
  if (r.y1 + corner_selection_size_ > mousepos.y) result |= WB_DRAGMODE_TOP;
  if (r.y2 - corner_selection_size_ < mousepos.y) result |= WB_DRAGMODE_BOTTOM;

  return result;
}

Rect Window::GetElementPos(WindowElement Element) {
  if (elements_.find(Element) != elements_.end()) {
    return elements_[Element].position_descriptor_.GetPosition(
               GetWindowRect().Size(), Size(0, 0), Rect(0, 0, 10, 10)) +
           GetWindowRect().TopLeft();
  }

  switch (Element) {
    case WindowElement::WB_WINELEMENT_CLOSE:
      return Rect(GetWindowRect().TopLeft(),
                  GetWindowRect().TopLeft() + Point(5, 5));
      break;
    case WindowElement::WB_WINELEMENT_MINIMIZE:
      return Rect(GetWindowRect().TopLeft(),
                  GetWindowRect().TopLeft() + Point(5, 5));
      break;
    case WindowElement::WB_WINELEMENT_INFO:
      return Rect(GetWindowRect().TopLeft(),
                  GetWindowRect().TopLeft() + Point(5, 5));
      break;
    case WindowElement::WB_WINELEMENT_TITLE:
      return Rect(GetWindowRect().TopLeft(),
                  GetWindowRect().TopRight() + Point(0, 15));
      break;
    default:
      return {};
      break;
  }
}

void Window::OnDraw(DrawAPI* API) {
  const ItemState i = GetState();
  const auto text_transform = static_cast<TextTransform>(
      GetDisplayDescriptor().GetValue(i, WB_ITEM_TEXTTRANSFORM));

  DrawBackground(API);

  // title text
  if ((style_ & WB_WINDOW_TITLE)) {
    CssPropertyBatch& TitleProps =
        elements_[WindowElement::WB_WINELEMENT_TITLE];

    Font* Font = TitleProps.GetFont(GetApplication(), i);
    if (Font) {
      Rect titlepos = GetElementPos(WindowElement::WB_WINELEMENT_TITLE);
      titlepos = TitleProps.position_descriptor_.GetPadding(titlepos.Size(),
                                                            Rect(0, 0, 0, 0)) +
                 titlepos.TopLeft();
      const Rect r = API->GetCropRect();
      API->SetCropRect(ClientToScreen(titlepos));

      const Point TitlePos = Font->GetTextPosition(
          window_title_, titlepos, TitleProps.text_align_x_,
          TitleProps.text_align_y_,
          static_cast<TextTransform>(TitleProps.display_descriptor_.GetValue(
              i, WB_ITEM_TEXTTRANSFORM)));
      Font->Write(API, window_title_, TitlePos,
                  TitleProps.display_descriptor_.GetColor(i, WB_ITEM_FONTCOLOR),
                  text_transform);
      API->SetCropRect(r);
    }
  }

  bool MouseOverButton = false;

  // close button
  if (style_ & WB_WINDOW_CLOSEABLE) {
    const Rect closebuttonpos =
        GetElementPos(WindowElement::WB_WINELEMENT_CLOSE);

    ItemState buttonstate = WB_STATE_NORMAL;
    if (GetMouseItem() == this) {
      if (closebuttonpos.Contains(ScreenToClient(GetMousePos()))) {
        MouseOverButton = true;
        if (GetMouseCaptureItem() == this) {
          buttonstate = WB_STATE_ACTIVE;
        } else {
          buttonstate = WB_STATE_HOVER;
        }
      }
    }

    if (elements_.find(WindowElement::WB_WINELEMENT_CLOSE) != elements_.end()) {
      if (elements_[WindowElement::WB_WINELEMENT_CLOSE]
              .display_descriptor_.GetSkin(
                  buttonstate, WB_ITEM_BACKGROUNDIMAGE) != 0xffffffff) {
        const int x = 0;
      }

      DrawBackgroundItem(
          API,
          elements_[WindowElement::WB_WINELEMENT_CLOSE].display_descriptor_,
          closebuttonpos, buttonstate);

      Font* Font = GetFont(i);
      if (elements_[WindowElement::WB_WINELEMENT_CLOSE]
              .display_descriptor_.GetSkin(
                  buttonstate, WB_ITEM_BACKGROUNDIMAGE) == 0xffffffff) {
        if (Font) Font->Write(API, "X", Font->GetCenter("X", closebuttonpos));
      }
    }
  }

  DrawBorder(API);

  if (MouseOverButton || !(style_ & WB_WINDOW_RESIZABLE)) return;
  if (GetMouseCaptureItem() && GetMouseCaptureItem() != this) return;
  if (ScrollbarDragged()) return;

  // change mouse cursor
  int32_t border = drag_mode_;
  if (!border) border = GetBorderSelectionArea(GetMousePos());

  if ((border & WB_DRAGMODE_LEFT) || (border & WB_DRAGMODE_RIGHT)) {
    SelectMouseCursor(renderer::MouseCursor::kSizeWe);
  }
  if ((border & WB_DRAGMODE_TOP) || (border & WB_DRAGMODE_BOTTOM)) {
    SelectMouseCursor(renderer::MouseCursor::kSizeNs);
  }
  if (((border & WB_DRAGMODE_LEFT) && (border & WB_DRAGMODE_TOP)) ||
      ((border & WB_DRAGMODE_RIGHT) && (border & WB_DRAGMODE_BOTTOM))) {
    SelectMouseCursor(renderer::MouseCursor::kSizeNwSe);
  }
  if (((border & WB_DRAGMODE_LEFT) && (border & WB_DRAGMODE_BOTTOM)) ||
      ((border & WB_DRAGMODE_RIGHT) && (border & WB_DRAGMODE_TOP))) {
    SelectMouseCursor(renderer::MouseCursor::kSizeNeSw);
  }
}

Window::Window(const TCHAR* txt, uint32_t style)
    : GuiType(),

      title_bar_height_(style & WB_WINDOW_TITLE ? 12 : 0),

      min_size_(corner_selection_size_ * 2 + 1, corner_selection_size_ * 2 + 1),
      window_title_(txt),
      style_(style) {}

Window::~Window() = default;

bool Window::Initialize(Widget* Parent, const Rect& Position) {
  auto& element = elements_[WindowElement::WB_WINELEMENT_CLOSE];
  auto& pdescriptor = element.position_descriptor_;
  pdescriptor.SetMetric(PositionType::WB_MARGIN_TOP, MetricType::WB_PIXELS, 3);
  pdescriptor.SetMetric(PositionType::WB_MARGIN_RIGHT, MetricType::WB_PIXELS,
                        3);
  pdescriptor.SetMetric(PositionType::WB_WIDTH, MetricType::WB_PIXELS, 11);
  pdescriptor.SetMetric(PositionType::WB_HEIGHT, MetricType::WB_PIXELS, 11);
  auto& ddescriptor = element.display_descriptor_;
  ddescriptor.SetValue(WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, 0xff2d2d30);
  ddescriptor.SetValue(WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR, 0xff1c97ea);
  ddescriptor.SetValue(WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR, 0xff3e3e40);

  GetDisplayDescriptor().SetValue(WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR,
                                  0xff2d2d30);
  GetDisplayDescriptor().SetValue(WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR,
                                  0xff007acc);
  GetDisplayDescriptor().SetValue(WB_STATE_NORMAL, WB_ITEM_BORDERCOLOR,
                                  0xff424245);
  GetDisplayDescriptor().SetValue(WB_STATE_ACTIVE, WB_ITEM_BORDERCOLOR,
                                  0xff007acc);

  if (!Widget::Initialize(Parent, Position)) return false;
  SetBorderSizes(1, 1, 1, 1);
  SetClientPadding(border_width_ - 1, title_bar_height_ + border_width_ - 1,
                   border_width_ - 1, border_width_ - 1);
  return true;
}

bool Window::MessageProc(const Message& message) {
  switch (message.Get()) {
    case WBM_LEFTBUTTONDOWN:
      if (Widget::MessageProc(message)) return true;
      if (GetMouseItem() == this) {
        SetCapture();
        SavePosition();

        if (style_ & WB_WINDOW_CLOSEABLE) {
          if (GetElementPos(WindowElement::WB_WINELEMENT_CLOSE)
                  .Contains(ScreenToClient(message.GetPosition()))) {
            drag_mode_ = WB_DRAGMODE_CLOSEBUTTON;
            return true;
          }
        }

        if (style_ & WB_WINDOW_RESIZABLE) {
          drag_mode_ = GetBorderSelectionArea(message.GetPosition());
          if (drag_mode_ & WB_DRAGMASK) return true;
        }

        if (style_ & WB_WINDOW_MOVEABLE) {
          if (!GetClientRect().Contains(
                  ScreenToClient(message.GetPosition()))) {
            drag_mode_ = WB_DRAGMODE_MOVE;
            return true;
          }
        }

        drag_mode_ = 0;
      }
      break;

    case WBM_MOUSEMOVE:
      if (Widget::MessageProc(message)) return true;
      if (GetMouseCaptureItem() == this) {
        if (drag_mode_ == WB_DRAGMODE_MOVE && (style_ & WB_WINDOW_MOVEABLE)) {
          SetPosition(GetSavedPosition() - GetApplication()->GetLeftDownPos() +
                      message.GetPosition());
          return true;
        }
        if ((drag_mode_ & WB_DRAGMASK) && (style_ & WB_WINDOW_RESIZABLE)) {
          Rect r = GetSavedPosition();
          const Point md =
              message.GetPosition() - GetApplication()->GetLeftDownPos();
          if (drag_mode_ & WB_DRAGMODE_LEFT) {
            r.x1 = std::min(r.x1 + md.x, r.x2 - min_size_.x);
          }
          if (drag_mode_ & WB_DRAGMODE_TOP) {
            r.y1 = std::min(r.y1 + md.y, r.y2 - min_size_.y);
          }
          if (drag_mode_ & WB_DRAGMODE_RIGHT) {
            r.x2 = std::max(r.x2 + md.x, r.x1 + min_size_.x);
          }
          if (drag_mode_ & WB_DRAGMODE_BOTTOM) {
            r.y2 = std::max(r.y2 + md.y, r.y1 + min_size_.y);
          }
          SetPosition(r);
          return true;
        }
      }
      break;

    case WBM_LEFTBUTTONUP:
      if (Widget::MessageProc(message)) return true;
      if (GetMouseCaptureItem() == this) {
        if (drag_mode_ == WB_DRAGMODE_CLOSEBUTTON) {
          if (GetElementPos(WindowElement::WB_WINELEMENT_CLOSE)
                  .Contains(ScreenToClient(message.GetPosition()))) {
            GetApplication()->Send(
                Message(GetApplication(), WBM_CLOSE, GetGuid()));
          }
        }

        const bool b = ReleaseCapture();
        if (drag_mode_) {
          drag_mode_ = 0;
          GetApplication()->Send(
              Message(GetApplication(), WBM_WINDOWDRAGSTOPPED, GetGuid()));
          return b;
        }
      }
      break;

    case WBM_CLOSE:
      if (message.GetTarget() == GetGuid()) {
        MarkForDeletion();
      }
      break;

    default:
      break;
  }

  return Widget::MessageProc(message);
}

uint32_t Window::GetDragMode() { return drag_mode_; }

bool Window::ApplyStyle(std::string_view prop, std::string_view value,
                        const std::vector<std::string>& pseudo) {
  bool ElementTarget = false;

  for (size_t x = 1; x < pseudo.size(); x++) {
    if (pseudo[x] == "title" || pseudo[x] == "close" ||
        pseudo[x] == "minimize" || pseudo[x] == "info") {
      ElementTarget = true;
      break;
    }
  }

  // apply font styling to list item anyway
  if (!ElementTarget) {
    InterpretFontString(elements_[WindowElement::WB_WINELEMENT_TITLE], prop,
                        value, pseudo);
  }
  if (!ElementTarget) return Widget::ApplyStyle(prop, value, pseudo);

  bool Handled = false;

  for (size_t x = 1; x < pseudo.size(); x++) {
    if (pseudo[x] == "title") {
      Handled |= elements_[WindowElement::WB_WINELEMENT_TITLE].ApplyStyle(
          this, prop, value, pseudo);
      continue;
    }

    if (pseudo[x] == "close") {
      Handled |= elements_[WindowElement::WB_WINELEMENT_CLOSE].ApplyStyle(
          this, prop, value, pseudo);
      continue;
    }

    if (pseudo[x] == "minimize") {
      Handled |= elements_[WindowElement::WB_WINELEMENT_MINIMIZE].ApplyStyle(
          this, prop, value, pseudo);
      continue;
    }

    if (pseudo[x] == "info") {
      Handled |= elements_[WindowElement::WB_WINELEMENT_INFO].ApplyStyle(
          this, prop, value, pseudo);
      continue;
    }
  }

  return Handled;
}

Widget* Window::Factory(Widget* Root, const XMLNode& node, const Rect& Pos) {
  auto window = Window::Create(Root, Pos);
  if (node.HasAttribute("title")) window->SetTitle(node.GetAttribute("title"));
  return window;
}

}  // namespace gui
