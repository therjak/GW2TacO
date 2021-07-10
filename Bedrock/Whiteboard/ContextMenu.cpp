#include "BasePCH.h"
#include "Application.h"
#include "ContextMenu.h"
#include "Button.h"

void CWBContextMenu::OnDraw(CWBDrawAPI *API) {
  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont(i);
  WBTEXTTRANSFORM TextTransform =
      (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue(
          i, WB_ITEM_TEXTTRANSFORM);

  DrawBackground(API, WB_STATE_NORMAL);

  CRect Client = GetClientRect();
  CRect padding = CSSProperties.PositionDescriptor.GetPadding(
      Client.Size(), CRect(0, 0, 0, 0));

  CPoint Offset = CPoint(padding.x1, padding.y1);

  for (int32_t x = 0; x < Items.NumItems(); x++) {
    if (Items[x]->Separator) {
      int arbitraryValue = 200;

      CRect padding = SeparatorElements.PositionDescriptor.GetPadding(
          CSize(GetWindowRect().Width(), arbitraryValue), CRect(0, 0, 0, 0));
      Offset.y += padding.y1;

      int height = SeparatorElements.PositionDescriptor.GetHeight(CSize(0, 0),
                                                                  CSize(0, 0));

      CRect separatorRect =
          CRect(GetWindowRect().x1 + padding.x1, Offset.y,
                GetWindowRect().x1 + padding.x2, Offset.y + height);

      WBSKINELEMENTID id = SeparatorElements.DisplayDescriptor.GetSkin(
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
      CRect EntryPos = GetItemRect(x);
      EntryPos.x1 += padding.x1;
      EntryPos.x2 -= Client.Width() - padding.x2;

      CColor textColor = CSSProperties.DisplayDescriptor.GetColor(
          WB_STATE_NORMAL, WB_ITEM_FONTCOLOR);

      if (Items[x]->Highlighted)
        textColor = CSSProperties.DisplayDescriptor.GetColor(WB_STATE_ACTIVE,
                                                             WB_ITEM_FONTCOLOR);

      // highlight background
      if (MouseOver() &&
          EntryPos.Contains(ScreenToClient(App->GetMousePos()))) {
        API->DrawRect(EntryPos, CSSProperties.DisplayDescriptor.GetColor(
                                    WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR));
        textColor = CSSProperties.DisplayDescriptor.GetColor(WB_STATE_HOVER,
                                                             WB_ITEM_FONTCOLOR);
      }

      int32_t width =
          Font->Write(API, Items[x]->Text, Offset, textColor, TextTransform);

      if (Items[x]->Children.NumItems())  // draw sub arrow
      {
        if (CSSProperties.DisplayDescriptor.GetSkin(i, WB_ITEM_SUBSKIN) !=
            0xffffffff) {
        } else {
          int32_t wi = Font->GetWidth(_T( ">" ));
          Font->Write(
              API, _T( ">" ),
              CPoint(GetWindowRect().x2 - wi - CSSProperties.BorderSizes.x2 -
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

CWBContextMenu::CWBContextMenu() : CWBItem() {
  Target = NULL;
  Pushed = false;
}

CWBContextMenu::CWBContextMenu(CWBItem *Parent, const CRect &Pos, WBGUID trg)
    : CWBItem() {
  CWBContextMenu::Initialize(Parent, Pos, trg);
}

CWBContextMenu::~CWBContextMenu() {
  if (ParentMenu) ParentMenu->SubMenu = NULL;
  Items.FreeArray();
  SAFEDELETE(SubMenu);
}

void CWBContextMenu::ResizeToContentSize() {
  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont(i);
  WBTEXTTRANSFORM TextTransform =
      (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue(
          i, WB_ITEM_TEXTTRANSFORM);

  CSize ContentSize = CSize(0, 0);

  TBOOL NeedsSubArrow = false;
  for (int32_t x = 0; x < Items.NumItems(); x++)
    if (Items[x]->Children.NumItems()) NeedsSubArrow = true;

  int arbitraryValue = 0;
  CRect padding = SeparatorElements.PositionDescriptor.GetPadding(
      CSize(0, arbitraryValue), CRect(0, 0, 0, 0));
  int height =
      SeparatorElements.PositionDescriptor.GetHeight(CSize(0, 0), CSize(0, 0));
  int separatorHeight = padding.y1 + height + arbitraryValue - padding.y2;

  CRect Client = GetClientRect();
  padding = CSSProperties.PositionDescriptor.GetPadding(Client.Size(),
                                                        CRect(0, 0, 0, 0));

  ContentSize.y = padding.y1;

  for (int32_t x = 0; x < Items.NumItems(); x++) {
    if (Items[x]->Separator) {
      ContentSize.y += separatorHeight;
    } else if (Font) {
      int32_t arrowPadding = 0;

      if (Items[x]->Children.NumItems())  // account for sub arrow
      {
        if (CSSProperties.DisplayDescriptor.GetSkin(i, WB_ITEM_SUBSKIN) !=
            0xffffffff) {
        } else {
          arrowPadding = Font->GetWidth(_T( ">" ));
        }
      }

      ContentSize.x =
          max(ContentSize.x,
              padding.x1 + Client.x2 - padding.x2 + arrowPadding +
                  Font->GetWidth(Items[x]->Text, true, TextTransform));
      ContentSize.y += Font->GetLineHeight();
    }
  }

  ContentSize.y += Client.Size().y - padding.y2;

  CRect r = GetPosition();
  CSize Size = ContentSize + GetClientWindowSizeDifference();
  SetPosition(CRect(r.TopLeft(), r.TopLeft() + Size));
}

TBOOL CWBContextMenu::Initialize(CWBItem *Parent, const CRect &Position,
                                 WBGUID trg) {
  ParentMenu = NULL;
  SubMenu = NULL;
  Pushed = false;
  Target = trg;
  if (!CWBItem::Initialize(Parent, Position)) return false;

  SetClientPadding(2, 0, 2, 0);
  SetBorderSizes(1, 1, 1, 1);

  CSSProperties.DisplayDescriptor.SetValue(
      WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB(0xff1b1b1c));
  CSSProperties.DisplayDescriptor.SetValue(
      WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB(0xff434346));

  SeparatorElements.PositionDescriptor.SetValue(WB_PADDING_TOP, 0, 1);
  SeparatorElements.PositionDescriptor.SetValue(WB_HEIGHT, 0, 1);
  SeparatorElements.PositionDescriptor.SetValue(WB_PADDING_BOTTOM, 0, 1);

  App->SetCapture(this);
  return true;
}

CWBContextItem *CWBContextMenu::AddItem(std::string_view Text, int32_t ID,
                                        TBOOL Highlighted,
                                        TBOOL closesContext) {
  CWBContextItem *Item = new CWBContextItem();
  Item->Text = Text;
  Item->ReturnID = ID;
  Item->Highlighted = Highlighted;
  Item->closesContext = closesContext;
  Items += Item;
  ResizeToContentSize();

  return Item;
}

void CWBContextMenu::AddSeparator() {
  Items += new CWBContextItem();
  Items.Last()->Separator = true;
}

TBOOL CWBContextMenu::MessageProc(CWBMessage &Message) {
  switch (Message.GetMessage()) {
    case WBM_REPOSITION:
      if (Message.GetTarget() == GetGuid()) {
        // push position inside of parent
        CRect p = Message.Rectangle;
        CRect r = GetParent()->GetClientRect();
        if (p.x1 < 0) p += CPoint(-p.x1, 0);
        if (p.y1 < 0) p += CPoint(0, -p.y1);
        if (p.x2 > r.x2) p -= CPoint(p.x2 - r.x2, 0);
        if (p.y2 > r.y2) p -= CPoint(0, p.y2 - r.y2);
        Message.Rectangle = p;
      }
      break;
    case WBM_MOUSEMOVE:
      if (MouseOver()) {
        for (int32_t x = 0; x < Items.NumItems(); x++)
          if (!Items[x]->Separator) {
            CRect EntryPos = GetItemRect(x);
            if (EntryPos.Contains(ScreenToClient(App->GetMousePos()))) {
              SpawnSubMenu(x);
              return true;
            }
          }
      }
      if (SubMenu) SubMenu->MessageProc(Message);
      return true;

    case WBM_RIGHTBUTTONDOWN:
    case WBM_LEFTBUTTONDOWN:
    case WBM_MIDDLEBUTTONDOWN:

      if (!MouseInContextHierarchy() &&
          !MarkedForDeletion())  // need to close the context menu and resend
                                 // the click
      {
        MarkForDeletion();
        MarkParentForDeletion();
        App->ReleaseCapture();
        App->SendMessage(Message);  // resend message
        return true;
      }

      Pushed = true;
      App->SetCapture(GetContextRoot());
      if (!MouseOver())
        if (SubMenu) SubMenu->MessageProc(Message);
      return true;

    case WBM_RIGHTBUTTONUP:
    case WBM_LEFTBUTTONUP:

      if (!Pushed) return true;  // original click doesn't count

      // check if any of the items are clicked on
      for (int32_t x = 0; x < Items.NumItems(); x++)
        if (!Items[x]->Separator) {
          CRect EntryPos = GetItemRect(x);
          if (MouseOver() &&
              EntryPos.Contains(ScreenToClient(App->GetMousePos()))) {
            App->SendMessage(CWBMessage(App, WBM_CONTEXTMESSAGE, Target,
                                        Items[x]->ReturnID));
            if (Items[x]->closesContext) {
              MarkForDeletion();
              MarkParentForDeletion();
              App->ReleaseCapture();
            } else
              App->SendMessage(CWBMessage(App, WBM_REBUILDCONTEXTITEM, Target,
                                          Items[x]->ReturnID, GetGuid()));

            return true;
          }
        }

      if (MouseInContextHierarchy())
        if (SubMenu) SubMenu->MessageProc(Message);

      // check for off item mouse releases
      if (!MouseInContextHierarchy() && Pushed) {
        MarkForDeletion();
        MarkParentForDeletion();
        App->ReleaseCapture();
      }

      return true;
  }

  return CWBItem::MessageProc(Message);
}

void CWBContextMenu::SpawnSubMenu(int32_t itemidx) {
  if (itemidx < 0 || itemidx >= Items.NumItems()) return;
  SAFEDELETE(SubMenu);
  if (Items[itemidx]->Children.NumItems() <= 0) return;

  CRect p = GetItemRect(itemidx) + GetPosition().TopLeft();
  CRect w = GetPosition();

  CRect newpos = CRect(w.x2 - 1, p.y1, w.x2 + 10, p.y1 + 10);

  SubMenu = new CWBContextMenu(GetParent(), newpos, Target);
  App->ApplyStyle(SubMenu);
  SubMenu->ParentMenu = this;
  App->SetCapture(
      GetContextRoot());  // message control must stay with the root item

  for (int32_t x = 0; x < Items[itemidx]->Children.NumItems(); x++) {
    CWBContextItem *olditm = Items[itemidx]->Children[x];
    CWBContextItem *newitm = SubMenu->AddItem(olditm->Text, olditm->ReturnID);
    newitm->CopyOf = olditm;
    newitm->Separator = olditm->Separator;
    newitm->Highlighted = olditm->Highlighted;
    newitm->closesContext = olditm->closesContext;
    newitm->CopyChildrenFrom(olditm);
  }

  SubMenu->ResizeToContentSize();
}

CRect CWBContextMenu::GetItemRect(int32_t idx) {
  CPoint Offset = CPoint(0, 0);

  int arbitraryValue = 0;
  CRect padding = SeparatorElements.PositionDescriptor.GetPadding(
      CSize(0, arbitraryValue), CRect(0, 0, 0, 0));
  int height =
      SeparatorElements.PositionDescriptor.GetHeight(CSize(0, 0), CSize(0, 0));
  int separatorHeight = padding.y1 + height + arbitraryValue - padding.y2;

  CRect Client = GetClientRect();
  padding = CSSProperties.PositionDescriptor.GetPadding(Client.Size(),
                                                        CRect(0, 0, 0, 0));

  Offset.y = padding.y1;

  CWBFont *Font = GetFont(WB_STATE_HOVER);
  for (int32_t x = 0; x < Items.NumItems(); x++) {
    if (Items[x]->Separator) {
      Offset.y += separatorHeight;
    } else if (Font) {
      CRect EntryPos = CRect(GetWindowRect().x1, Offset.y, GetWindowRect().x2,
                             Offset.y + Font->GetLineHeight());
      if (x == idx) return EntryPos;
      Offset.y += Font->GetLineHeight();
    }
  }

  return CRect(0, 0, 0, 0);
}

void CWBContextMenu::MarkParentForDeletion() {
  if (ParentMenu) {
    ParentMenu->MarkForDeletion();
    ParentMenu->MarkParentForDeletion();
  }
}

TBOOL CWBContextMenu::MouseInContextHierarchy() {
  if (!SubMenu) return MouseOver();
  return MouseOver() || SubMenu->MouseInContextHierarchy();
}

CWBContextMenu *CWBContextMenu::GetContextRoot() {
  if (!ParentMenu) return this;
  return ParentMenu->GetContextRoot();
}

CWBContextItem::CWBContextItem() {
  Text = _T( "" );
  ReturnID = 0;
  Separator = false;
}

CWBContextItem::~CWBContextItem() { Children.FreeArray(); }

void CWBContextItem::CopyChildrenFrom(CWBContextItem *itm) {
  for (int32_t x = 0; x < itm->Children.NumItems(); x++) {
    CWBContextItem *olditm = itm->Children[x];
    CWBContextItem *newitm = AddItem(olditm->Text, olditm->ReturnID);
    newitm->Separator = olditm->Separator;
    newitm->Highlighted = olditm->Highlighted;
    newitm->closesContext = olditm->closesContext;
    newitm->CopyChildrenFrom(olditm);
    newitm->CopyOf = olditm;
  }
}

CWBContextItem *CWBContextItem::AddItem(std::string_view Text, int32_t ID,
                                        TBOOL Highlighted,
                                        TBOOL closesContext) {
  CWBContextItem *Item = new CWBContextItem();
  Item->Text = Text;
  Item->ReturnID = ID;
  Item->Highlighted = Highlighted;
  Item->closesContext = closesContext;
  Children += Item;
  return Item;
}

void CWBContextItem::AddSeparator() {
  CWBContextItem *Item = new CWBContextItem();
  Children += Item;
  Children.Last()->Separator = true;
}

void CWBContextItem::SetText(std::string_view text) {
  Text = text;
  if (CopyOf) CopyOf->SetText(text);
}

void CWBContextItem::SetHighlight(TBOOL highlighted) {
  Highlighted = highlighted;
  if (CopyOf) CopyOf->SetHighlight(highlighted);
}

TBOOL CWBContextMenu::ApplyStyle(std::string_view prop, std::string_view value,
                                 const std::vector<std::string> &pseudo) {
  TBOOL ElementTarget = false;

  for (size_t x = 1; x < pseudo.size(); x++) {
    if (pseudo[x] == _T( "separator" )) {
      ElementTarget = true;
      break;
    }
  }

  // apply font styling to list item anyway
  if (!ElementTarget) InterpretFontString(CSSProperties, prop, value, pseudo);
  if (!ElementTarget) return CWBItem::ApplyStyle(prop, value, pseudo);

  TBOOL Handled = false;

  for (size_t x = 1; x < pseudo.size(); x++) {
    if (pseudo[x] == _T( "separator" )) {
      Handled |= SeparatorElements.ApplyStyle(this, prop, value, pseudo);
      continue;
    }
  }

  return Handled;
}

CWBContextItem *CWBContextMenu::GetItem(int32_t ID) {
  for (int32_t x = 0; x < Items.NumItems(); x++)
    if (Items[x]->ReturnID == ID) return Items[x];
  return nullptr;
}
