module;

#include <cassert>
#include <string>
#include <string_view>

#include "src/base/color.h"

module whiteboard;

import :application;
import :button;
import :font;

using math::Point;
using math::Rect;
using math::Size;

namespace gui {

WBITEMSTATE Button::GetState() {
  WBITEMSTATE i = Widget::GetState();
  if (pushed_) {
    i = IsEnabled() ? WB_STATE_ACTIVE : WB_STATE_DISABLED_ACTIVE;
  } else {
    if (GetApplication()->GetMouseCaptureItem() == this && MouseOver()) {
      i = WB_STATE_ACTIVE;
    } else {
      if (IsEnabled()) {
        i = MouseOver() ? WB_STATE_HOVER : WB_STATE_NORMAL;
      } else {
        i = WB_STATE_DISABLED;
      }
    }
  }

  if (i == WB_STATE_HOVER && GetApplication()->GetMouseCaptureItem()) {
    if (GetApplication()->GetMouseCaptureItem() != this &&
        GetApplication()->GetMouseCaptureItem()->GetType() != "contextmenu") {
      i = WB_STATE_NORMAL;
    }
  }

  return i;
}

void Button::OnDraw(DrawAPI* API) {
  DrawBackground(API);

  const WBITEMSTATE i = GetState();
  Font* Font = GetFont(i);
  const auto TextTransform = static_cast<WBTEXTTRANSFORM>(
      GetDisplayDescriptor().GetValue(i, WB_ITEM_TEXTTRANSFORM));

  if (Font) {
    const CColor TextColor =
        GetDisplayDescriptor().GetColor(i, WB_ITEM_FONTCOLOR);
    const Point TextPos = Font->GetTextPosition(
        text_, GetClientRect(), GetCSSProperties().text_align_x_,
        GetCSSProperties().text_align_y_, TextTransform);
    Font->Write(API, text_, TextPos, TextColor, TextTransform);
  }

  DrawBorder(API);
}

Button::Button(std::string_view Txt) : GuiType(), text_(Txt) {}

Button::~Button() = default;

bool Button::Initialize(Widget* Parent, const Rect& Position) {
  if (!Widget::Initialize(Parent, Position)) return false;

  GetDisplayDescriptor().SetValue(WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR,
                                  0xff2d2d30);
  GetDisplayDescriptor().SetValue(WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR,
                                  0xff3e3e40);
  GetDisplayDescriptor().SetValue(WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR,
                                  0xff007acc);

  GetDisplayDescriptor().SetValue(WB_STATE_DISABLED, WB_ITEM_FONTCOLOR,
                                  0xff656565);
  GetDisplayDescriptor().SetValue(WB_STATE_DISABLED_ACTIVE, WB_ITEM_FONTCOLOR,
                                  0xff656565);

  ContentChanged();
  return true;
}

bool Button::MessageProc(const CWBMessage& Message) {
  switch (Message.Get()) {
    default:
      break;

    case WBM_LEFTBUTTONDOWN:
    case WBM_RIGHTBUTTONDOWN:
    case WBM_MIDDLEBUTTONDOWN:
      if (IsEnabled()) SetCapture();
      return true;

    case WBM_LEFTBUTTONUP:
      GetApplication()->ReleaseCapture();
      if (GetApplication()->GetMouseItem() == this && IsEnabled()) {
        GetApplication()->Send(
            CWBMessage(GetApplication(), WBM_COMMAND, GetGuid()));
      }
      return true;

    case WBM_RIGHTBUTTONUP:
      GetApplication()->ReleaseCapture();
      return true;

    case WBM_MIDDLEBUTTONUP:
      GetApplication()->ReleaseCapture();
      return true;
  }

  return Widget::MessageProc(Message);
}

Size Button::GetContentSize() {
  const WBITEMSTATE i = GetState();
  Font* Font = GetFont(i);
  const auto TextTransform = static_cast<WBTEXTTRANSFORM>(
      GetDisplayDescriptor().GetValue(i, WB_ITEM_TEXTTRANSFORM));

  if (!Font) return Size(0, 0);
  return Size(Font->GetWidth(text_, false, TextTransform),
              Font->GetLineHeight());
}

void Button::Push(bool pushed) { pushed_ = pushed; }

void Button::SetText(std::string_view val) {
  text_ = val;
  ContentChanged();
}

std::string Button::GetText() const { return text_; }

bool Button::IsPushed() { return pushed_; }

Widget* Button::Factory(Widget* Root, const CXMLNode& node, const Rect& Pos) {
  auto button = Button::Create(Root, Pos);
  if (node.HasAttribute("text")) button->SetText(node.GetAttribute("text"));
  return button;
}

}  // namespace gui
