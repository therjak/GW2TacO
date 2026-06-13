module;

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "src/base/logger.h"

module whiteboard;

import :application;
import :box;
import :message;

using math::Point;
using math::Rect;
using math::Size;

namespace gui {

void Box::AddChild(std::unique_ptr<Widget>&& Item) {
  Widget::AddChild(std::move(Item));
  RearrangeChildren();
}

Box::Box()
    : GuiType(),
      arrangement_(BoxArrangement::WB_ARRANGE_NONE),
      spacing_(0),
      alignment_x_(Alignment::LEFT),
      alignment_y_(Alignment::TOP),
      sizing_x_(BoxSizing::WB_SIZING_KEEP),
      sizing_y_(BoxSizing::WB_SIZING_KEEP) {}

Box::~Box() = default;

bool Box::MessageProc(const Message& message) {
  switch (message.Get()) {
    default:
      break;

    case WBM_CLIENTAREACHANGED:
      if (message.GetTarget() == GetGuid()) {
        RearrangeChildren();
      }
      break;

    case WBM_MOUSEWHEEL:
      if (GetApplication()->GetCtrlState() && IsVScrollbarEnabled()) {
        const Point content = GetContentSize();
        const Rect client = GetClientRect();
        if (content.y < client.Height()) break;

        Rect BRect = Rect(0, 0, 0, 0);

        for (uint32_t x = 0; x < NumChildren(); x++) {
          if (!GetChild(x)->IsHidden()) {
            BRect = BRect & GetChild(x)->GetPosition();
          }
        }

        SetVScrollbarPos(
            std::max(BRect.y1,
                     std::min(BRect.y2 - client.Height(),
                              GetVScrollbarPos() -
                                  static_cast<int>(message.Data() *
                                                   client.Height() / 5.0f))),
            false);
        return true;
      }
      break;

    case WBM_HSCROLL: {
      const bool Result = Widget::MessageProc(message);
      if (message.GetTarget() == GetGuid()) {
        ChangeContentOffsetX(-message.Data());
        return true;
      }
      return Result;
    } break;
    case WBM_VSCROLL: {
      const bool Result = Widget::MessageProc(message);
      if (message.GetTarget() == GetGuid()) {
        ChangeContentOffsetY(-message.Data());
        return true;
      }
      return Result;
    } break;

    case WBM_REPOSITION: {
      if (message.GetTarget() == GetGuid()) {
        Widget::MessageProc(message);
        RearrangeChildren();
        return true;
      }

      Widget* i = GetApplication()->FindItemByGuid(message.GetTarget());

      if (i && i->GetParent() == this) {
        if (arrangement_ == BoxArrangement::WB_ARRANGE_NONE) {
          break;  // do nothing here
        }

        Rect r = message.Rectangle();
        const Rect o = i->GetPosition();

        if (sizing_x_ == BoxSizing::WB_SIZING_FILL) {
          r.x1 = o.x1;
          r.x2 = o.x2;
        }

        if (sizing_y_ == BoxSizing::WB_SIZING_FILL) {
          r.y1 = o.y1;
          r.y2 = o.y2;
        }
        Message m(GetApplication(), message.Get(), message.GetTarget(), r,
                  r.TopLeft() == o.TopLeft(), true);
        i->MessageProc(m);  // do the resize as the item sees fit

        RearrangeChildren();
        return true;
      }
    } break;
  }

  return Widget::MessageProc(message);
}

void Box::RearrangeHorizontal() {
  const Rect clientRect = GetClientRect();
  int32_t pos = 0;
  float Excess = 0;

  int32_t NumDynamicChildren = 0;
  int32_t NonDynamicWidth = 0;
  int32_t LastDynamic = 0;

  for (uint32_t x = 0; x < NumChildren(); x++) {
    if (!GetChild(x)->IsWidthSet()) {
      NumDynamicChildren++;
      LastDynamic = x;
    } else {
      NonDynamicWidth += GetChild(x)->GetCalculatedWidth(clientRect.Size());
    }
  }

  int32_t width = (NumChildren() - 1) * spacing_;
  const int32_t DynamicWidth =
      GetClientRect().Width() - NonDynamicWidth - width;
  const float itemsizes = DynamicWidth / static_cast<float>(NumDynamicChildren);

  for (uint32_t x = 0; x < NumChildren(); x++) {
    width += GetChild(x)->GetPosition().Width();
  }

  if (alignment_x_ == Alignment::RIGHT) pos = GetClientRect().Width() - width;
  if (alignment_x_ == Alignment::CENTER) {
    pos = (GetClientRect().Width() - width) / 2;
  }

  for (uint32_t x = 0; x < NumChildren(); x++) {
    Rect ChildPosition = GetChild(x)->GetPosition();

    if (sizing_x_ == BoxSizing::WB_SIZING_FILL) {
      if (!GetChild(x)->IsWidthSet()) {
        ChildPosition.x1 = pos;
        Excess += itemsizes;
        ChildPosition.x2 = ChildPosition.x1 + static_cast<int32_t>(Excess);
        Excess -= static_cast<int32_t>(Excess);
        if (x == LastDynamic && Excess >= 0.5) {
          ChildPosition.x2++;  // fucking float inaccuracies...
        }
      } else {
        ChildPosition.x1 = pos;
        ChildPosition.x2 = ChildPosition.x1 +
                           GetChild(x)->GetCalculatedWidth(clientRect.Size());
      }
    } else {
      const int32_t posw = ChildPosition.Width();
      ChildPosition.x1 = pos;
      ChildPosition.x2 = pos + posw;
    }

    if (sizing_y_ == BoxSizing::WB_SIZING_FILL) {
      ChildPosition.y1 = 0;
      ChildPosition.y2 = GetClientRect().Height();
    }

    int32_t off = 0;
    if (alignment_y_ == Alignment::BOTTOM) {
      off = clientRect.Height() - ChildPosition.Height();
    }
    if (alignment_y_ == Alignment::CENTER) {
      off = (clientRect.Height() - ChildPosition.Height()) / 2;
    }

    const Rect np = Rect(pos, off, pos + ChildPosition.Width(),
                         off + ChildPosition.Height());

    if (GetChild(x)->GetPosition() != np) {
      const Message m = GetChild(x)->BuildPositionMessage(np);
      GetChild(x)->MessageProc(m);
    }

    pos = ChildPosition.x2 + spacing_;
  }
}

void Box::RearrangeVertical() {
  const Rect clientRect = GetClientRect();
  int32_t pos = 0;
  float Excess = 0;

  int32_t NumDynamicChildren = 0;
  int32_t NonDynamicHeight = 0;
  int32_t LastDynamic = 0;

  for (uint32_t x = 0; x < NumChildren(); x++) {
    if (!GetChild(x)->IsHeightSet()) {
      NumDynamicChildren++;
      LastDynamic = x;
    } else {
      NonDynamicHeight += GetChild(x)->GetCalculatedHeight(clientRect.Size());
    }
  }

  int32_t height = (NumChildren() - 1) * spacing_;
  const int32_t DynamicHeight =
      GetClientRect().Height() - NonDynamicHeight - height;
  const float itemsizes =
      DynamicHeight / static_cast<float>(NumDynamicChildren);

  for (uint32_t x = 0; x < NumChildren(); x++) {
    height += GetChild(x)->GetPosition().Height();
  }

  if (alignment_y_ == Alignment::BOTTOM) {
    pos = GetClientRect().Height() - height;
  }
  if (alignment_y_ == Alignment::CENTER) {
    pos = (GetClientRect().Height() - height) / 2;
  }

  for (uint32_t x = 0; x < NumChildren(); x++) {
    Rect ChildPosition = GetChild(x)->GetPosition();

    if (sizing_x_ == BoxSizing::WB_SIZING_FILL) {
      ChildPosition.x1 = 0;
      ChildPosition.x2 = GetClientRect().Width();
    }

    if (sizing_y_ == BoxSizing::WB_SIZING_FILL) {
      if (!GetChild(x)->IsHeightSet()) {
        ChildPosition.y1 = pos;
        Excess += itemsizes;
        ChildPosition.y2 = ChildPosition.y1 + static_cast<int32_t>(Excess);
        Excess -= static_cast<int32_t>(Excess);
        if (x == LastDynamic && Excess >= 0.5) {
          ChildPosition.y2++;  // fucking float inaccuracies...
        }
      } else {
        ChildPosition.y1 = pos;
        ChildPosition.y2 = ChildPosition.y1 +
                           GetChild(x)->GetCalculatedHeight(clientRect.Size());
      }
    } else {
      const int32_t posh = ChildPosition.Height();
      ChildPosition.y1 = pos;
      ChildPosition.y2 = pos + posh;
    }

    int32_t off = 0;
    if (alignment_x_ == Alignment::BOTTOM) {
      off = clientRect.Width() - ChildPosition.Width();
    }
    if (alignment_x_ == Alignment::CENTER) {
      off = (clientRect.Width() - ChildPosition.Width()) / 2;
    }

    const Rect np = Rect(off, pos, off + ChildPosition.Width(),
                         pos + ChildPosition.Height());

    if (GetChild(x)->GetPosition() != np) {
      const Message m = GetChild(x)->BuildPositionMessage(np);
      GetChild(x)->MessageProc(m);
    }

    pos = ChildPosition.y2 + spacing_;
  }
}

void Box::RearrangeChildren() {
  switch (arrangement_) {
    case BoxArrangement::WB_ARRANGE_NONE:
      break;
    case BoxArrangement::WB_ARRANGE_HORIZONTAL:
      RearrangeHorizontal();
      break;
    case BoxArrangement::WB_ARRANGE_VERTICAL:
      RearrangeVertical();
      break;
  }

  UpdateScrollbarData();
}

void Box::OnDraw(DrawAPI* API) {
  DrawBackground(API);
  DrawBorder(API);
}

void Box::SetArrangement(BoxArrangement a) {
  arrangement_ = a;
  RearrangeChildren();
}

BoxArrangement Box::GetArrangement() { return arrangement_; }

void Box::SetSpacing(int32_t s) {
  spacing_ = s;
  RearrangeChildren();
}

bool Box::ApplyStyle(std::string_view prop, std::string_view value,
                     const std::vector<std::string>& pseudo) {
  if (Widget::ApplyStyle(prop, value, pseudo)) return true;

  if (prop == "child-layout") {
    if (value == "none") {
      SetArrangement(BoxArrangement::WB_ARRANGE_NONE);
      return true;
    }
    if (value == "horizontal") {
      SetArrangement(BoxArrangement::WB_ARRANGE_HORIZONTAL);
      return true;
    }
    if (value == "vertical") {
      SetArrangement(BoxArrangement::WB_ARRANGE_VERTICAL);
      return true;
    }
    Log_Warn(
        "[gui] {:s} has invalid parameter: '{:s}' (none/horizontal/vertical "
        "required)",
        prop, value);
    return false;
  }

  if (prop == "child-spacing") {
    int32_t dw = std::stoi(std::string(value));
    SetSpacing(dw);
    return true;
  }

  if (prop == "child-align-x") {
    if (value == "left") {
      SetAlignment(BoxAxis::WB_HORIZONTAL, Alignment::LEFT);
      return true;
    }
    if (value == "right") {
      SetAlignment(BoxAxis::WB_HORIZONTAL, Alignment::RIGHT);
      return true;
    }
    if (value == "center") {
      SetAlignment(BoxAxis::WB_HORIZONTAL, Alignment::CENTER);
      return true;
    }

    Log_Warn(
        "[gui] {:s} has invalid parameter: '{:s}' (left/center/right required)",
        prop, value);

    return true;
  }

  if (prop == "child-align-y") {
    if (value == "top") {
      SetAlignment(BoxAxis::WB_VERTICAL, Alignment::TOP);
      return true;
    }
    if (value == "bottom") {
      SetAlignment(BoxAxis::WB_VERTICAL, Alignment::BOTTOM);
      return true;
    }
    if (value == "center") {
      SetAlignment(BoxAxis::WB_VERTICAL, Alignment::CENTER);
      return true;
    }

    Log_Warn(
        "[gui] {:s} has invalid parameter: '{:s}' (top/center/bottom required)",
        prop, value);

    return true;
  }

  if (prop == "child-fill-x") {
    if (value == "false") {
      SetSizing(BoxAxis::WB_HORIZONTAL, BoxSizing::WB_SIZING_KEEP);
      return true;
    }
    if (value == "true") {
      SetSizing(BoxAxis::WB_HORIZONTAL, BoxSizing::WB_SIZING_FILL);
      return true;
    }

    Log_Warn("[gui] {:s} has invalid parameter: '{:s}' (true/false required)",
             prop, value);

    return true;
  }

  if (prop == "child-fill-y") {
    if (value == "false") {
      SetSizing(BoxAxis::WB_VERTICAL, BoxSizing::WB_SIZING_KEEP);
      return true;
    }
    if (value == "true") {
      SetSizing(BoxAxis::WB_VERTICAL, BoxSizing::WB_SIZING_FILL);
      return true;
    }

    Log_Warn("[gui] {:s} has invalid parameter: '{:s}' (true/false required)",
             prop, value);

    return true;
  }

  return false;
}

bool Box::IsMouseTransparent(const Point& ClientSpacePoint,
                             MessageType message_type) {
  return click_through_;
}

void Box::SetAlignment(BoxAxis axis, Alignment align) {
  if (axis == BoxAxis::WB_HORIZONTAL) alignment_x_ = align;
  if (axis == BoxAxis::WB_VERTICAL) alignment_y_ = align;
  RearrangeChildren();
}

void Box::SetSizing(BoxAxis axis, BoxSizing siz) {
  if (axis == BoxAxis::WB_HORIZONTAL) sizing_x_ = siz;
  if (axis == BoxAxis::WB_VERTICAL) sizing_y_ = siz;
  RearrangeChildren();
}

Widget* Box::Factory(Widget* Root, const XMLNode& node, const Rect& Pos) {
  auto box = Box::Create(Root, Pos);

  if (node.HasAttribute("clickthrough")) {
    int32_t b = 0;
    node.GetAttributeAsInteger("clickthrough", &b);
    box->click_through_ = b != 0;
  }

  return box;
}

void Box::UpdateScrollbarData() {
  if (!ScrollbarsEnabled()) return;

  Rect BRect = Rect(0, 0, 0, 0);

  for (uint32_t x = 0; x < NumChildren(); x++) {
    if (!GetChild(x)->IsHidden()) BRect = BRect & GetChild(x)->GetPosition();
  }

  SetHScrollbarParameters(BRect.x1, BRect.x2, GetClientRect().Width());
  SetVScrollbarParameters(BRect.y1, BRect.y2, GetClientRect().Height());

  if (GetClientRect().Width() >= BRect.Width() &&
      GetHScrollbarPos() != BRect.x1) {
    SetHScrollbarPos(BRect.x1, true);
  }
  if (GetClientRect().Height() >= BRect.Height() &&
      GetVScrollbarPos() != BRect.y1) {
    SetVScrollbarPos(BRect.y1, true);
  }
}

}  // namespace gui
