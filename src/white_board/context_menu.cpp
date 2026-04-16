module;

#include <algorithm>
#include <memory>
#include <string>

module whiteboard;

import :application;
import :button;
import :context_menu;
import :font;

using math::Point;
using math::Rect;
using math::Size;

namespace gui {

void CWBContextMenu::OnDraw(CWBDrawAPI* API) {
  const WBITEMSTATE i = GetState();
  CWBFont* Font = GetFont(i);
  const auto TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));

  DrawBackground(API, WB_STATE_NORMAL);

  const Rect Client = GetClientRect();
  const Rect padding = CSSProperties.PositionDescriptor.GetPadding(
      Client.Size(), Rect(0, 0, 0, 0));

  Point Offset = Point(padding.x1, padding.y1);

  for (int32_t x = 0; x < Items.size(); x++) {
    if (Items[x]->Separator) {
      constexpr int arbitraryValue = 200;

      const Rect padding = SeparatorElements.PositionDescriptor.GetPadding(
          Size(GetWindowRect().Width(), arbitraryValue), Rect(0, 0, 0, 0));
      Offset.y += padding.y1;

      const int height = SeparatorElements.PositionDescriptor.GetHeight(
          Size(0, 0), Size(0, 0));

      const Rect separatorRect =
          Rect(GetWindowRect().x1 + padding.x1, Offset.y,
                GetWindowRect().x1 + padding.x2, Offset.y + height);

      const WBSKINELEMENTID id = SeparatorElements.DisplayDescriptor.GetSkin(
          WB_STATE_NORMAL, WB_ITEM_BACKGROUNDIMAGE);

      // draw separator
      if (id != 0xffffffff) {
        App->GetSkin()->RenderElement(API, id, separatorRect);
      } else {
        API->DrawRect(separatorRect, CSSProperties.DisplayDescriptor.GetColor(
                                         i, WB_ITEM_BORDERCOLOR));
      }

      Offset.y += height;
      Offset.y += arbitraryValue - padding.y2;
    } else if (Font) {
      // draw entry
      Rect EntryPos = GetItemRect(x);
      EntryPos.x1 += padding.x1;
      EntryPos.x2 -= Client.Width() - padding.x2;

      CColor textColor = CSSProperties.DisplayDescriptor.GetColor(
          WB_STATE_NORMAL, WB_ITEM_FONTCOLOR);

      if (Items[x]->Highlighted) {
        textColor = CSSProperties.DisplayDescriptor.GetColor(WB_STATE_ACTIVE,
                                                             WB_ITEM_FONTCOLOR);
      }

      // highlight background
      if (MouseOver() &&
          EntryPos.Contains(ScreenToClient(App->GetMousePos()))) {
        API->DrawRect(EntryPos, CSSProperties.DisplayDescriptor.GetColor(
                                    WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR));
        textColor = CSSProperties.DisplayDescriptor.GetColor(WB_STATE_HOVER,
                                                             WB_ITEM_FONTCOLOR);
      }

      Font->Write(API, Items[x]->Text, Offset, textColor, TextTransform);

      // draw sub arrow
      if (!Items[x]->Children.empty()) {
        if (CSSProperties.DisplayDescriptor.GetSkin(i, WB_ITEM_SUBSKIN) !=
            0xffffffff) {
        } else {
          const int32_t wi = Font->GetWidth(">");
          Font->Write(
              API, ">",
              Point(GetWindowRect().x2 - wi - CSSProperties.BorderSizes.x2 -
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

CWBContextMenu::CWBContextMenu(WBGUID trg) : CWBGuiType(), Target(trg) {}

CWBContextMenu::~CWBContextMenu() {
  if (ParentMenu) {
    ParentMenu->SubMenu = nullptr;
    ParentMenu->SubMenuIdx = -1;
  }
}

void CWBContextMenu::ResizeToContentSize() {
  const WBITEMSTATE i = GetState();
  CWBFont* Font = GetFont(i);
  const auto TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));

  Size ContentSize = Size(0, 0);

  bool NeedsSubArrow = false;
  for (const auto& item : Items) {
    if (!item->Children.empty()) {
      NeedsSubArrow = true;
    }
  }
  constexpr int arbitraryValue = 0;
  Rect padding = SeparatorElements.PositionDescriptor.GetPadding(
      Size(0, arbitraryValue), Rect(0, 0, 0, 0));
  const int height =
      SeparatorElements.PositionDescriptor.GetHeight(Size(0, 0), Size(0, 0));
  const int separatorHeight = padding.y1 + height + arbitraryValue - padding.y2;

  const Rect Client = GetClientRect();
  padding = CSSProperties.PositionDescriptor.GetPadding(Client.Size(),
                                                        Rect(0, 0, 0, 0));

  ContentSize.y = padding.y1;

  for (const auto& item : Items) {
    if (item->Separator) {
      ContentSize.y += separatorHeight;
    } else if (Font) {
      int32_t arrowPadding = 0;

      // account for sub arrow
      if (!item->Children.empty()) {
        if (CSSProperties.DisplayDescriptor.GetSkin(i, WB_ITEM_SUBSKIN) !=
            0xffffffff) {
        } else {
          arrowPadding = Font->GetWidth(">");
        }
      }

      ContentSize.x = std::max(
          ContentSize.x, padding.x1 + Client.x2 - padding.x2 + arrowPadding +
                             Font->GetWidth(item->Text, true, TextTransform));
      ContentSize.y += Font->GetLineHeight();
    }
  }

  ContentSize.y += Client.Size().y - padding.y2;

  const Rect r = GetPosition();
  const Size Size = ContentSize + GetClientWindowSizeDifference();
  SetPosition(Rect(r.TopLeft(), r.TopLeft() + Size));
}

bool CWBContextMenu::Initialize(CWBItem* Parent, const Rect& Position) {
  if (!CWBItem::Initialize(Parent, Position)) return false;

  SetClientPadding(2, 0, 2, 0);
  SetBorderSizes(1, 1, 1, 1);

  CSSProperties.DisplayDescriptor.SetValue(WB_STATE_NORMAL,
                                           WB_ITEM_BACKGROUNDCOLOR, 0xff1b1b1c);
  CSSProperties.DisplayDescriptor.SetValue(WB_STATE_HOVER,
                                           WB_ITEM_BACKGROUNDCOLOR, 0xff434346);

  SeparatorElements.PositionDescriptor.SetValue(WBPOSITIONTYPE::WB_PADDING_TOP,
                                                0, 1);
  SeparatorElements.PositionDescriptor.SetValue(WBPOSITIONTYPE::WB_HEIGHT, 0,
                                                1);
  SeparatorElements.PositionDescriptor.SetValue(
      WBPOSITIONTYPE::WB_PADDING_BOTTOM, 0, 1);

  App->SetCapture(this);
  return true;
}

CWBContextItem* CWBContextMenu::AddItem(std::string_view Text, int32_t ID,
                                        bool Highlighted, bool closesContext) {
  Items.emplace_back(std::make_unique<CWBContextItem>());
  auto& item = Items.back();
  item->Text = Text;
  item->ReturnID = ID;
  item->Highlighted = Highlighted;
  item->closesContext = closesContext;
  ResizeToContentSize();

  return item.get();
}

void CWBContextMenu::AddSeparator() {
  Items.emplace_back(std::make_unique<CWBContextItem>());
  Items.back()->Separator = true;
}

bool CWBContextMenu::MessageProc(const CWBMessage& Message) {
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
        CWBMessage m(App, WBM_REPOSITION, Message.GetTarget(), p,
                     Message.Moved(), Message.Resized());
        return CWBItem::MessageProc(m);
      }
      break;
    case WBM_MOUSEMOVE:
      if (MouseOver()) {
        for (int32_t x = 0; x < Items.size(); x++) {
          if (!Items[x]->Separator) {
            const Rect EntryPos = GetItemRect(x);
            if (EntryPos.Contains(ScreenToClient(App->GetMousePos()))) {
              SpawnSubMenu(x);
              return true;
            }
          }
        }
      }
      if (SubMenu) SubMenu->MessageProc(Message);
      return true;

    case WBM_RIGHTBUTTONDOWN:
    case WBM_LEFTBUTTONDOWN:
    case WBM_MIDDLEBUTTONDOWN:
      // need to close the context menu and resend the click
      if (!MouseInContextHierarchy() && !MarkedForDeletion()) {
        MarkForDeletion();
        MarkParentForDeletion();
        App->ReleaseCapture();
        // resend message
        App->Send(Message);
        return true;
      }

      Pushed = true;
      App->SetCapture(GetContextRoot());
      if (!MouseOver()) {
        if (SubMenu) SubMenu->MessageProc(Message);
      }
      return true;

    case WBM_RIGHTBUTTONUP:
    case WBM_LEFTBUTTONUP:

      if (!Pushed) {
        // original click doesn't count
        return true;
      }

      // check if any of the items are clicked on
      for (int32_t x = 0; x < Items.size(); x++) {
        if (!Items[x]->Separator) {
          const Rect EntryPos = GetItemRect(x);
          if (MouseOver() &&
              EntryPos.Contains(ScreenToClient(App->GetMousePos()))) {
            App->Send(CWBMessage(App, WBM_CONTEXTMESSAGE, Target,
                                 Items[x]->ReturnID));
            if (Items[x]->closesContext) {
              MarkForDeletion();
              MarkParentForDeletion();
              App->ReleaseCapture();
            } else {
              App->Send(CWBMessage(
                  App, WBM_REBUILDCONTEXTITEM, Target,
                  CWBMessage::menucontext{Items[x]->ReturnID, GetGuid()}));
            }

            return true;
          }
        }
      }

      if (MouseInContextHierarchy()) {
        if (SubMenu) SubMenu->MessageProc(Message);
      }

      // check for off item mouse releases
      if (!MouseInContextHierarchy() && Pushed) {
        MarkForDeletion();
        MarkParentForDeletion();
        App->ReleaseCapture();
      }

      return true;

    default:
      break;
  }

  return CWBItem::MessageProc(Message);
}

void CWBContextMenu::MarkForDeletion() {
  if (ParentMenu) {
    ParentMenu->SubMenu = nullptr;
    ParentMenu->SubMenuIdx = -1;
    ParentMenu = nullptr;
  }
  if (SubMenu) {
    SubMenu->MarkForDeletion();
  }
  CWBItem::MarkForDeletion();
}

void CWBContextMenu::SpawnSubMenu(int32_t itemidx) {
  if (itemidx < 0 || itemidx >= Items.size()) return;
  if (itemidx == SubMenuIdx) {
    return;
  }
  if (SubMenu) {
    SubMenu->MarkForDeletion();
    SubMenu = nullptr;
    SubMenuIdx = -1;
  }
  if (Items[itemidx]->Children.size() <= 0) return;

  const Rect p = GetItemRect(itemidx) + GetPosition().TopLeft();
  const Rect w = GetPosition();

  const Rect newpos = Rect(w.x2 - 1, p.y1, w.x2 + 10, p.y1 + 10);

  // We need to select the parent as we are restricted to draw within the parent
  // rect.
  auto sm = CWBContextMenu::Create(GetParent(), newpos, Target);
  SubMenu = sm;
  SubMenuIdx = itemidx;
  App->ApplyStyle(SubMenu);
  SubMenu->ParentMenu = this;
  // message control must stay with the root item
  App->SetCapture(GetContextRoot());

  for (auto& child : Items[itemidx]->Children) {
    CWBContextItem* olditm = child.get();
    CWBContextItem* newitm = SubMenu->AddItem(olditm->Text, olditm->ReturnID);
    newitm->CopyOf = olditm;
    newitm->Separator = olditm->Separator;
    newitm->Highlighted = olditm->Highlighted;
    newitm->closesContext = olditm->closesContext;
    newitm->CopyChildrenFrom(olditm);
  }

  SubMenu->ResizeToContentSize();
}

Rect CWBContextMenu::GetItemRect(int32_t idx) {
  Point Offset = Point(0, 0);

  constexpr int arbitraryValue = 0;
  Rect padding = SeparatorElements.PositionDescriptor.GetPadding(
      Size(0, arbitraryValue), Rect(0, 0, 0, 0));
  const int height =
      SeparatorElements.PositionDescriptor.GetHeight(Size(0, 0), Size(0, 0));
  const int separatorHeight = padding.y1 + height + arbitraryValue - padding.y2;

  const Rect Client = GetClientRect();
  padding = CSSProperties.PositionDescriptor.GetPadding(Client.Size(),
                                                        Rect(0, 0, 0, 0));

  Offset.y = padding.y1;

  CWBFont* Font = GetFont(WB_STATE_HOVER);
  for (int32_t x = 0; x < Items.size(); x++) {
    if (Items[x]->Separator) {
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

void CWBContextMenu::MarkParentForDeletion() {
  if (ParentMenu) {
    ParentMenu->MarkForDeletion();
    ParentMenu->MarkParentForDeletion();
  }
}

bool CWBContextMenu::MouseInContextHierarchy() {
  if (!SubMenu) return MouseOver();
  return MouseOver() || SubMenu->MouseInContextHierarchy();
}

CWBContextMenu* CWBContextMenu::GetContextRoot() {
  if (!ParentMenu) return this;
  return ParentMenu->GetContextRoot();
}

CWBContextItem::CWBContextItem()
    : Text(""),
      ReturnID(0),
      Separator(false),
      Highlighted(false),
      closesContext(false) {}

CWBContextItem::~CWBContextItem() = default;

void CWBContextItem::CopyChildrenFrom(CWBContextItem* itm) {
  for (auto& child : itm->Children) {
    CWBContextItem* olditm = child.get();
    CWBContextItem* newitm = AddItem(olditm->Text, olditm->ReturnID);
    newitm->Separator = olditm->Separator;
    newitm->Highlighted = olditm->Highlighted;
    newitm->closesContext = olditm->closesContext;
    newitm->CopyChildrenFrom(olditm);
    newitm->CopyOf = olditm;
  }
}

CWBContextItem* CWBContextItem::AddItem(std::string_view Text, int32_t ID,
                                        bool Highlighted, bool closesContext) {
  Children.emplace_back(std::make_unique<CWBContextItem>());
  auto& item = Children.back();
  item->Text = Text;
  item->ReturnID = ID;
  item->Highlighted = Highlighted;
  item->closesContext = closesContext;
  return item.get();
}

void CWBContextItem::AddSeparator() {
  Children.emplace_back(std::make_unique<CWBContextItem>());
  Children.back()->Separator = true;
}

void CWBContextItem::SetText(std::string_view text) {
  Text = text;
  if (CopyOf) CopyOf->SetText(text);
}

void CWBContextItem::SetHighlight(bool highlighted) {
  Highlighted = highlighted;
  if (CopyOf) CopyOf->SetHighlight(highlighted);
}

bool CWBContextMenu::ApplyStyle(std::string_view prop, std::string_view value,
                                const std::vector<std::string>& pseudo) {
  bool ElementTarget = false;

  for (size_t x = 1; x < pseudo.size(); x++) {
    if (pseudo[x] == "separator") {
      ElementTarget = true;
      break;
    }
  }

  // apply font styling to list item anyway
  if (!ElementTarget) InterpretFontString(CSSProperties, prop, value, pseudo);
  if (!ElementTarget) return CWBItem::ApplyStyle(prop, value, pseudo);

  bool Handled = false;

  for (size_t x = 1; x < pseudo.size(); x++) {
    if (pseudo[x] == "separator") {
      Handled |= SeparatorElements.ApplyStyle(this, prop, value, pseudo);
      continue;
    }
  }

  return Handled;
}

CWBContextItem* CWBContextMenu::GetItem(int32_t ID) {
  for (const auto& item : Items) {
    if (item->ReturnID == ID) return item.get();
  }
  return nullptr;
}

}  // namespace gui
