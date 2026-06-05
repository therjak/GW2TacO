module;

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "src/base/color.h"

module whiteboard;

import :application;
import :button;
import :context_menu;
import :font;

using math::Point;
using math::Rect;
using math::Size;

namespace gui {

void ContextMenu::OnDraw(DrawAPI* API) {
  const WBITEMSTATE i = GetState();
  Font* Font = GetFont(i);
  const auto TextTransform = static_cast<WBTEXTTRANSFORM>(
      GetDisplayDescriptor().GetValue(i, WB_ITEM_TEXTTRANSFORM));

  DrawBackground(API, WB_STATE_NORMAL);

  const Rect Client = GetClientRect();
  const Rect padding =
      GetPositionDescriptor().GetPadding(Client.Size(), Rect(0, 0, 0, 0));

  Point Offset = Point(padding.x1, padding.y1);

  for (int32_t x = 0; x < items_.size(); x++) {
    if (items_[x]->separator_) {
      constexpr int arbitraryValue = 200;

      const Rect padding = separator_elements_.position_descriptor_.GetPadding(
          Size(GetWindowRect().Width(), arbitraryValue), Rect(0, 0, 0, 0));
      Offset.y += padding.y1;

      const int height = separator_elements_.position_descriptor_.GetHeight(
          Size(0, 0), Size(0, 0));

      const Rect separatorRect =
          Rect(GetWindowRect().x1 + padding.x1, Offset.y,
               GetWindowRect().x1 + padding.x2, Offset.y + height);

      const WBSKINELEMENTID id =
          separator_elements_.display_descriptor_.GetSkin(
              WB_STATE_NORMAL, WB_ITEM_BACKGROUNDIMAGE);

      // draw separator
      if (id != 0xffffffff) {
        GetApplication()->GetSkin()->RenderElement(API, id, separatorRect);
      } else {
        API->DrawRect(separatorRect,
                      GetDisplayDescriptor().GetColor(i, WB_ITEM_BORDERCOLOR));
      }

      Offset.y += height;
      Offset.y += arbitraryValue - padding.y2;
    } else if (Font) {
      // draw entry
      Rect EntryPos = GetItemRect(x);
      EntryPos.x1 += padding.x1;
      EntryPos.x2 -= Client.Width() - padding.x2;

      CColor textColor =
          GetDisplayDescriptor().GetColor(WB_STATE_NORMAL, WB_ITEM_FONTCOLOR);

      if (items_[x]->highlighted_) {
        textColor =
            GetDisplayDescriptor().GetColor(WB_STATE_ACTIVE, WB_ITEM_FONTCOLOR);
      }

      // highlight background
      if (MouseOver() && EntryPos.Contains(ScreenToClient(GetMousePos()))) {
        API->DrawRect(EntryPos, GetDisplayDescriptor().GetColor(
                                    WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR));
        textColor =
            GetDisplayDescriptor().GetColor(WB_STATE_HOVER, WB_ITEM_FONTCOLOR);
      }

      Font->Write(API, items_[x]->text_, Offset, textColor, TextTransform);

      // draw sub arrow
      if (!items_[x]->children_.empty()) {
        if (GetDisplayDescriptor().GetSkin(i, WB_ITEM_SUBSKIN) != 0xffffffff) {
        } else {
          const int32_t wi = Font->GetWidth(">");
          Font->Write(API, ">",
                      Point(GetWindowRect().x2 - wi - GetBorderSizes().x2 -
                                (Client.x2 - padding.x2),
                            Offset.y),
                      textColor);
        }
      }

      Offset.y += Font->GetLineHeight();
    }
  }

  DrawBorder(API);
}

ContextMenu::ContextMenu(WBGUID trg) : GuiType(), target_(trg) {}

ContextMenu::~ContextMenu() {
  if (parent_menu_) {
    parent_menu_->sub_menu_ = nullptr;
    parent_menu_->sub_menu_idx_ = -1;
  }
}

void ContextMenu::ResizeToContentSize() {
  const WBITEMSTATE i = GetState();
  Font* Font = GetFont(i);
  const auto TextTransform = static_cast<WBTEXTTRANSFORM>(
      GetDisplayDescriptor().GetValue(i, WB_ITEM_TEXTTRANSFORM));

  Size ContentSize = Size(0, 0);

  bool NeedsSubArrow = false;
  for (const auto& item : items_) {
    if (!item->children_.empty()) {
      NeedsSubArrow = true;
    }
  }
  constexpr int arbitraryValue = 0;
  Rect padding = separator_elements_.position_descriptor_.GetPadding(
      Size(0, arbitraryValue), Rect(0, 0, 0, 0));
  const int height = separator_elements_.position_descriptor_.GetHeight(
      Size(0, 0), Size(0, 0));
  const int separatorHeight = padding.y1 + height + arbitraryValue - padding.y2;

  const Rect Client = GetClientRect();
  padding = GetPositionDescriptor().GetPadding(Client.Size(), Rect(0, 0, 0, 0));

  ContentSize.y = padding.y1;

  for (const auto& item : items_) {
    if (item->separator_) {
      ContentSize.y += separatorHeight;
    } else if (Font) {
      int32_t arrowPadding = 0;

      // account for sub arrow
      if (!item->children_.empty()) {
        if (GetDisplayDescriptor().GetSkin(i, WB_ITEM_SUBSKIN) != 0xffffffff) {
        } else {
          arrowPadding = Font->GetWidth(">");
        }
      }

      ContentSize.x = std::max(
          ContentSize.x, padding.x1 + Client.x2 - padding.x2 + arrowPadding +
                             Font->GetWidth(item->text_, true, TextTransform));
      ContentSize.y += Font->GetLineHeight();
    }
  }

  ContentSize.y += Client.Size().y - padding.y2;

  const Rect r = GetPosition();
  const Size Size = ContentSize + GetClientWindowSizeDifference();
  SetPosition(Rect(r.TopLeft(), r.TopLeft() + Size));
}

bool ContextMenu::Initialize(Widget* Parent, const Rect& Position) {
  if (!Widget::Initialize(Parent, Position)) return false;

  SetClientPadding(2, 0, 2, 0);
  SetBorderSizes(1, 1, 1, 1);

  GetDisplayDescriptor().SetValue(WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR,
                                  0xff1b1b1c);
  GetDisplayDescriptor().SetValue(WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR,
                                  0xff434346);

  separator_elements_.position_descriptor_.SetValue(
      WBPOSITIONTYPE::WB_PADDING_TOP, 0, 1);
  separator_elements_.position_descriptor_.SetValue(WBPOSITIONTYPE::WB_HEIGHT,
                                                    0, 1);
  separator_elements_.position_descriptor_.SetValue(
      WBPOSITIONTYPE::WB_PADDING_BOTTOM, 0, 1);

  SetCapture();
  return true;
}

ContextItem* ContextMenu::AddItem(std::string_view Text, int32_t ID,
                                  bool Highlighted, bool closesContext) {
  items_.emplace_back(std::make_unique<ContextItem>());
  auto& item = items_.back();
  item->text_ = Text;
  item->return_id_ = ID;
  item->highlighted_ = Highlighted;
  item->closes_context_ = closesContext;
  ResizeToContentSize();

  return item.get();
}

void ContextMenu::AddSeparator() {
  items_.emplace_back(std::make_unique<ContextItem>());
  items_.back()->separator_ = true;
}

bool ContextMenu::MessageProc(const CWBMessage& Message) {
  switch (Message.Get()) {
    case WBM_REPOSITION:
      if (Message.GetTarget() == GetGuid()) {
        // push position inside of parent
        Rect p = Message.Rectangle();
        const Rect r = GetParent()->GetClientRect();
        if (p.x1 < 0) p += Point(-p.x1, 0);
        if (p.y1 < 0) p += Point(0, -p.y1);
        if (p.x2 > r.x2) p -= Point(p.x2 - r.x2, 0);
        if (p.y2 > r.y2) p -= Point(0, p.y2 - r.y2);
        CWBMessage m(GetApplication(), WBM_REPOSITION, Message.GetTarget(), p,
                     Message.Moved(), Message.Resized());
        return Widget::MessageProc(m);
      }
      break;
    case WBM_MOUSEMOVE:
      if (MouseOver()) {
        for (int32_t x = 0; x < items_.size(); x++) {
          if (!items_[x]->separator_) {
            const Rect EntryPos = GetItemRect(x);
            if (EntryPos.Contains(ScreenToClient(GetMousePos()))) {
              SpawnSubMenu(x);
              return true;
            }
          }
        }
      }
      if (sub_menu_) sub_menu_->MessageProc(Message);
      return true;

    case WBM_RIGHTBUTTONDOWN:
    case WBM_LEFTBUTTONDOWN:
    case WBM_MIDDLEBUTTONDOWN:
      // need to close the context menu and resend the click
      if (!MouseInContextHierarchy() && !MarkedForDeletion()) {
        MarkForDeletion();
        MarkParentForDeletion();
        GetApplication()->ReleaseCapture();
        // resend message
        GetApplication()->Send(Message);
        return true;
      }

      pushed_ = true;
      GetApplication()->SetCapture(GetContextRoot());
      if (!MouseOver()) {
        if (sub_menu_) sub_menu_->MessageProc(Message);
      }
      return true;

    case WBM_RIGHTBUTTONUP:
    case WBM_LEFTBUTTONUP:

      if (!pushed_) {
        // original click doesn't count
        return true;
      }

      // check if any of the items are clicked on
      for (int32_t x = 0; x < items_.size(); x++) {
        if (!items_[x]->separator_) {
          const Rect EntryPos = GetItemRect(x);
          if (MouseOver() && EntryPos.Contains(ScreenToClient(GetMousePos()))) {
            GetApplication()->Send(CWBMessage(GetApplication(),
                                              WBM_CONTEXTMESSAGE, target_,
                                              items_[x]->return_id_));
            if (items_[x]->closes_context_) {
              MarkForDeletion();
              MarkParentForDeletion();
              GetApplication()->ReleaseCapture();
            } else {
              GetApplication()->Send(CWBMessage(
                  GetApplication(), WBM_REBUILDCONTEXTITEM, target_,
                  CWBMessage::menucontext{items_[x]->return_id_, GetGuid()}));
            }

            return true;
          }
        }
      }

      if (MouseInContextHierarchy()) {
        if (sub_menu_) sub_menu_->MessageProc(Message);
      }

      // check for off item mouse releases
      if (!MouseInContextHierarchy() && pushed_) {
        MarkForDeletion();
        MarkParentForDeletion();
        GetApplication()->ReleaseCapture();
      }

      return true;

    default:
      break;
  }

  return Widget::MessageProc(Message);
}

void ContextMenu::MarkForDeletion() {
  if (parent_menu_) {
    parent_menu_->sub_menu_ = nullptr;
    parent_menu_->sub_menu_idx_ = -1;
    parent_menu_ = nullptr;
  }
  if (sub_menu_) {
    sub_menu_->MarkForDeletion();
  }
  Widget::MarkForDeletion();
}

void ContextMenu::SpawnSubMenu(int32_t itemidx) {
  if (itemidx < 0 || itemidx >= items_.size()) return;
  if (itemidx == sub_menu_idx_) {
    return;
  }
  if (sub_menu_) {
    sub_menu_->MarkForDeletion();
    sub_menu_ = nullptr;
    sub_menu_idx_ = -1;
  }
  if (items_[itemidx]->children_.size() <= 0) return;

  const Rect p = GetItemRect(itemidx) + GetPosition().TopLeft();
  const Rect w = GetPosition();

  const Rect newpos = Rect(w.x2 - 1, p.y1, w.x2 + 10, p.y1 + 10);

  // We need to select the parent as we are restricted to draw within the parent
  // rect.
  auto sm = ContextMenu::Create(GetParent(), newpos, target_);
  sub_menu_ = sm;
  sub_menu_idx_ = itemidx;
  GetApplication()->ApplyStyle(sub_menu_);
  sub_menu_->parent_menu_ = this;
  // message control must stay with the root item
  GetApplication()->SetCapture(GetContextRoot());

  for (auto& child : items_[itemidx]->children_) {
    ContextItem* olditm = child.get();
    ContextItem* newitm = sub_menu_->AddItem(olditm->text_, olditm->return_id_);
    newitm->copy_of_ = olditm;
    newitm->separator_ = olditm->separator_;
    newitm->highlighted_ = olditm->highlighted_;
    newitm->closes_context_ = olditm->closes_context_;
    newitm->CopyChildrenFrom(olditm);
  }

  sub_menu_->ResizeToContentSize();
}

Rect ContextMenu::GetItemRect(int32_t idx) {
  Point Offset = Point(0, 0);

  constexpr int arbitraryValue = 0;
  Rect padding = separator_elements_.position_descriptor_.GetPadding(
      Size(0, arbitraryValue), Rect(0, 0, 0, 0));
  const int height = separator_elements_.position_descriptor_.GetHeight(
      Size(0, 0), Size(0, 0));
  const int separatorHeight = padding.y1 + height + arbitraryValue - padding.y2;

  const Rect Client = GetClientRect();
  padding = GetPositionDescriptor().GetPadding(Client.Size(), Rect(0, 0, 0, 0));

  Offset.y = padding.y1;

  Font* Font = GetFont(WB_STATE_HOVER);
  for (int32_t x = 0; x < items_.size(); x++) {
    if (items_[x]->separator_) {
      Offset.y += separatorHeight;
    } else if (Font) {
      const Rect EntryPos(GetWindowRect().x1, Offset.y, GetWindowRect().x2,
                          Offset.y + Font->GetLineHeight());
      if (x == idx) return EntryPos;
      Offset.y += Font->GetLineHeight();
    }
  }

  return Rect(0, 0, 0, 0);
}

void ContextMenu::MarkParentForDeletion() {
  if (parent_menu_) {
    parent_menu_->MarkForDeletion();
    parent_menu_->MarkParentForDeletion();
  }
}

bool ContextMenu::MouseInContextHierarchy() {
  if (!sub_menu_) return MouseOver();
  return MouseOver() || sub_menu_->MouseInContextHierarchy();
}

ContextMenu* ContextMenu::GetContextRoot() {
  if (!parent_menu_) return this;
  return parent_menu_->GetContextRoot();
}

ContextItem::ContextItem()
    : text_(""),
      return_id_(0),
      separator_(false),
      highlighted_(false),
      closes_context_(false) {}

ContextItem::~ContextItem() = default;

void ContextItem::CopyChildrenFrom(ContextItem* itm) {
  for (auto& child : itm->children_) {
    ContextItem* olditm = child.get();
    ContextItem* newitm = AddItem(olditm->text_, olditm->return_id_);
    newitm->separator_ = olditm->separator_;
    newitm->highlighted_ = olditm->highlighted_;
    newitm->closes_context_ = olditm->closes_context_;
    newitm->CopyChildrenFrom(olditm);
    newitm->copy_of_ = olditm;
  }
}

ContextItem* ContextItem::AddItem(std::string_view Text, int32_t ID,
                                  bool Highlighted, bool closesContext) {
  children_.emplace_back(std::make_unique<ContextItem>());
  auto& item = children_.back();
  item->text_ = Text;
  item->return_id_ = ID;
  item->highlighted_ = Highlighted;
  item->closes_context_ = closesContext;
  return item.get();
}

void ContextItem::AddSeparator() {
  children_.emplace_back(std::make_unique<ContextItem>());
  children_.back()->separator_ = true;
}

void ContextItem::SetText(std::string_view text) {
  text_ = text;
  if (copy_of_) copy_of_->SetText(text);
}

void ContextItem::SetHighlight(bool highlighted) {
  highlighted_ = highlighted;
  if (copy_of_) copy_of_->SetHighlight(highlighted);
}

bool ContextMenu::ApplyStyle(std::string_view prop, std::string_view value,
                             const std::vector<std::string>& pseudo) {
  bool ElementTarget = false;

  for (size_t x = 1; x < pseudo.size(); x++) {
    if (pseudo[x] == "separator") {
      ElementTarget = true;
      break;
    }
  }

  // apply font styling to list item anyway
  if (!ElementTarget)
    InterpretFontString(GetCSSProperties(), prop, value, pseudo);
  if (!ElementTarget) return Widget::ApplyStyle(prop, value, pseudo);

  bool Handled = false;

  for (size_t x = 1; x < pseudo.size(); x++) {
    if (pseudo[x] == "separator") {
      Handled |= separator_elements_.ApplyStyle(this, prop, value, pseudo);
      continue;
    }
  }

  return Handled;
}

ContextItem* ContextMenu::GetItem(int32_t ID) {
  for (const auto& item : items_) {
    if (item->return_id_ == ID) return item.get();
  }
  return nullptr;
}

}  // namespace gui
