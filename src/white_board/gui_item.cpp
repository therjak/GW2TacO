module;
#include <algorithm>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "src/base/color.h"
#include "src/base/logger.h"
#include "src/base/string_format.h"

module whiteboard;

import :application;
import :context_menu;
import :font;
import :gui_item;

using math::Point;
using math::Rect;
using math::Size;

namespace gui {

static WBGUID WB_GUID_COUNTER = 1337;

//////////////////////////////////////////////////////////////////////////
// Display Descriptor

DisplayState::DisplayState() = default;

DisplayState::~DisplayState() = default;

CColor DisplayState::GetColor(WBITEMVISUALCOMPONENT v) {
  return CColor(visuals_[v]);
}

bool DisplayState::IsSet(WBITEMVISUALCOMPONENT v) { return visual_set_[v]; }

void DisplayState::SetValue(WBITEMVISUALCOMPONENT v, int32_t value) {
  visuals_[v] = value;
  visual_set_[v] = true;
}

WBSKINELEMENTID DisplayState::GetSkin(WBITEMVISUALCOMPONENT v) {
  return visuals_[v];
}

int32_t DisplayState::GetValue(WBITEMVISUALCOMPONENT v) { return visuals_[v]; }

DisplayProperties::DisplayProperties() = default;

DisplayProperties::~DisplayProperties() = default;

WBSKINELEMENTID DisplayProperties::GetSkin(WBITEMSTATE s,
                                           WBITEMVISUALCOMPONENT v) {
  // if (States.HasKey(s))
  {
    if (states_[s].IsSet(v)) return states_[s].GetSkin(v);
  }
  // if (States.HasKey(WB_STATE_NORMAL))
  {
    if (states_[WB_STATE_NORMAL].IsSet(v)) {
      return states_[WB_STATE_NORMAL].GetSkin(v);
    }
  }

  return 0xffffffff;
}

CColor DisplayProperties::GetColor(WBITEMSTATE s, WBITEMVISUALCOMPONENT v) {
  // if (States.HasKey(s))
  {
    if (states_[s].IsSet(v)) return states_[s].GetColor(v);
  }
  // if (States.HasKey(WB_STATE_NORMAL))
  {
    if (states_[WB_STATE_NORMAL].IsSet(v)) {
      return states_[WB_STATE_NORMAL].GetColor(v);
    }
  }

  switch (v) {
    default:
      return CColor{0xffffffff};
    case WB_ITEM_BACKGROUNDCOLOR:
      return CColor{0};
    case WB_ITEM_FOREGROUNDCOLOR:
      return CColor{0};
    case WB_ITEM_BORDERCOLOR:
      return CColor{0xff434346};
    case WB_ITEM_FONTCOLOR:
      return CColor{0xffffffff};
  }
}

void DisplayProperties::SetValue(WBITEMSTATE s, WBITEMVISUALCOMPONENT v,
                                 int32_t value) {
  states_[s].SetValue(v, value);
}

int32_t DisplayProperties::GetValue(WBITEMSTATE s, WBITEMVISUALCOMPONENT v) {
  if (states_[s].IsSet(v)) return states_[s].GetValue(v);
  return -1;
}

CssPropertyBatch::CssPropertyBatch()
    : border_sizes_(0, 0, 0, 0),
      text_align_x_(WBTEXTALIGNMENTX::WBTA_CENTERX),
      text_align_y_(WBTEXTALIGNMENTY::WBTA_CENTERY) {}

Font* CssPropertyBatch::GetFont(Application* app, WBITEMSTATE State) {
  if (fonts_.find(State) != fonts_.end()) {
    return app->GetFont(fonts_[State]);
  }
  if (fonts_.find(WB_STATE_NORMAL) != fonts_.end()) {
    return app->GetFont(fonts_[WB_STATE_NORMAL]);
  }
  return app->GetDefaultFont();
}

bool CssPropertyBatch::ApplyStyle(Widget* Owner, std::string_view prop,
                                  std::string_view value,
                                  const std::vector<std::string>& pseudo) {
  if (Owner->InterpretPositionString(*this, prop, value, pseudo)) return true;
  if (Owner->InterpretDisplayString(*this, prop, value, pseudo)) return true;
  return false;
}

//////////////////////////////////////////////////////////////////////////
// Widget

void Widget::UpdateScreenRect() {
  const Rect sr = screen_rect_;

  if (parent_) {
    screen_rect_ = position_ + parent_->ClientToScreen(Point(0, 0)) +
                   parent_->content_offset_;
  } else {
    screen_rect_ = position_;
  }

  if (sr != screen_rect_) {
    for (auto& c : children_) {
      c->UpdateScreenRect();
    }
  }
}

void Widget::HandleHScrollbarClick(WBSCROLLDRAGMODE m) {
  switch (m) {
    case WB_SCROLLDRAG_BUTTON1:
      SetHScrollbarPos(GetHScrollbarPos() - GetScrollbarStep(), true);
      break;
    case WB_SCROLLDRAG_BUTTON2:
      SetHScrollbarPos(GetHScrollbarPos() + GetScrollbarStep(), true);
      break;
    case WB_SCROLLDRAG_UP:
      SetHScrollbarPos(GetHScrollbarPos() - HScrollbar.view_size_, true);
      break;
    case WB_SCROLLDRAG_DOWN:
      SetHScrollbarPos(GetHScrollbarPos() + HScrollbar.view_size_, true);
      break;
    default:
      break;
  }
}

void Widget::HandleVScrollbarClick(WBSCROLLDRAGMODE m) {
  switch (m) {
    case WB_SCROLLDRAG_BUTTON1:
      SetVScrollbarPos(GetVScrollbarPos() - GetScrollbarStep(), true);
      break;
    case WB_SCROLLDRAG_BUTTON2:
      SetVScrollbarPos(GetVScrollbarPos() + GetScrollbarStep(), true);
      break;
    case WB_SCROLLDRAG_UP:
      SetVScrollbarPos(GetVScrollbarPos() - VScrollbar.view_size_, true);
      break;
    case WB_SCROLLDRAG_DOWN:
      SetVScrollbarPos(GetVScrollbarPos() + VScrollbar.view_size_, true);
      break;
    default:
      break;
  }
}

bool Widget::MessageProc(const CWBMessage& Message) {
  switch (Message.Get()) {
    case WBM_NONE:
      Log_Err("[gui] Message Type 0 Encountered. Message Target is {:d}",
              Message.GetTarget());
      return true;

    case WBM_LEFTBUTTONDOWN:
      // handle scrollbars
      if (GetMouseItem() == this) {
        Rect b1, up, th, dn, b2;
        // mouse pos
        Point mp = Message.GetPosition();
        if (GetHScrollbarRectangles(&b1, &up, &th, &dn, &b2)) {
          if (ClientToScreen(b1).Contains(mp)) {
            HScrollbar.drag_mode_ = WB_SCROLLDRAG_BUTTON1;
          }
          if (ClientToScreen(up).Contains(mp)) {
            HScrollbar.drag_mode_ = WB_SCROLLDRAG_UP;
          }
          if (ClientToScreen(th).Contains(mp)) {
            HScrollbar.drag_mode_ = WB_SCROLLDRAG_THUMB;
          }
          if (ClientToScreen(dn).Contains(mp)) {
            HScrollbar.drag_mode_ = WB_SCROLLDRAG_DOWN;
          }
          if (ClientToScreen(b2).Contains(mp)) {
            HScrollbar.drag_mode_ = WB_SCROLLDRAG_BUTTON2;
          }
          if (HScrollbar.drag_mode_ != WB_SCROLLDRAG_NONE) {
            HScrollbar.drag_start_position_ = HScrollbar.scroll_pos_;
            HandleHScrollbarClick(HScrollbar.drag_mode_);
            SetCapture();
            return true;
          }
        }
        if (GetVScrollbarRectangles(&b1, &up, &th, &dn, &b2)) {
          if (ClientToScreen(b1).Contains(mp)) {
            VScrollbar.drag_mode_ = WB_SCROLLDRAG_BUTTON1;
          }
          if (ClientToScreen(up).Contains(mp)) {
            VScrollbar.drag_mode_ = WB_SCROLLDRAG_UP;
          }
          if (ClientToScreen(th).Contains(mp)) {
            VScrollbar.drag_mode_ = WB_SCROLLDRAG_THUMB;
          }
          if (ClientToScreen(dn).Contains(mp)) {
            VScrollbar.drag_mode_ = WB_SCROLLDRAG_DOWN;
          }
          if (ClientToScreen(b2).Contains(mp)) {
            VScrollbar.drag_mode_ = WB_SCROLLDRAG_BUTTON2;
          }
          if (VScrollbar.drag_mode_ != WB_SCROLLDRAG_NONE) {
            VScrollbar.drag_start_position_ = VScrollbar.scroll_pos_;
            HandleVScrollbarClick(VScrollbar.drag_mode_);
            SetCapture();
            return true;
          }
        }
      }
      return false;
      break;

    case WBM_LEFTBUTTONREPEAT:
      if (GetMouseItem() == this) {
        if (HScrollbar.drag_mode_ != WB_SCROLLDRAG_NONE &&
            HScrollbar.drag_mode_ != WB_SCROLLDRAG_THUMB) {
          Rect b1, up, th, dn, b2;
          // mouse pos
          Point mp = Message.GetPosition();
          if (GetHScrollbarRectangles(&b1, &up, &th, &dn, &b2)) {
            if (ClientToScreen(b1).Contains(mp) &&
                HScrollbar.drag_mode_ == WB_SCROLLDRAG_BUTTON1) {
              HandleHScrollbarClick(HScrollbar.drag_mode_);
            }
            if (ClientToScreen(up).Contains(mp) &&
                HScrollbar.drag_mode_ == WB_SCROLLDRAG_UP) {
              HandleHScrollbarClick(HScrollbar.drag_mode_);
            }
            if (ClientToScreen(dn).Contains(mp) &&
                HScrollbar.drag_mode_ == WB_SCROLLDRAG_DOWN) {
              HandleHScrollbarClick(HScrollbar.drag_mode_);
            }
            if (ClientToScreen(b2).Contains(mp) &&
                HScrollbar.drag_mode_ == WB_SCROLLDRAG_BUTTON2) {
              HandleHScrollbarClick(HScrollbar.drag_mode_);
            }
          }

          return true;
        }

        if (VScrollbar.drag_mode_ != WB_SCROLLDRAG_NONE &&
            VScrollbar.drag_mode_ != WB_SCROLLDRAG_THUMB) {
          Rect b1, up, th, dn, b2;
          // mouse pos
          Point mp = Message.GetPosition();
          if (GetVScrollbarRectangles(&b1, &up, &th, &dn, &b2)) {
            if (ClientToScreen(b1).Contains(mp) &&
                VScrollbar.drag_mode_ == WB_SCROLLDRAG_BUTTON1) {
              HandleVScrollbarClick(VScrollbar.drag_mode_);
            }
            if (ClientToScreen(up).Contains(mp) &&
                VScrollbar.drag_mode_ == WB_SCROLLDRAG_UP) {
              HandleVScrollbarClick(VScrollbar.drag_mode_);
            }
            if (ClientToScreen(dn).Contains(mp) &&
                VScrollbar.drag_mode_ == WB_SCROLLDRAG_DOWN) {
              HandleVScrollbarClick(VScrollbar.drag_mode_);
            }
            if (ClientToScreen(b2).Contains(mp) &&
                VScrollbar.drag_mode_ == WB_SCROLLDRAG_BUTTON2) {
              HandleVScrollbarClick(VScrollbar.drag_mode_);
            }
          }

          return true;
        }
      }
      break;

    case WBM_MOUSEMOVE:
      // handle scrollbars only
      if (GetMouseCaptureItem() == this) {
        if (ScrollbarDragged()) {
          // mouse delta
          const Point md = Message.GetPosition() - app_->GetLeftDownPos();

          if (HScrollbar.drag_mode_ == WB_SCROLLDRAG_THUMB) {
            const int32_t newpos = CalculateScrollbarMovement(
                HScrollbar, GetClientRect().Width(), md.x);
            HScrollbar.scroll_pos_ = newpos;
            app_->Send(CWBMessage(app_, WBM_HSCROLL, GetGuid(), newpos));
          }
          if (VScrollbar.drag_mode_ == WB_SCROLLDRAG_THUMB) {
            const int32_t newpos = CalculateScrollbarMovement(
                VScrollbar, GetClientRect().Height(), md.y);
            VScrollbar.scroll_pos_ = newpos;
            app_->Send(CWBMessage(app_, WBM_VSCROLL, GetGuid(), newpos));
          }

          return true;
        }
      }
      return false;
      break;

    case WBM_LEFTBUTTONUP:
      // handle scrollbars only
      if (GetMouseCaptureItem() == this) {
        if (ScrollbarDragged()) {
          HScrollbar.drag_mode_ = WB_SCROLLDRAG_NONE;
          VScrollbar.drag_mode_ = WB_SCROLLDRAG_NONE;
          app_->ReleaseCapture();
          return true;
        }
      }
      return false;
      break;

    case WBM_REPOSITION:
      if (Message.GetTarget() == guid_) {
        ApplyPosition(Message.Rectangle());
        if (Message.Moved()) OnMove(Message.Rectangle().TopLeft());
        if (Message.Resized()) {
          OnResize(Message.Rectangle().Size());
        }
        for (auto& c : children_) {
          c->CalculateWindowPosition(GetClientRect().Size());
        }

        return true;
      }
      return false;
      break;

      // case WBM_FOCUSLOST:
      //   if (Message.GetTarget() == Guid) {
      //     if (Parent) Parent->child_in_focus_ = NULL;
      //     return true;
      //   }
      //   return false;

    case WBM_HIDE:
      if (Message.GetTarget() == guid_) {
        hidden_ = true;
        return true;
      }
      return false;
      break;

    case WBM_UNHIDE:
      if (Message.GetTarget() == guid_) {
        hidden_ = false;
        return true;
      }
      return false;
      break;

    case WBM_CONTENTOFFSETCHANGE:
      if (Message.GetTarget() == GetGuid()) {
        ChangeContentOffset(Message.GetPosition());
        return true;
      }
      break;

    default:
      return false;
  }

  return false;
}

void Widget::DrawBackgroundItem(DrawAPI* API, DisplayProperties& Descriptor,
                                const Rect& Pos, WBITEMSTATE i,
                                WBITEMVISUALCOMPONENT v) {
  CColor bck = Descriptor.GetColor(i, WB_ITEM_BACKGROUNDCOLOR);
  if (bck.A()) {
    // API->SetCropRect(ClientToScreen(Pos));
    API->DrawRect(Pos, Descriptor.GetColor(i, WB_ITEM_BACKGROUNDCOLOR));
  }

  const WBSKINELEMENTID id = Descriptor.GetSkin(i, v);

  if (id != 0xffffffff) {
    if (id & 0x80000000) {
      // skin element
      SkinElement* e = app_->GetSkin()->GetElement(id);
      if (e) {
        const auto AlignX = static_cast<WBALIGNMENT>(
            Descriptor.GetValue(i, WB_ITEM_BACKGROUNDALIGNMENT_X));
        const auto AlignY = static_cast<WBALIGNMENT>(
            Descriptor.GetValue(i, WB_ITEM_BACKGROUNDALIGNMENT_Y));
        const Size elementsize = e->GetElementSize(API);
        Point offset = Point(0, 0);
        Size size = Pos.Size();

        if (e->GetBehavior(0) ==
            WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_PIXELCORRECT) {
          if (AlignX == WBALIGNMENT::RIGHT) {
            offset.x = Pos.Width() - elementsize.x;
          }
          if (AlignX == WBALIGNMENT::CENTER) {
            offset.x = (Pos.Width() - elementsize.x) / 2;
          }
          size.x = elementsize.x;
        }

        if (e->GetBehavior(1) ==
            WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_PIXELCORRECT) {
          if (AlignY == WBALIGNMENT::BOTTOM) {
            offset.y = Pos.Height() - elementsize.y;
          }
          if (AlignY == WBALIGNMENT::MIDDLE) {
            offset.y = (Pos.Height() - elementsize.y) / 2;
          }
          size.y = elementsize.y;
        }

        const Rect DisplayRect =
            Rect(Pos.TopLeft() + offset, Pos.TopLeft() + offset + size);

        app_->GetSkin()->RenderElement(API, id, DisplayRect);
      }
    } else {
      // mosaic
      app_->GetSkin()->RenderElement(API, id, Pos);
    }
    return;
  }
}

void Widget::DrawBackground(DrawAPI* API, WBITEMSTATE State) {
  DrawBackground(API, GetWindowRect(), State, css_properties_);
}

void Widget::DrawBackground(DrawAPI* API) { DrawBackground(API, GetState()); }

void Widget::DrawBackground(DrawAPI* API, const Rect& rect, WBITEMSTATE State,
                            CssPropertyBatch& cssProps) {
  DrawBackgroundItem(API, cssProps.display_descriptor_, rect, State);
}

void Widget::DrawBorder(DrawAPI* API) {
  DrawBorder(API, GetWindowRect(), css_properties_);
}

void Widget::DrawBorder(DrawAPI* API, const Rect& r,
                        CssPropertyBatch& cssProps) {
  const auto& crop = API->GetCropRect();

  const CColor color =
      cssProps.display_descriptor_.GetColor(GetState(), WB_ITEM_BORDERCOLOR);

  API->SetCropRect(ClientToScreen(r));

  if (cssProps.border_sizes_.x1 > 0) {
    API->DrawRect(
        Rect(r.TopLeft(), r.BottomLeft() + Point(cssProps.border_sizes_.x1, 0)),
        color);
  }
  if (cssProps.border_sizes_.y1 > 0) {
    API->DrawRect(
        Rect(r.TopLeft(), r.TopRight() + Point(0, cssProps.border_sizes_.y1)),
        color);
  }
  if (cssProps.border_sizes_.x2 > 0) {
    API->DrawRect(Rect(r.TopRight() - Point(cssProps.border_sizes_.x2, 0),
                       r.BottomRight()),
                  color);
  }
  if (cssProps.border_sizes_.y2 > 0) {
    API->DrawRect(Rect(r.BottomLeft() - Point(0, cssProps.border_sizes_.y2),
                       r.BottomRight()),
                  color);
  }

  API->SetCropRect(crop);
}

void Widget::OnDraw(DrawAPI* API) {
  DrawBackground(API);
  DrawBorder(API);
}

void Widget::OnPostDraw(DrawAPI* API) {}

int32_t Widget::GetScrollbarStep() { return 1; }

void Widget::OnMove(const Point& p) {}

void Widget::OnResize(const Size& s) {}

void Widget::OnMouseEnter() {
  // Log_Dbg("Mouse Entered Item {:d}",GetGuid());
}

void Widget::OnMouseLeave() {
  // Log_Dbg("Mouse Left Item {:d}",GetGuid());
}

void Widget::CalculateClientPosition() {
  const Point p = ClientToScreen(Point(0, 0));

  client_rect_ = GetPositionDescriptor().GetPadding(GetWindowRect().Size(),
                                                    GetBorderSizes());

  HScrollbar.visible_ = false;
  VScrollbar.visible_ = false;
  AdjustClientAreaToFitScrollbars();

  if (p != ClientToScreen(Point(0, 0)) || screen_rect_.Width() < 0) {
    // this updates all child items for the new position
    UpdateScreenRect();
  }
}

void Widget::CalculateWindowPosition(const Size& s) {
  SetPosition(GetPositionDescriptor().GetPosition(s, stored_content_size_,
                                                  GetPosition()));
}

void Widget::DrawTree(DrawAPI* API) {
  if (hidden_) return;

  Rect PCrop = API->GetParentCropRect();
  Rect Crop = API->GetCropRect();
  API->SetParentCropRect(Crop);
  const Point Offset = API->GetOffset();

  API->SetCropRect(screen_rect_);
  API->SetOffset(ClientToScreen(Point(0, 0)));

  ApplyOpacity(API);
  OnDraw(API);

  if (VScrollbar.visible_ || HScrollbar.visible_) {
    API->SetCropRect(screen_rect_);
    DrawHScrollbar(API);
    DrawVScrollbar(API);
  }

  // crop children to client rect
  API->SetCropRect(ClientToScreen(GetClientRect()));

  for (auto& c : children_) {
    c->DrawTree(API);
  }

  OnPostDraw(API);

  API->SetParentCropRect(PCrop);
  API->SetCropRect(Crop);
  API->SetOffset(Offset);
}

void Widget::ApplyPosition(const Rect& Pos) {
  const Rect r = GetScreenRect();

  position_ = Pos;
  CalculateClientPosition();
  UpdateScreenRect();

  const Rect r2 = GetScreenRect();

  if (app_ && r.Contains(GetMousePos()) != r2.Contains(GetMousePos())) {
    app_->UpdateMouseItem();
  }
}

bool Widget::Focusable() const { return true; }

Widget* Widget::GetChildInFocus() { return child_in_focus_; }

Widget* Widget::SetCapture() { return app_->SetCapture(this); }

bool Widget::ReleaseCapture() const { return app_->ReleaseCapture(); }

bool Widget::IsMouseTransparent(const Point& ClientSpacePoint,
                                WBMESSAGE MessageType) {
  if (force_mouse_transparent_) return true;
  if (hidden_) return true;
  return false;
}

bool Widget::FindItemInParentTree(const Widget* Item) {
  Widget* i = this;
  while (i) {
    if (i == Item) return true;
    i = i->parent_;
  }
  return false;
}

Widget* Widget::GetItemUnderMouse(const Point& Pos, const Rect& CropRect,
                                  WBMESSAGE MessageType) {
  Rect LocalCropRect = CropRect;
  LocalCropRect = LocalCropRect | screen_rect_;

  if (hidden_ || !LocalCropRect.Contains(Pos)) {
    return nullptr;
  }

  LocalCropRect = LocalCropRect | ClientToScreen(GetClientRect());

  for (int32_t x = children_.size(); x > 0; x--) {
    Widget* Res =
        children_[x - 1]->GetItemUnderMouse(Pos, LocalCropRect, MessageType);
    if (Res) return Res;
  }

  if (IsMouseTransparent(ScreenToClient(Pos), MessageType)) return nullptr;

  return this;
}

void Widget::SetChildAsTopmost(int32_t Index) {
  auto it = children_.begin() + Index;
  std::rotate(it, it + 1, children_.end());
}

void Widget::SetChildAsBottommost(int32_t Index) {
  auto it = children_.begin() + Index;
  std::rotate(children_.begin(), it, it);
}

void Widget::SetTopmost() {
  if (!GetParent()) return;
  const int32_t x = GetParent()->GetChildIndex(this);
  if (x < 0) return;
  parent_->SetChildAsTopmost(x);
}

void Widget::SetBottommost() {
  if (!GetParent()) return;
  const int32_t x = GetParent()->GetChildIndex(this);
  if (x < 0) return;
  parent_->SetChildAsBottommost(x);
}

Widget::Widget()
    : guid_(WB_GUID_COUNTER++),
      hidden_(false),
      disabled_(false),
      data_(nullptr),
      child_in_focus_(nullptr),
      scrollbar_thumb_minimal_size_(4),
      scrollbar_size_(16),
      scrollbar_button_size_(16),
      content_offset_(0, 0) {}

Widget::~Widget() {
  for (auto& c : children_) {
    c->parent_ = nullptr;
  }

  // Make sure Children is empty before we delete them, as they try to
  // unregister themselfes.
  std::vector<std::unique_ptr<Widget>> c;
  std::swap(c, children_);

  if (parent_) {
    auto& pc = parent_->children_;
    auto it = std::find_if(
        pc.begin(), pc.end(),
        [this](const std::unique_ptr<Widget>& i) { return i.get() == this; });
    if (it != pc.end()) {
      pc.erase(it);
    }
    if (parent_->child_in_focus_ == this) {
      parent_->child_in_focus_ = nullptr;
    }
  }

  if (app_) {
    app_->UnRegisterItem(this);
  }

  c.clear();
}

void Widget::AddChild(std::unique_ptr<Widget>&& Item) {
  children_.emplace_back(std::move(Item));
}

void Widget::RemoveChild(const Widget* Item) {
  auto it = std::find_if(
      children_.begin(), children_.end(),
      [Item](const std::unique_ptr<Widget>& si) { return si.get() == Item; });
  if (it != children_.end()) {
    std::unique_ptr<Widget> item;
    std::swap(item, *it);
    if (child_in_focus_ == Item) {
      child_in_focus_ = nullptr;
    }
    children_.erase(it);
    app_->AddToTrash(std::move(item));
  }
}

int32_t Widget::GetChildIndex(Widget* Item) {
  auto it = std::find_if(
      children_.begin(), children_.end(),
      [Item](std::unique_ptr<Widget>& i) { return i.get() == Item; });
  if (it == children_.end()) {
    return -1;
  }
  return std::distance(children_.begin(), it);
}

bool Widget::Initialize(Widget* parent, const Rect& position) {
  parent_ = parent;
  stored_content_size_ = position.Size();

  SetBorderSizes(0, 0, 0, 0);

  if (parent_) {
    app_ = parent_->GetApplication();
    if (app_) app_->RegisterItem(this);
  }

  ApplyPosition(position);

  return true;
}

Rect Widget::GetClientRect() const {
  return Rect(0, 0, client_rect_.Width(), client_rect_.Height());
}

Rect Widget::GetWindowRect() const {
  return position_ - (position_.TopLeft() + client_rect_.TopLeft());
}

Rect Widget::GetScreenRect() const { return screen_rect_; }

Point Widget::ClientToScreen(const Point& p) const {
  return screen_rect_.TopLeft() + client_rect_.TopLeft() + p;
}

Rect Widget::ClientToScreen(const Rect& p) const {
  return p + screen_rect_.TopLeft() + client_rect_.TopLeft();
}

Point Widget::ScreenToClient(const Point& p) const {
  return p - screen_rect_.TopLeft() - client_rect_.TopLeft();
}

Rect Widget::ScreenToClient(const Rect& p) const {
  return p - screen_rect_.TopLeft() - client_rect_.TopLeft();
}

CWBMessage Widget::BuildPositionMessage(const Rect& Pos) const {
  const bool moved = Pos.x1 != position_.x1 || Pos.y1 != position_.y1;
  const bool resized =
      Pos.Width() != position_.Width() || Pos.Height() != position_.Height();
  return CWBMessage(app_, WBM_REPOSITION, guid_, Pos, moved, resized);
}

CWBMessage Widget::BuildPositionMessage(const Rect& Pos, bool resized) const {
  const bool moved = Pos.x1 != position_.x1 || Pos.y1 != position_.y1;
  return CWBMessage(app_, WBM_REPOSITION, guid_, Pos, moved, resized);
}

void Widget::SetPosition(const Rect& Pos) {
  const CWBMessage m = BuildPositionMessage(Pos);
  app_->Send(m);
}

void Widget::SetClientPadding(int32_t left, int32_t top, int32_t right,
                              int32_t bottom) {
  if (left != WBMarginKeep) {
    GetPositionDescriptor().SetValue(WBPOSITIONTYPE::WB_PADDING_LEFT, 0,
                                     static_cast<float>(left));
  }
  if (right != WBMarginKeep) {
    GetPositionDescriptor().SetValue(WBPOSITIONTYPE::WB_PADDING_RIGHT, 0,
                                     static_cast<float>(right));
  }
  if (top != WBMarginKeep) {
    GetPositionDescriptor().SetValue(WBPOSITIONTYPE::WB_PADDING_TOP, 0,
                                     static_cast<float>(top));
  }
  if (bottom != WBMarginKeep) {
    GetPositionDescriptor().SetValue(WBPOSITIONTYPE::WB_PADDING_BOTTOM, 0,
                                     static_cast<float>(bottom));
  }

  CalculateClientPosition();
}

bool Widget::IsWidthSet() const { return GetPositionDescriptor().IsWidthSet(); }

bool Widget::IsHeightSet() const {
  return GetPositionDescriptor().IsHeightSet();
}

int32_t Widget::GetCalculatedWidth(Size ParentSize) const {
  return GetPositionDescriptor().GetWidth(ParentSize, stored_content_size_);
}

int32_t Widget::GetCalculatedHeight(Size ParentSize) const {
  return GetPositionDescriptor().GetHeight(ParentSize, stored_content_size_);
}

Rect Widget::GetPosition() const { return position_; }

bool Widget::InFocus() const {
  if (!parent_) return true;

  const Widget* fi = GetRoot();
  while (fi) {
    if (fi == this) return true;
    fi = fi->child_in_focus_;
  }
  return false;
}

bool Widget::InLocalFocus() const {
  if (!parent_) return true;
  return parent_->child_in_focus_ == this;
}

void Widget::SetFocus() {
  const Widget* fi = GetFocusItem();
  if (fi != this) {
    app_->Send(CWBMessage(app_, WBM_FOCUSGAINED, GetGuid()));
    if (fi) app_->Send(CWBMessage(app_, WBM_FOCUSLOST, fi->GetGuid()));
  }

  Widget* p = parent_;
  Widget* i = this;
  while (p) {
    p->child_in_focus_ = i;
    i = p;
    p = i->parent_;
  }
}

void Widget::ClearFocus() {
  // if (Parent) Parent->child_in_focus_ = NULL;
  app_->Send(CWBMessage(app_, WBM_FOCUSLOST, GetGuid()));
}

bool Widget::MouseOver() const {
  const Widget* mi = GetMouseItem();
  while (mi) {
    if (mi == this) return true;
    mi = mi->parent_;
  }
  return false;
}

void Widget::SavePosition() { stored_position_ = position_; }

Rect Widget::GetSavedPosition() const { return stored_position_; }

void Widget::SetSavedPosition(const Rect& savedPos) {
  stored_position_ = savedPos;
}

uint32_t Widget::NumChildren() const { return children_.size(); }

Widget* Widget::GetChild(uint32_t idx) { return children_[idx].get(); }

Size Widget::GetContentSize() { return GetClientRect().Size(); }

void Widget::Hide(bool Hide) {
  app_->Send(CWBMessage(app_, Hide ? WBM_HIDE : WBM_UNHIDE, GetGuid()));
}

bool Widget::IsHidden() { return hidden_; }

void Widget::SetData(void* data) { data_ = data; }

void* Widget::GetData() { return data_; }

void Widget::MarkForDeletion() { parent_->RemoveChild(this); }

ContextMenu* Widget::OpenContextMenu(Point pos) {
  if (!app_) return nullptr;
  auto ctx =
      ContextMenu::Create(GetRoot(), Rect(pos, pos + Point(10, 10)), GetGuid());
  app_->ApplyStyle(ctx);
  return ctx;
}

void Widget::ScrollbardisplayHelperFunct(const ScrollbarParams& s, int32_t* a1,
                                         int32_t* a2, int32_t* thumbsize,
                                         int32_t* thumbpos) {
  *a1 += scrollbar_button_size_;
  *a2 -= scrollbar_button_size_;

  const int32_t mi = s.min_scroll_;
  const int32_t ma = s.max_scroll_;

  const auto scrollsize = static_cast<float>(ma - mi);
  const float rs = std::max(0.0f, std::min(1.0f, s.view_size_ / scrollsize));
  const float rp = std::max(
      0.0f, std::min(1.0f, (s.scroll_pos_ - mi) / (scrollsize - s.view_size_)));

  *thumbsize = static_cast<int32_t>(std::max(
      static_cast<float>(scrollbar_thumb_minimal_size_), rs * (*a2 - *a1)));
  *thumbpos = static_cast<int32_t>((*a2 - *thumbsize - *a1) * rp) + *a1;
}

int32_t Widget::CalculateScrollbarMovement(const ScrollbarParams& s,
                                           int32_t scrollbarsize,
                                           int32_t delta) {
  int32_t a1 = 0;
  int32_t a2 = scrollbarsize;
  int32_t thumbsize = 0;
  int32_t thumbpos = 0;
  ScrollbardisplayHelperFunct(s, &a1, &a2, &thumbsize, &thumbpos);

  const int32_t mi = s.min_scroll_;
  const int32_t ma = s.max_scroll_;
  const auto scrollsize = static_cast<float>(ma - mi);

  const float sp =
      std::max(0.0f, std::min(1.0f, (s.drag_start_position_ - mi) /
                                        (scrollsize - s.view_size_)));
  const auto thumbposstart = static_cast<int32_t>((a2 - thumbsize - a1) * sp);

  const int32_t thumbposdelta =
      std::max(0, std::min(a2 - thumbsize - a1, thumbposstart + delta));
  int32_t newscrollpos =
      static_cast<int32_t>(
          (thumbposdelta / static_cast<float>(a2 - thumbsize - a1)) *
          (scrollsize - s.view_size_)) +
      mi;

  // invalid state
  if (a2 - thumbsize - a1 == 0) {
    newscrollpos = s.scroll_pos_;

    if (s.scroll_pos_ < s.min_scroll_ &&
        s.scroll_pos_ + s.view_size_ > s.max_scroll_) {
      newscrollpos = s.min_scroll_;
    } else if (s.scroll_pos_ < s.min_scroll_) {
      newscrollpos = s.min_scroll_;
    } else if (s.scroll_pos_ + s.view_size_ > s.max_scroll_) {
      newscrollpos = s.max_scroll_ - s.view_size_;
    }
    if (newscrollpos < s.min_scroll_) {
      newscrollpos = s.min_scroll_;
    }
  }

  return newscrollpos;
}

WBITEMSTATE Widget::GetScrollbarState(WBITEMVISUALCOMPONENT Component, Rect r) {
  // mouse not over item, early exit if dragging is not in effect
  const bool HBar =
      Component == WB_ITEM_SCROLL_HBAR || Component == WB_ITEM_SCROLL_HTHUMB ||
      Component == WB_ITEM_SCROLL_LEFT || Component == WB_ITEM_SCROLL_RIGHT;
  if (!MouseOver() &&
      ((HBar && HScrollbar.drag_mode_ == WB_SCROLLDRAG_NONE) ||
       (!HBar && VScrollbar.drag_mode_ == WB_SCROLLDRAG_NONE))) {
    return WB_STATE_NORMAL;
  }

  Point MousePos = GetMousePos();

  const Rect ScreenRect = ClientToScreen(r);
  const bool Hover = ScreenRect.Contains(MousePos);

  // don't highlight if something else uses the mouse (including this item)
  if (GetMouseCaptureItem()) {
    if (HScrollbar.drag_mode_ == WB_SCROLLDRAG_NONE &&
        VScrollbar.drag_mode_ == WB_SCROLLDRAG_NONE &&
        !GetMouseCaptureItem()->AllowMouseHighlightWhileCaptureItem()) {
      return WB_STATE_NORMAL;
    }
  }

  // scrollbar not clicked, return hover/normal mode
  if (HBar && HScrollbar.drag_mode_ == WB_SCROLLDRAG_NONE) {
    return Hover ? WB_STATE_HOVER : WB_STATE_NORMAL;
  }
  if (!HBar && VScrollbar.drag_mode_ == WB_SCROLLDRAG_NONE) {
    return Hover ? WB_STATE_HOVER : WB_STATE_NORMAL;
  }

  // handle clicked modes
  switch (Component) {
    case WB_ITEM_SCROLL_UP:
      if (VScrollbar.drag_mode_ == WB_SCROLLDRAG_BUTTON1) {
        return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
      }
      break;
    case WB_ITEM_SCROLL_DOWN:
      if (VScrollbar.drag_mode_ == WB_SCROLLDRAG_BUTTON2) {
        return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
      }
      break;
    case WB_ITEM_SCROLL_LEFT:
      if (HScrollbar.drag_mode_ == WB_SCROLLDRAG_BUTTON1) {
        return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
      }
      break;
    case WB_ITEM_SCROLL_RIGHT:
      if (HScrollbar.drag_mode_ == WB_SCROLLDRAG_BUTTON2) {
        return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
      }
      break;
    case WB_ITEM_SCROLL_HBAR:
      if (HScrollbar.drag_mode_ == WB_SCROLLDRAG_UP ||
          HScrollbar.drag_mode_ == WB_SCROLLDRAG_DOWN) {
        return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
      }
      if (HScrollbar.drag_mode_ == WB_SCROLLDRAG_THUMB) return WB_STATE_ACTIVE;
      break;
    case WB_ITEM_SCROLL_VBAR:
      if (VScrollbar.drag_mode_ == WB_SCROLLDRAG_UP ||
          VScrollbar.drag_mode_ == WB_SCROLLDRAG_DOWN) {
        return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
      }
      if (VScrollbar.drag_mode_ == WB_SCROLLDRAG_THUMB) return WB_STATE_ACTIVE;
      break;
    case WB_ITEM_SCROLL_HTHUMB:
      if (HScrollbar.drag_mode_ == WB_SCROLLDRAG_THUMB) return WB_STATE_ACTIVE;
      break;
    case WB_ITEM_SCROLL_VTHUMB:
      if (VScrollbar.drag_mode_ == WB_SCROLLDRAG_THUMB) return WB_STATE_ACTIVE;
      break;
    default:
      break;
  }

  return WB_STATE_NORMAL;
}

bool Widget::GetHScrollbarRectangles(Rect* button1, Rect* Scrollup, Rect* Thumb,
                                     Rect* Scrolldown, Rect* button2) {
  if (!HScrollbar.enabled_ || !HScrollbar.visible_) return false;

  Rect r = Rect(GetClientRect().BottomLeft(),
                GetClientRect().BottomRight() + Point(0, scrollbar_size_));
  *button1 =
      Rect(r.TopLeft(), r.BottomLeft() + Point(scrollbar_button_size_, 0));
  *button2 =
      Rect(r.TopRight() - Point(scrollbar_button_size_, 0), r.BottomRight());

  int32_t thumbsize = 0, thumbpos = 0;
  ScrollbardisplayHelperFunct(HScrollbar, &r.x1, &r.x2, &thumbsize, &thumbpos);
  if (ScrollbarRequired(HScrollbar)) {
    *Scrollup = Rect(r.x1, r.y1, thumbpos, r.y2);
    *Thumb = Rect(thumbpos, r.y1, thumbpos + thumbsize, r.y2);
    *Scrolldown = Rect(thumbpos + thumbsize, r.y1, r.x2, r.y2);
  } else {
    *Scrollup = *Thumb = *Scrolldown = Rect(1, 1, -1, -1);
  }

  return true;
}

void Widget::DrawScrollbarButton(DrawAPI* API, ScrollbarParams& s, Rect& r,
                                 WBITEMVISUALCOMPONENT Button) {
  const WBITEMSTATE State = GetScrollbarState(Button, r);
  const WBSKINELEMENTID ButtonSkin =
      GetDisplayDescriptor().GetSkin(State, Button);

  if (ButtonSkin == 0xffffffff) {
    CColor color{0xff999999};
    if (State == WB_STATE_HOVER) color = CColor{0xff1c97ea};
    if (State == WB_STATE_ACTIVE) color = CColor{0xff007acc};
    if (!ScrollbarRequired(s)) color = CColor{0xff555558};

    const Point margin = Point(4, 4);
    API->DrawRect(Rect(r.TopLeft() + margin, r.BottomRight() - margin), color);
  } else {
    app_->GetSkin()->RenderElement(API, ButtonSkin, r);
  }
}

void Widget::DrawHScrollbar(DrawAPI* API) {
  if (!HScrollbar.enabled_ || !HScrollbar.visible_) return;
  Rect b1, su, th, sd, b2;
  GetHScrollbarRectangles(&b1, &su, &th, &sd, &b2);

  Rect pr = API->GetParentCropRect();
  const Size RealClientRectSize =
      GetPositionDescriptor()
          .GetPadding(GetWindowRect().Size(), GetBorderSizes())
          .Size();
  Rect RealClientRect =
      ClientToScreen(Rect(0, 0, RealClientRectSize.x, RealClientRectSize.y)) |
      pr;
  API->SetParentCropRect(RealClientRect);
  API->SetCropRect(RealClientRect);

  // draw background
  const WBITEMSTATE BackgroundState =
      GetScrollbarState(WB_ITEM_SCROLL_HBAR, th);
  const Rect BackgroundRect = Rect(b1.TopLeft(), b2.BottomRight());
  const WBSKINELEMENTID Background =
      GetDisplayDescriptor().GetSkin(BackgroundState, WB_ITEM_SCROLL_HBAR);

  if (Background == 0xffffffff) {
    API->DrawRect(BackgroundRect, CColor{0xff3e3e42});
  } else {
    app_->GetSkin()->RenderElement(API, Background, BackgroundRect);
  }

  // draw thumb
  const WBITEMSTATE ThumbState = GetScrollbarState(WB_ITEM_SCROLL_HTHUMB, th);
  const WBSKINELEMENTID Thumb =
      GetDisplayDescriptor().GetSkin(ThumbState, WB_ITEM_SCROLL_HTHUMB);

  if (Thumb == 0xffffffff) {
    CColor color = CColor{0xff686868};
    if (ThumbState == WB_STATE_HOVER) color = CColor{0xff9e9e9e};
    if (ThumbState == WB_STATE_ACTIVE) color = CColor{0xffefebef};

    const Point thumbmargin = Point(0, 4);
    API->DrawRect(
        Rect(th.TopLeft() + thumbmargin, th.BottomRight() - thumbmargin),
        color);
  } else {
    app_->GetSkin()->RenderElement(API, Thumb, th);
  }

  // draw buttons
  DrawScrollbarButton(API, HScrollbar, b2, WB_ITEM_SCROLL_RIGHT);
  DrawScrollbarButton(API, HScrollbar, b1, WB_ITEM_SCROLL_LEFT);

  API->SetParentCropRect(pr);
}

bool Widget::GetVScrollbarRectangles(Rect* button1, Rect* Scrollup, Rect* Thumb,
                                     Rect* Scrolldown, Rect* button2) {
  if (!VScrollbar.enabled_ || !VScrollbar.visible_) return false;

  Rect r = Rect(GetClientRect().TopRight(),
                GetClientRect().BottomRight() + Point(scrollbar_size_, 0));
  *button1 = Rect(r.TopLeft(), r.TopRight() + Point(0, scrollbar_button_size_));
  *button2 =
      Rect(r.BottomLeft() - Point(0, scrollbar_button_size_), r.BottomRight());

  int32_t thumbsize = 0, thumbpos = 0;
  ScrollbardisplayHelperFunct(VScrollbar, &r.y1, &r.y2, &thumbsize, &thumbpos);
  if (ScrollbarRequired(VScrollbar)) {
    *Scrollup = Rect(r.x1, r.y1, r.x2, thumbpos);
    *Thumb = Rect(r.x1, thumbpos, r.x2, thumbpos + thumbsize);
    *Scrolldown = Rect(r.x1, thumbpos + thumbsize, r.x2, r.y2);
  } else {
    *Scrollup = *Thumb = *Scrolldown = Rect(1, 1, -1, -1);
  }

  return true;
}

void Widget::DrawVScrollbar(DrawAPI* API) {
  if (!VScrollbar.enabled_ || !VScrollbar.visible_) return;
  Rect b1, su, th, sd, b2;
  GetVScrollbarRectangles(&b1, &su, &th, &sd, &b2);

  Rect pr = API->GetParentCropRect();
  const Size RealClientRectSize =
      GetPositionDescriptor()
          .GetPadding(GetWindowRect().Size(), GetBorderSizes())
          .Size();
  Rect RealClientRect =
      ClientToScreen(Rect(0, 0, RealClientRectSize.x, RealClientRectSize.y)) |
      pr;
  API->SetParentCropRect(RealClientRect);
  API->SetCropRect(RealClientRect);

  // draw background
  const WBITEMSTATE BackgroundState =
      GetScrollbarState(WB_ITEM_SCROLL_VBAR, th);
  const Rect BackgroundRect = Rect(b1.TopLeft(), b2.BottomRight());
  const WBSKINELEMENTID Background =
      GetDisplayDescriptor().GetSkin(BackgroundState, WB_ITEM_SCROLL_VBAR);

  if (Background == 0xffffffff) {
    API->DrawRect(BackgroundRect, CColor{0xff3e3e42});
  } else {
    app_->GetSkin()->RenderElement(API, Background, BackgroundRect);
  }

  // draw thumb
  const WBITEMSTATE ThumbState = GetScrollbarState(WB_ITEM_SCROLL_VTHUMB, th);
  const WBSKINELEMENTID Thumb =
      GetDisplayDescriptor().GetSkin(ThumbState, WB_ITEM_SCROLL_VTHUMB);

  if (Thumb == 0xffffffff) {
    CColor color = CColor{0xff686868};
    if (ThumbState == WB_STATE_HOVER) color = CColor{0xff9e9e9e};
    if (ThumbState == WB_STATE_ACTIVE) color = CColor{0xffefebef};

    const Point thumbmargin = Point(4, 0);
    API->DrawRect(
        Rect(th.TopLeft() + thumbmargin, th.BottomRight() - thumbmargin),
        color);
  } else {
    app_->GetSkin()->RenderElement(API, Thumb, th);
  }

  // draw buttons
  DrawScrollbarButton(API, VScrollbar, b2, WB_ITEM_SCROLL_DOWN);
  DrawScrollbarButton(API, VScrollbar, b1, WB_ITEM_SCROLL_UP);

  API->SetParentCropRect(pr);
}

bool Widget::ScrollbarDragged() {
  return HScrollbar.drag_mode_ != WB_SCROLLDRAG_NONE ||
         VScrollbar.drag_mode_ != WB_SCROLLDRAG_NONE;
}

void Widget::ScrollbarHelperFunct(ScrollbarParams& s, int32_t* r,
                                  bool ScrollbarNeeded) {
  if (!s.enabled_) {
    if (s.visible_) {
      // remove scrollbar
      *r += scrollbar_size_;
      s.visible_ = false;
    }
  } else {
    if (!s.dynamic_) {
      if (!s.visible_) {
        // add scrollbar
        *r -= scrollbar_size_;
        s.visible_ = true;
      }
    } else {
      if (ScrollbarNeeded != s.visible_) {
        if (s.visible_) {
          // remove scrollbar
          *r += scrollbar_size_;
        } else {
          // add scrollbar
          *r -= scrollbar_size_;
        }
        s.visible_ = !s.visible_;
      }
    }
  }
}

bool Widget::ScrollbarRequired(const ScrollbarParams& s) {
  return s.view_size_ < s.max_scroll_ - s.min_scroll_ ||
         (s.scroll_pos_ < s.min_scroll_ &&
          s.scroll_pos_ + s.view_size_ < s.max_scroll_) ||
         (s.scroll_pos_ > s.max_scroll_ - s.view_size_ &&
          s.scroll_pos_ > s.min_scroll_);
}

void Widget::AdjustClientAreaToFitScrollbars() {
  Rect Rect = client_rect_;

  // x axis
  ScrollbarHelperFunct(VScrollbar, &Rect.x2, ScrollbarRequired(VScrollbar));

  // y axis
  ScrollbarHelperFunct(HScrollbar, &Rect.y2, ScrollbarRequired(HScrollbar));

  if (app_ && Rect != client_rect_) {
    app_->Send(CWBMessage(app_, WBM_CLIENTAREACHANGED, GetGuid()));
  }

  client_rect_ = Rect;
}

void Widget::EnableHScrollbar(bool Enabled, bool Dynamic) {
  HScrollbar.enabled_ = Enabled;
  HScrollbar.dynamic_ = Dynamic;
  AdjustClientAreaToFitScrollbars();
}

void Widget::EnableVScrollbar(bool Enabled, bool Dynamic) {
  VScrollbar.enabled_ = Enabled;
  VScrollbar.dynamic_ = Dynamic;
  AdjustClientAreaToFitScrollbars();
}

bool Widget::IsHScrollbarEnabled() { return HScrollbar.enabled_; }

bool Widget::IsVScrollbarEnabled() { return VScrollbar.enabled_; }

void Widget::SetHScrollbarParameters(int32_t MinScroll, int32_t MaxScroll,
                                     int32_t ViewSize) {
  const bool Changed = HScrollbar.min_scroll_ != MinScroll ||
                       HScrollbar.max_scroll_ != MaxScroll ||
                       HScrollbar.view_size_ != ViewSize;
  HScrollbar.min_scroll_ = MinScroll;
  HScrollbar.max_scroll_ = MaxScroll;
  HScrollbar.view_size_ = ViewSize;
  if (Changed) CalculateClientPosition();
}

void Widget::SetVScrollbarParameters(int32_t MinScroll, int32_t MaxScroll,
                                     int32_t ViewSize) {
  const bool Changed = VScrollbar.min_scroll_ != MinScroll ||
                       VScrollbar.max_scroll_ != MaxScroll ||
                       VScrollbar.view_size_ != ViewSize;
  VScrollbar.min_scroll_ = MinScroll;
  VScrollbar.max_scroll_ = MaxScroll;
  VScrollbar.view_size_ = ViewSize;
  if (Changed) CalculateClientPosition();
}

void Widget::GetHScrollbarParameters(int32_t* MinScroll, int32_t* MaxScroll,
                                     int32_t* ViewSize) {
  if (MinScroll) *MinScroll = HScrollbar.min_scroll_;
  if (MaxScroll) *MaxScroll = HScrollbar.max_scroll_;
  if (ViewSize) *ViewSize = HScrollbar.view_size_;
}

void Widget::GetVScrollbarParameters(int32_t* MinScroll, int32_t* MaxScroll,
                                     int32_t* ViewSize) {
  if (MinScroll) *MinScroll = VScrollbar.min_scroll_;
  if (MaxScroll) *MaxScroll = VScrollbar.max_scroll_;
  if (ViewSize) *ViewSize = VScrollbar.view_size_;
}

void Widget::SetHScrollbarPos(int32_t ScrollPos, bool Clamp) {
  int32_t sc = ScrollPos;
  if (Clamp) {
    sc = std::max(HScrollbar.min_scroll_,
                  std::min(sc, HScrollbar.max_scroll_ - HScrollbar.view_size_));
  }
  if (sc != HScrollbar.scroll_pos_) {
    HScrollbar.scroll_pos_ = sc;
    app_->Send(CWBMessage(app_, WBM_HSCROLL, GetGuid(), sc));
    CalculateClientPosition();
  }
}

void Widget::SetVScrollbarPos(int32_t ScrollPos, bool Clamp) {
  int32_t sc = ScrollPos;
  if (Clamp) {
    sc = std::max(VScrollbar.min_scroll_,
                  std::min(sc, VScrollbar.max_scroll_ - VScrollbar.view_size_));
  }
  if (sc != VScrollbar.scroll_pos_) {
    VScrollbar.scroll_pos_ = sc;
    app_->Send(CWBMessage(app_, WBM_VSCROLL, GetGuid(), sc));
    CalculateClientPosition();
  }
}

void Widget::ApplyRelativePosition() {
  if (parent_) {
    SetPosition(GetPositionDescriptor().GetPosition(
        parent_->GetClientRect().Size(), stored_content_size_, GetPosition()));
  }
}

void Widget::VisualStyleApplicator(DisplayProperties& desc,
                                   WBITEMVISUALCOMPONENT TargetComponent,
                                   int32_t Value,
                                   const std::vector<std::string>& pseudo) {
  int32_t StateCount = 0;
  for (size_t x = 1; x < pseudo.size(); x++) {
    const auto p = Trim(pseudo[x]);
    if (p == "active" || p == "hover" || p == "disabled" ||
        p == "disabled-active" || p == "normal") {
      StateCount++;
    }
  }

  if (!StateCount) {
    desc.SetValue(WB_STATE_NORMAL, TargetComponent, Value);
    desc.SetValue(WB_STATE_ACTIVE, TargetComponent, Value);
    desc.SetValue(WB_STATE_HOVER, TargetComponent, Value);
    desc.SetValue(WB_STATE_DISABLED, TargetComponent, Value);
    desc.SetValue(WB_STATE_DISABLED_ACTIVE, TargetComponent, Value);
  } else {
    for (size_t x = 1; x < pseudo.size(); x++) {
      const auto p = Trim(pseudo[x]);
      if (p == "active") {
        desc.SetValue(WB_STATE_ACTIVE, TargetComponent, Value);
        continue;
      }
      if (p == "hover") {
        desc.SetValue(WB_STATE_HOVER, TargetComponent, Value);
        continue;
      }
      if (p == "disabled") {
        desc.SetValue(WB_STATE_DISABLED, TargetComponent, Value);
        continue;
      }
      if (p == "disabled-active") {
        desc.SetValue(WB_STATE_DISABLED_ACTIVE, TargetComponent, Value);
        continue;
      }
      if (p == "normal") {
        desc.SetValue(WB_STATE_NORMAL, TargetComponent, Value);
        continue;
      }
    }
  }
}

bool Widget::InterpretPositionString(CssPropertyBatch& props,
                                     std::string_view prop,
                                     std::string_view value,
                                     const std::vector<std::string>& pseudo) {
  if (prop == "left") {
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_MARGIN_LEFT, value);
    return true;
  }

  if (prop == "right") {
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_MARGIN_RIGHT, value);
    return true;
  }

  if (prop == "top") {
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_MARGIN_TOP, value);
    return true;
  }

  if (prop == "bottom") {
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_MARGIN_BOTTOM, value);
    return true;
  }

  if (prop == "width") {
    PositionApplicator(props.position_descriptor_, WBPOSITIONTYPE::WB_WIDTH,
                       value);
    return true;
  }

  if (prop == "height") {
    PositionApplicator(props.position_descriptor_, WBPOSITIONTYPE::WB_HEIGHT,
                       value);
    return true;
  }

  if (prop == "padding-left") {
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_PADDING_LEFT, value);
    return true;
  }

  if (prop == "padding-right") {
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_PADDING_RIGHT, value);
    return true;
  }

  if (prop == "padding-top") {
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_PADDING_TOP, value);
    return true;
  }

  if (prop == "padding-bottom") {
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_PADDING_BOTTOM, value);
    return true;
  }

  if (prop == "margin") {
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_MARGIN_LEFT, value);
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_MARGIN_RIGHT, value);
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_MARGIN_TOP, value);
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_MARGIN_BOTTOM, value);
    return true;
  }

  if (prop == "padding") {
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_PADDING_LEFT, value);
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_PADDING_RIGHT, value);
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_PADDING_TOP, value);
    PositionApplicator(props.position_descriptor_,
                       WBPOSITIONTYPE::WB_PADDING_BOTTOM, value);
    return true;
  }

  int32_t dw = 0;

  if (prop == "border") {
    if (ScanPXValue(value, &dw, prop))
      props.border_sizes_ = Rect(dw, dw, dw, dw);
    return true;
  }
  if (prop == "border-left") {
    if (ScanPXValue(value, &dw, prop)) props.border_sizes_.x1 = dw;
    return true;
  }
  if (prop == "border-top") {
    if (ScanPXValue(value, &dw, prop)) props.border_sizes_.y1 = dw;
    return true;
  }
  if (prop == "border-right") {
    if (ScanPXValue(value, &dw, prop)) props.border_sizes_.x2 = dw;
    return true;
  }
  if (prop == "border-bottom") {
    if (ScanPXValue(value, &dw, prop)) props.border_sizes_.y2 = dw;
    return true;
  }

  return false;
}

std::vector<std::string> Widget::ExplodeValueWithoutSplittingParameters(
    std::string_view String) {
  std::vector<std::string> aOut;
  int nPrevious = 0;

  unsigned int x = 0;
  int32_t bracketcnt = 0;

  while (x < String.size()) {
    while (x < String.size() &&
           isspace(static_cast<unsigned char>(String[x]))) {
      x++;
    }

    if (String[x] == '(') bracketcnt++;
    if (String[x] == ')' && bracketcnt) bracketcnt--;

    nPrevious = x;
    while (x < String.size() &&
           (bracketcnt || (!isspace(static_cast<unsigned char>(String[x]))))) {
      if (String[x] == '(') bracketcnt++;
      if (String[x] == ')' && bracketcnt) bracketcnt--;
      x++;
    }

    aOut.emplace_back(String.substr(nPrevious, x - nPrevious));
  }

  return aOut;
}

bool Widget::InterpretDisplayString(CssPropertyBatch& props,
                                    std::string_view prop,
                                    std::string_view value,
                                    const std::vector<std::string>& pseudo) {
  if (prop == "background") {
    auto Attribs = ExplodeValueWithoutSplittingParameters(value);

    for (const auto& attrib : Attribs) {
      uint32_t dw = 0;
      if (std::sscanf(attrib.c_str(), "#%x", &dw) == 1) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDCOLOR, dw | 0xff000000, pseudo);
      }
      if (attrib == ("none")) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDCOLOR, 0, pseudo);
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDIMAGE, 0xffffffff, pseudo);
      }

      if (attrib == ("left")) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDALIGNMENT_X,
                              static_cast<int32_t>(WBALIGNMENT::LEFT), pseudo);
      }
      if (attrib == ("center")) {
        VisualStyleApplicator(
            props.display_descriptor_, WB_ITEM_BACKGROUNDALIGNMENT_X,
            static_cast<int32_t>(WBALIGNMENT::CENTER), pseudo);
      }
      if (attrib == ("right")) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDALIGNMENT_X,
                              static_cast<int32_t>(WBALIGNMENT::RIGHT), pseudo);
      }
      if (attrib == ("top")) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDALIGNMENT_Y,
                              static_cast<int32_t>(WBALIGNMENT::TOP), pseudo);
      }
      if (attrib == ("middle")) {
        VisualStyleApplicator(
            props.display_descriptor_, WB_ITEM_BACKGROUNDALIGNMENT_Y,
            static_cast<int32_t>(WBALIGNMENT::MIDDLE), pseudo);
      }
      if (attrib == ("bottom")) {
        VisualStyleApplicator(
            props.display_descriptor_, WB_ITEM_BACKGROUNDALIGNMENT_Y,
            static_cast<int32_t>(WBALIGNMENT::BOTTOM), pseudo);
      }

      if (attrib.find("rgba(") == 0) {
        CColor col;
        if (!ParseRGBA(attrib, &col)) {
          Log_Warn("[gui] CSS rgba() description invalid, skipping: {:s}",
                   attrib);
          continue;
        }
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDCOLOR, col.argb(), pseudo);
      }

      WBSKINELEMENTID id = 0;
      if (ScanSkinValue(attrib, &id, prop)) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDIMAGE, id, pseudo);
      }
    }

    return true;
  }

  if (prop == "background-color") {
    auto Attribs = ExplodeValueWithoutSplittingParameters(value);

    for (const auto& attrib : Attribs) {
      uint32_t dw = 0;
      if (std::sscanf(attrib.c_str(), "#%x", &dw) == 1) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDCOLOR, dw | 0xff000000, pseudo);
      }
      if (attrib == ("none")) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDCOLOR, 0, pseudo);
      }

      if (attrib.find("rgba(") == 0) {
        CColor col;
        if (!ParseRGBA(attrib, &col)) {
          Log_Warn("[gui] CSS rgba() description invalid, skipping: {:s}",
                   attrib);
          continue;
        }
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDCOLOR, col.argb(), pseudo);
      }
    }

    return true;
  }

  if (prop == "foreground-color") {
    auto Attribs = ExplodeValueWithoutSplittingParameters(value);

    for (const auto& attrib : Attribs) {
      uint32_t dw = 0;
      if (std::sscanf(attrib.c_str(), "#%x", &dw) == 1) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_FOREGROUNDCOLOR, dw | 0xff000000, pseudo);
      }
      if (attrib == ("none")) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_FOREGROUNDCOLOR, 0, pseudo);
      }

      if (attrib.find("rgba(") == 0) {
        CColor col;
        if (!ParseRGBA(attrib, &col)) {
          Log_Warn("[gui] CSS rgba() description invalid, skipping: {:s}",
                   attrib);
          continue;
        }
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_FOREGROUNDCOLOR, col.argb(), pseudo);
      }
    }

    return true;
  }

  if (prop == "background-position") {
    auto Attribs = SplitByWhitespace(value);

    for (const auto& attrib : Attribs) {
      if (attrib == ("left")) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDALIGNMENT_X,
                              static_cast<int32_t>(WBALIGNMENT::LEFT), pseudo);
      }
      if (attrib == ("center")) {
        VisualStyleApplicator(
            props.display_descriptor_, WB_ITEM_BACKGROUNDALIGNMENT_X,
            static_cast<int32_t>(WBALIGNMENT::CENTER), pseudo);
      }
      if (attrib == ("right")) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDALIGNMENT_X,
                              static_cast<int32_t>(WBALIGNMENT::RIGHT), pseudo);
      }
      if (attrib == ("top")) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDALIGNMENT_Y,
                              static_cast<int32_t>(WBALIGNMENT::TOP), pseudo);
      }
      if (attrib == ("middle")) {
        VisualStyleApplicator(
            props.display_descriptor_, WB_ITEM_BACKGROUNDALIGNMENT_Y,
            static_cast<int32_t>(WBALIGNMENT::MIDDLE), pseudo);
      }
      if (attrib == ("bottom")) {
        VisualStyleApplicator(
            props.display_descriptor_, WB_ITEM_BACKGROUNDALIGNMENT_Y,
            static_cast<int32_t>(WBALIGNMENT::BOTTOM), pseudo);
      }
    }

    return true;
  }

  if (prop == "background-image") {
    auto Attribs = ExplodeValueWithoutSplittingParameters(value);

    for (const auto& attrib : Attribs) {
      if (attrib == ("none")) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDIMAGE, 0xffffffff, pseudo);
      }

      WBSKINELEMENTID id = 0;
      if (ScanSkinValue(attrib, &id, prop)) {
        VisualStyleApplicator(props.display_descriptor_,
                              WB_ITEM_BACKGROUNDIMAGE, id, pseudo);
      }
    }

    return true;
  }

  if (prop == "border-color") {
    uint32_t dw = 0;
    std::string v(value);
    std::sscanf(v.c_str(), "#%x", &dw);
    VisualStyleApplicator(props.display_descriptor_, WB_ITEM_BORDERCOLOR,
                          dw | 0xff000000, pseudo);
    return true;
  }

  if (prop == "opacity") {
    float dw = 0;
    std::string v(value);
    std::sscanf(v.c_str(), "%f", &dw);

    const int32_t o =
        static_cast<int32_t>(std::max(0.f, std::min(255.f, dw * 255)));

    VisualStyleApplicator(props.display_descriptor_, WB_ITEM_OPACITY,
                          o * 0x01010101, pseudo);
    return true;
  }

  // quick check for scrollbar related stuff
  if (prop[0] == 's') {
    WBSKINELEMENTID id = 0;

    if (prop == "scrollbar-up") {
      if (ScanSkinValue(value, &id, prop)) {
        VisualStyleApplicator(props.display_descriptor_, WB_ITEM_SCROLL_UP, id,
                              pseudo);
      }
      return true;
    }
    if (prop == "scrollbar-down") {
      if (ScanSkinValue(value, &id, prop)) {
        VisualStyleApplicator(props.display_descriptor_, WB_ITEM_SCROLL_DOWN,
                              id, pseudo);
      }
      return true;
    }
    if (prop == "scrollbar-left") {
      if (ScanSkinValue(value, &id, prop)) {
        VisualStyleApplicator(props.display_descriptor_, WB_ITEM_SCROLL_LEFT,
                              id, pseudo);
      }
      return true;
    }
    if (prop == "scrollbar-right") {
      if (ScanSkinValue(value, &id, prop)) {
        VisualStyleApplicator(props.display_descriptor_, WB_ITEM_SCROLL_RIGHT,
                              id, pseudo);
      }
      return true;
    }
    if (prop == "scrollbar-background-horizontal") {
      if (ScanSkinValue(value, &id, prop)) {
        VisualStyleApplicator(props.display_descriptor_, WB_ITEM_SCROLL_HBAR,
                              id, pseudo);
      }
      return true;
    }
    if (prop == "scrollbar-background-vertical") {
      if (ScanSkinValue(value, &id, prop)) {
        VisualStyleApplicator(props.display_descriptor_, WB_ITEM_SCROLL_VBAR,
                              id, pseudo);
      }
      return true;
    }
    if (prop == "scrollbar-thumb-horizontal") {
      if (ScanSkinValue(value, &id, prop)) {
        VisualStyleApplicator(props.display_descriptor_, WB_ITEM_SCROLL_HTHUMB,
                              id, pseudo);
      }
      return true;
    }
    if (prop == "scrollbar-thumb-vertical") {
      if (ScanSkinValue(value, &id, prop)) {
        VisualStyleApplicator(props.display_descriptor_, WB_ITEM_SCROLL_VTHUMB,
                              id, pseudo);
      }
      return true;
    }
  }

  if (InterpretFontString(props, prop, value, pseudo)) return true;

  return false;
}

bool Widget::InterpretFontString(CssPropertyBatch& props, std::string_view prop,
                                 std::string_view value,
                                 const std::vector<std::string>& pseudo) {
  if (prop == "font-color") {
    uint32_t dw = 0;
    std::string v(value);
    if (std::sscanf(v.c_str(), "#%x", &dw) == 1) {
      VisualStyleApplicator(props.display_descriptor_, WB_ITEM_FONTCOLOR,
                            dw | 0xff000000, pseudo);
      return true;
    }

    CColor col;
    if (ParseRGBA(value, &col)) {
      VisualStyleApplicator(props.display_descriptor_, WB_ITEM_FONTCOLOR,
                            col.argb(), pseudo);
    }

    return true;
  }

  if (prop == "text-transform") {
    if (value == ("none")) {
      VisualStyleApplicator(props.display_descriptor_, WB_ITEM_TEXTTRANSFORM,
                            static_cast<int32_t>(WBTEXTTRANSFORM::WBTT_NONE),
                            pseudo);
    }
    if (value == ("capitalize")) {
      VisualStyleApplicator(
          props.display_descriptor_, WB_ITEM_TEXTTRANSFORM,
          static_cast<int32_t>(WBTEXTTRANSFORM::WBTT_CAPITALIZE), pseudo);
    }
    if (value == ("uppercase")) {
      VisualStyleApplicator(
          props.display_descriptor_, WB_ITEM_TEXTTRANSFORM,
          static_cast<int32_t>(WBTEXTTRANSFORM::WBTT_UPPERCASE), pseudo);
    }
    if (value == ("lowercase")) {
      VisualStyleApplicator(
          props.display_descriptor_, WB_ITEM_TEXTTRANSFORM,
          static_cast<int32_t>(WBTEXTTRANSFORM::WBTT_LOWERCASE), pseudo);
    }

    return true;
  }

  if (prop == "font") {
    auto Attribs = ExplodeValueWithoutSplittingParameters(value);

    for (const auto& attrib : Attribs) {
      // try to apply as color
      uint32_t dw = 0;
      if (std::sscanf(attrib.c_str(), "#%x", &dw) == 1) {
        VisualStyleApplicator(props.display_descriptor_, WB_ITEM_FONTCOLOR,
                              dw | 0xff000000, pseudo);
        continue;
      }

      CColor col;
      if (ParseRGBA(attrib, &col)) {
        VisualStyleApplicator(props.display_descriptor_, WB_ITEM_FONTCOLOR,
                              col.argb(), pseudo);
        continue;
      }

      // if failed apply as font
      FontStyleApplicator(props, pseudo, attrib);
    }

    return true;
  }

  if (prop == "font-family") {
    FontStyleApplicator(props, pseudo, value);
    return true;
  }

  if (prop == "text-align") {
    if (value == "left") props.text_align_x_ = WBTEXTALIGNMENTX::WBTA_LEFT;
    if (value == "center") props.text_align_x_ = WBTEXTALIGNMENTX::WBTA_CENTERX;
    if (value == "right") props.text_align_x_ = WBTEXTALIGNMENTX::WBTA_RIGHT;
    return true;
  }

  if (prop == "vertical-align") {
    if (value == "top") props.text_align_y_ = WBTEXTALIGNMENTY::WBTA_TOP;
    if (value == "middle") props.text_align_y_ = WBTEXTALIGNMENTY::WBTA_CENTERY;
    if (value == "bottom") props.text_align_y_ = WBTEXTALIGNMENTY::WBTA_BOTTOM;
    return true;
  }

  return false;
}

bool Widget::ApplyStyle(std::string_view prop, std::string_view value,
                        const std::vector<std::string>& pseudo) {
  if (InterpretPositionString(css_properties_, prop, value, pseudo)) {
    ContentChanged();
    return true;
  }
  if (InterpretDisplayString(css_properties_, prop, value, pseudo)) {
    ContentChanged();
    return true;
  }

  if (prop == "visibility") {
    if (value == "hidden") {
      hidden_ = true;
      return true;
    }
    if (value == "visible") {
      hidden_ = false;
      return true;
    }
    Log_Warn("[guiitem] Item style error: invalid visibility value '{:s}'",
             value);
    return true;
  }

  if (prop == "overflow") {
    if (value == "hidden") {
      EnableHScrollbar(false, false);
      EnableVScrollbar(false, false);
      return true;
    }

    if (value == "auto") {
      EnableHScrollbar(true, true);
      EnableVScrollbar(true, true);
      return true;
    }

    if (value == "scroll") {
      EnableHScrollbar(true, false);
      EnableVScrollbar(true, false);
      return true;
    }
  }

  if (prop == "overflow-x") {
    if (value == "hidden") {
      EnableHScrollbar(false, false);
      return true;
    }

    if (value == "auto") {
      EnableHScrollbar(true, true);
      return true;
    }

    if (value == "scroll") {
      EnableHScrollbar(true, false);
      return true;
    }
  }

  if (prop == "overflow-y") {
    if (value == "hidden") {
      EnableVScrollbar(false, false);
      return true;
    }

    if (value == "auto") {
      EnableVScrollbar(true, true);
      return true;
    }

    if (value == "scroll") {
      EnableVScrollbar(true, false);
      return true;
    }
  }

  // quick check for scrollbar related stuff
  if (prop[0] == 's') {
    int32_t dw = 0;

    if (prop == "scrollbar-size") {
      if (ScanPXValue(value, &dw, prop)) scrollbar_size_ = dw;
      return true;
    }

    if (prop == "scrollbar-button-size") {
      if (ScanPXValue(value, &dw, prop)) scrollbar_button_size_ = dw;
      return true;
    }

    if (prop == "scrollbar-thumb-minimum-size") {
      if (ScanPXValue(value, &dw, prop)) scrollbar_thumb_minimal_size_ = dw;
      return true;
    }
  }

  return false;
}

void Widget::PositionApplicator(PositionDescriptor& pos, WBPOSITIONTYPE Type,
                                std::string_view value) {
  if (Type == WBPOSITIONTYPE::WB_WIDTH || Type == WBPOSITIONTYPE::WB_HEIGHT) {
    if (value == "none") {
      pos.ClearMetrics(Type);
      return;
    }
    if (value == "auto") {
      pos.ClearMetrics(Type);
      pos.SetAutoSize(Type);
      return;
    }

  } else if (value == "auto") {
    pos.ClearMetrics(Type);
    return;
  }

  const bool px = value.find("px") != std::string_view::npos;
  const bool pc = value.find('%') != std::string_view::npos;

  float pxv = 0;
  float pcv = 0;

  if (!pc && !px) {
    Log_Warn("[guiitem] Item style error: missing 'px' or '%' in '{:s}' value",
             value);
    return;
  }

  std::string v(value);
  if (px && !pc) {
    if (std::sscanf(v.c_str(), "%fpx", &pxv) != 1) {
      Log_Warn("[guiitem] Item style error: invalid value '{:s}' (px)", v);
      return;
    }
    pos.ClearMetrics(Type);
    pos.SetMetric(Type, WBMETRICTYPE::WB_PIXELS, pxv);
    return;
  }

  if (pc && !px) {
    if (std::sscanf(v.c_str(), "%f%%", &pcv) != 1) {
      Log_Warn("[guiitem] Item style error: invalid value '{:s}' (%)", v);
      return;
    }
    pos.ClearMetrics(Type);
    pos.SetMetric(Type, WBMETRICTYPE::WB_RELATIVE, pcv / 100.0f);
    return;
  }

  if (std::sscanf(v.c_str(), "%fpx%f%%", &pxv, &pcv) != 2) {
    if (std::sscanf(v.c_str(), "%f%%%fpx", &pcv, &pxv) != 2) {
      Log_Warn("[guiitem] Item style error: invalid value '{:s}' (px, %)", v);
      return;
    }
  }

  pos.ClearMetrics(Type);
  pos.SetMetric(Type, WBMETRICTYPE::WB_PIXELS, pxv);
  pos.SetMetric(Type, WBMETRICTYPE::WB_RELATIVE, pcv / 100.0f);
}

Widget* Widget::GetRoot() { return app_->GetRoot(); }
const Widget* Widget::GetRoot() const { return app_->GetRoot(); }
Widget* Widget::GetFocusItem() { return app_->GetFocusItem(); }
const Widget* Widget::GetFocusItem() const { return app_->GetFocusItem(); }

Widget* Widget::FindChildByID(std::string_view value, std::string_view type) {
  Widget* i = ChildSearcherFunct(value, type);
  return i;
}

Widget* Widget::FindParentByID(std::string_view value,
                               std::string_view type) const {
  Widget* i = GetParent();
  while (i) {
    if (i->GetID() == value) {
      if (type.empty()) return i;
      if (i->InstanceOf(type)) return i;
    }
    i = i->GetParent();
  }
  return nullptr;
}

Widget* Widget::ChildSearcherFunct(std::string_view value,
                                   std::string_view type) {
  if (GetID() == value) {
    if (type.empty()) {
      return this;
    }
    if (InstanceOf(type)) {
      return this;
    }
  }

  for (auto& c : children_) {
    Widget* i = c->ChildSearcherFunct(value, type);
    if (i) {
      return i;
    }
  }

  return nullptr;
}

void Widget::ApplyStyleDeclarations(std::string_view String) {
  if (!app_) return;
  app_->style_manager_.ApplyStylesFromDeclarations(this, String);
}

WBITEMSTATE Widget::GetState() {
  WBITEMSTATE i = WB_STATE_NORMAL;
  if (disabled_) i = WB_STATE_DISABLED;
  if (MouseOver() && !disabled_) i = WB_STATE_HOVER;

  if (i == WB_STATE_HOVER && GetMouseCaptureItem()) {
    if (GetMouseCaptureItem() != this &&
        !GetMouseCaptureItem()->AllowMouseHighlightWhileCaptureItem()) {
      i = WB_STATE_NORMAL;
    }
  }

  if (InFocus()) i = disabled_ ? WB_STATE_DISABLED_ACTIVE : WB_STATE_ACTIVE;
  return i;
}

void Widget::SetBorderSizes(char Left, char Top, char Right, char Bottom) {
  css_properties_.border_sizes_ = Rect(Left, Top, Right, Bottom);
}

void Widget::SetDisplayProperty(WBITEMSTATE s, WBITEMVISUALCOMPONENT v,
                                int32_t value) {
  GetDisplayDescriptor().SetValue(s, v, value);
}

Font* Widget::GetFont(WBITEMSTATE State) {
  return css_properties_.GetFont(app_, State);
}

void Widget::ApplyOpacity(DrawAPI* API) {
  CColor o = GetDisplayDescriptor().GetColor(GetState(), WB_ITEM_OPACITY);
  API->SetOpacity(static_cast<uint8_t>(o.A() * opacity_multiplier_));
}

Size Widget::GetClientWindowSizeDifference() {
  const Rect w = GetWindowRect();
  const Rect c = GetClientRect();

  return Size(w.Width() - c.Width(), w.Height() - c.Height());
}

void Widget::Enable(bool Enabled) { disabled_ = !Enabled; }

bool Widget::IsEnabled() { return !disabled_; }

void Widget::SetChildInFocus(Widget* i) {
  if (!i) {
    child_in_focus_ = nullptr;
    return;
  }
  if (i->parent_ == this) child_in_focus_ = i;
}

bool Widget::ParseRGBA(std::string_view description, CColor* output) {
  auto Params = Split(description, ",");
  if (Params.size() < 3 || Params.size() > 4) return false;

  int32_t result = 0;
  int32_t c[3];
  result += std::sscanf(Params[0].c_str(), "rgba(%d", &c[0]);
  result += std::sscanf(Params[1].c_str(), "%d", &c[1]);
  result += std::sscanf(Params[2].c_str(), "%d", &c[2]);

  if (result < 3) return false;

  float a = 1;

  if (Params.size() == 4) {
    if (std::sscanf(Params[3].c_str(), "%f", &a) != 1) return false;
  }

  uint8_t Colors[3];
  for (int32_t y = 0; y < 3; y++) Colors[y] = std::max(0, std::min(255, c[y]));
  const uint8_t Alpha =
      static_cast<int32_t>(std::max(0.f, std::min(1.f, a)) * 255.f);

  if (output) *output = CColor(Colors[0], Colors[1], Colors[2], Alpha);
  return true;
}

void Widget::FontStyleApplicator(CssPropertyBatch& desc,
                                 const std::vector<std::string>& pseudo,
                                 std::string_view name) {
  if (pseudo.size() <= 1) {
    desc.fonts_.clear();
    desc.fonts_[WB_STATE_NORMAL] = name;
  } else {
    for (size_t y = 1; y < pseudo.size(); y++) {
      const auto p = Trim(pseudo[y]);
      if (p == "active") {
        desc.fonts_[WB_STATE_ACTIVE] = name;
        continue;
      }
      if (p == "hover") {
        desc.fonts_[WB_STATE_HOVER] = name;
        continue;
      }
      if (p == "disabled") {
        desc.fonts_[WB_STATE_DISABLED] = name;
        continue;
      }
      if (p == "disabled-active") {
        desc.fonts_[WB_STATE_DISABLED_ACTIVE] = name;
        continue;
      }
      if (p == "normal") {
        desc.fonts_[WB_STATE_NORMAL] = name;
        continue;
      }
    }
  }
}

bool Widget::ScanPXValue(std::string_view Value, int32_t* Result,
                         std::string_view PropName) {
  *Result = 0;
  std::string v(Value);
  if (std::sscanf(v.c_str(), "%dpx", Result) != 1) {
    Log_Warn("[guiitem] Item style error: invalid {:s} value '{:s}' (px)",
             PropName, Value);
    return false;
  }
  return true;
}

bool Widget::ScanSkinValue(std::string_view Value, WBSKINELEMENTID* Result,
                           std::string_view PropName) {
  if (Value.find("skin(") == 0) {
    const int32_t i = Value.find(')');
    if (i != std::string_view::npos) {
      // Value.GetPointer()[ i ] = 0;
      *Result = app_->GetSkin()->GetElementID(Value.substr(5, i - 5));
      if (*Result == 0xffffffff) {
        Log_Warn("[gui] Skin element not found: {:s}", Value.substr(5, i - 5));
        return false;
      }

      return true;
    }
  }
  return false;
}

void Widget::SetFont(WBITEMSTATE State, std::string_view Font) {
  css_properties_.fonts_[State] = Font;
}

void Widget::ContentChanged() {
  if (!GetPositionDescriptor().IsAutoResizer()) return;

  const Size ParentSize = GetWindowRect().Size();
  const Size ClientSize =
      GetPositionDescriptor()
          .GetPadding(GetWindowRect().Size(), GetBorderSizes())
          .Size();

  stored_content_size_ = GetContentSize() + ParentSize - ClientSize;

  CWBMessage m(app_, WBM_REPOSITION, guid_,
               Rect(GetPosition().TopLeft(),
                    GetPosition().TopLeft() + stored_content_size_),
               true, true);
  app_->Send(m);
}

void Widget::ChangeContentOffset(Point ContentOff) {
  if (ContentOff == content_offset_) return;
  content_offset_ = ContentOff;
  for (uint32_t x = 0; x < NumChildren(); x++) GetChild(x)->UpdateScreenRect();
}

void Widget::ChangeContentOffsetX(int32_t OffsetX) {
  app_->Send(CWBMessage(app_, WBM_CONTENTOFFSETCHANGE, GetGuid(), OffsetX,
                        content_offset_.y));
}

void Widget::ChangeContentOffsetY(int32_t OffsetY) {
  app_->Send(CWBMessage(app_, WBM_CONTENTOFFSETCHANGE, GetGuid(),
                        content_offset_.x, OffsetY));
}

bool Widget::ScrollbarsEnabled() {
  return HScrollbar.enabled_ || VScrollbar.enabled_;
}

void Widget::SetTreeOpacityMultiplier(float OpacityMul) {
  opacity_multiplier_ = OpacityMul;
  for (auto& c : children_) {
    c->SetTreeOpacityMultiplier(OpacityMul);
  }
}

float Widget::GetTreeOpacityMultiplier() { return opacity_multiplier_; }

void Widget::ReapplyStyles() {
  app_->style_manager_.ApplyStyles(this);
  CWBMessage m = BuildPositionMessage(GetPosition(), true);
  MessageProc(m);
}

void Widget::SetForcedMouseTransparency(bool transparent) {
  force_mouse_transparent_ = transparent;
}

bool Widget::MarkedForDeletion() {
  std::scoped_lock l(app_->trash_mutex_);
  auto it = std::find_if(
      app_->trash_.begin(), app_->trash_.end(),
      [this](const std::unique_ptr<Widget>& i) { return i.get() == this; });
  return (it != app_->trash_.end());
}

const Widget* Widget::GetMouseItem() const { return app_->GetMouseItem(); }

const Widget* Widget::GetMouseCaptureItem() const {
  return app_->GetMouseCaptureItem();
}

const math::Point Widget::GetMousePos() const { return app_->GetMousePos(); }

void Widget::SelectMouseCursor(renderer::MouseCursor cursor) {
  app_->SelectMouseCursor(cursor);
}

}  // namespace gui
