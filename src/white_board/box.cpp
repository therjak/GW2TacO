#include "src/white_board/box.h"

#include "src/base/logger.h"

void CWBBox::AddChild(const std::shared_ptr<CWBItem>& Item) {
  CWBItem::AddChild(Item);
  RearrangeChildren();
}

CWBBox::CWBBox(CWBItem* Parent, const CRect& Pos) : CWBItem() {
  Initialize(Parent, Pos);
}

CWBBox::~CWBBox() = default;

bool CWBBox::Initialize(CWBItem* Parent, const CRect& Position) {
  Arrangement = WBBOXARRANGEMENT::WB_ARRANGE_NONE;
  Spacing = 0;
  AlignmentX = WB_ALIGN_LEFT;
  AlignmentY = WB_ALIGN_TOP;
  SizingX = WBBOXSIZING::WB_SIZING_KEEP;
  SizingY = WBBOXSIZING::WB_SIZING_KEEP;

  if (!CWBItem::Initialize(Parent, Position)) return false;
  return true;
}

bool CWBBox::MessageProc(const CWBMessage& Message) {
  switch (Message.GetMessage()) {
    default:
      break;

    case WBM_CLIENTAREACHANGED:
      if (Message.GetTarget() == GetGuid()) {
        RearrangeChildren();
      }
      break;

    case WBM_MOUSEWHEEL:
      if (App->GetCtrlState() && IsVScrollbarEnabled()) {
        const CPoint content = GetContentSize();
        const CRect client = GetClientRect();
        if (content.y < client.Height()) break;

        CRect BRect = CRect(0, 0, 0, 0);

        for (uint32_t x = 0; x < NumChildren(); x++)
          if (!GetChild(x)->IsHidden())
            BRect = BRect & GetChild(x)->GetPosition();

        SetVScrollbarPos(
            std::max(BRect.y1, std::min(BRect.y2 - client.Height(),
                                        GetVScrollbarPos() -
                                            (int)(Message.Data *
                                                  client.Height() / 5.0f))),
            false);
        return true;
      }
      break;

    case WBM_HSCROLL: {
      const bool Result = CWBItem::MessageProc(Message);
      if (Message.GetTarget() == GetGuid()) {
        ChangeContentOffsetX(-Message.Data);
        return true;
      }
      return Result;
    } break;
    case WBM_VSCROLL: {
      const bool Result = CWBItem::MessageProc(Message);
      if (Message.GetTarget() == GetGuid()) {
        ChangeContentOffsetY(-Message.Data);
        return true;
      }
      return Result;
    } break;

    case WBM_REPOSITION: {
      if (Message.GetTarget() == GetGuid()) {
        CWBItem::MessageProc(Message);
        RearrangeChildren();
        return true;
      }

      CWBItem* i = App->FindItemByGuid(Message.GetTarget());

      if (i && i->GetParent() == this) {
        if (Arrangement == WBBOXARRANGEMENT::WB_ARRANGE_NONE) {
          break;  // do nothing here
        }

        CRect r = Message.Rectangle;
        const CRect o = i->GetPosition();

        if (SizingX == WBBOXSIZING::WB_SIZING_FILL) {
          r.x1 = o.x1;
          r.x2 = o.x2;
        }

        if (SizingY == WBBOXSIZING::WB_SIZING_FILL) {
          r.y1 = o.y1;
          r.y2 = o.y2;
        }
        auto m = Message;
        m.Rectangle = r;
        m.Resized = true;
        m.Moved = r.TopLeft() == o.TopLeft();

        i->MessageProc(m);  // do the resize as the item sees fit

        RearrangeChildren();
        return true;
      }
    } break;
  }

  return CWBItem::MessageProc(Message);
}

void CWBBox::RearrangeHorizontal() {
  const CRect ClientRect = GetClientRect();
  int32_t pos = 0;
  float Excess = 0;

  int32_t NumDynamicChildren = 0;
  int32_t NonDynamicWidth = 0;
  int32_t LastDynamic = 0;

  for (uint32_t x = 0; x < NumChildren(); x++) {
    if (!GetChild(x)->IsWidthSet()) {
      NumDynamicChildren++;
      LastDynamic = x;
    } else
      NonDynamicWidth += GetChild(x)->GetCalculatedWidth(ClientRect.Size());
  }

  int32_t width = (NumChildren() - 1) * Spacing;
  const int32_t DynamicWidth =
      GetClientRect().Width() - NonDynamicWidth - width;
  const float itemsizes = DynamicWidth / static_cast<float>(NumDynamicChildren);

  for (uint32_t x = 0; x < NumChildren(); x++)
    width += GetChild(x)->GetPosition().Width();

  if (AlignmentX == WB_ALIGN_RIGHT) pos = GetClientRect().Width() - width;
  if (AlignmentX == WB_ALIGN_CENTER)
    pos = (GetClientRect().Width() - width) / 2;

  for (uint32_t x = 0; x < NumChildren(); x++) {
    CRect ChildPosition = GetChild(x)->GetPosition();

    if (SizingX == WBBOXSIZING::WB_SIZING_FILL) {
      if (!GetChild(x)->IsWidthSet()) {
        ChildPosition.x1 = pos;
        Excess += itemsizes;
        ChildPosition.x2 = ChildPosition.x1 + static_cast<int32_t>(Excess);
        Excess -= static_cast<int32_t>(Excess);
        if (x == LastDynamic && Excess >= 0.5)
          ChildPosition.x2++;  // fucking float inaccuracies...
      } else {
        ChildPosition.x1 = pos;
        ChildPosition.x2 = ChildPosition.x1 +
                           GetChild(x)->GetCalculatedWidth(ClientRect.Size());
      }
    } else {
      const int32_t posw = ChildPosition.Width();
      ChildPosition.x1 = pos;
      ChildPosition.x2 = pos + posw;
    }

    if (SizingY == WBBOXSIZING::WB_SIZING_FILL) {
      ChildPosition.y1 = 0;
      ChildPosition.y2 = GetClientRect().Height();
    }

    int32_t off = 0;
    if (AlignmentY == WB_ALIGN_BOTTOM)
      off = ClientRect.Height() - ChildPosition.Height();
    if (AlignmentY == WB_ALIGN_CENTER)
      off = (ClientRect.Height() - ChildPosition.Height()) / 2;

    const CRect np = CRect(pos, off, pos + ChildPosition.Width(),
                           off + ChildPosition.Height());

    if (GetChild(x)->GetPosition() != np) {
      const CWBMessage m = GetChild(x)->BuildPositionMessage(np);
      GetChild(x)->MessageProc(m);
    }

    pos = ChildPosition.x2 + Spacing;
  }
}

void CWBBox::RearrangeVertical() {
  const CRect ClientRect = GetClientRect();
  int32_t pos = 0;
  float Excess = 0;

  int32_t NumDynamicChildren = 0;
  int32_t NonDynamicHeight = 0;
  int32_t LastDynamic = 0;

  for (uint32_t x = 0; x < NumChildren(); x++) {
    if (!GetChild(x)->IsHeightSet()) {
      NumDynamicChildren++;
      LastDynamic = x;
    } else
      NonDynamicHeight += GetChild(x)->GetCalculatedHeight(ClientRect.Size());
  }

  int32_t height = (NumChildren() - 1) * Spacing;
  const int32_t DynamicHeight =
      GetClientRect().Height() - NonDynamicHeight - height;
  const float itemsizes =
      DynamicHeight / static_cast<float>(NumDynamicChildren);

  for (uint32_t x = 0; x < NumChildren(); x++)
    height += GetChild(x)->GetPosition().Height();

  if (AlignmentY == WB_ALIGN_BOTTOM) pos = GetClientRect().Height() - height;
  if (AlignmentY == WB_ALIGN_CENTER)
    pos = (GetClientRect().Height() - height) / 2;

  for (uint32_t x = 0; x < NumChildren(); x++) {
    CRect ChildPosition = GetChild(x)->GetPosition();

    if (SizingX == WBBOXSIZING::WB_SIZING_FILL) {
      ChildPosition.x1 = 0;
      ChildPosition.x2 = GetClientRect().Width();
    }

    if (SizingY == WBBOXSIZING::WB_SIZING_FILL) {
      if (!GetChild(x)->IsHeightSet()) {
        ChildPosition.y1 = pos;
        Excess += itemsizes;
        ChildPosition.y2 = ChildPosition.y1 + static_cast<int32_t>(Excess);
        Excess -= static_cast<int32_t>(Excess);
        if (x == LastDynamic && Excess >= 0.5)
          ChildPosition.y2++;  // fucking float inaccuracies...
      } else {
        ChildPosition.y1 = pos;
        ChildPosition.y2 = ChildPosition.y1 +
                           GetChild(x)->GetCalculatedHeight(ClientRect.Size());
      }
    } else {
      const int32_t posh = ChildPosition.Height();
      ChildPosition.y1 = pos;
      ChildPosition.y2 = pos + posh;
    }

    int32_t off = 0;
    if (AlignmentX == WB_ALIGN_BOTTOM)
      off = ClientRect.Width() - ChildPosition.Width();
    if (AlignmentX == WB_ALIGN_CENTER)
      off = (ClientRect.Width() - ChildPosition.Width()) / 2;

    const CRect np = CRect(off, pos, off + ChildPosition.Width(),
                           pos + ChildPosition.Height());

    if (GetChild(x)->GetPosition() != np) {
      const CWBMessage m = GetChild(x)->BuildPositionMessage(np);
      GetChild(x)->MessageProc(m);
    }

    pos = ChildPosition.y2 + Spacing;
  }
}

void CWBBox::RearrangeChildren() {
  switch (Arrangement) {
    case WBBOXARRANGEMENT::WB_ARRANGE_NONE:
      break;
    case WBBOXARRANGEMENT::WB_ARRANGE_HORIZONTAL:
      RearrangeHorizontal();
      break;
    case WBBOXARRANGEMENT::WB_ARRANGE_VERTICAL:
      RearrangeVertical();
      break;
  }

  UpdateScrollbarData();
}

void CWBBox::OnDraw(CWBDrawAPI* API) {
  DrawBackground(API);
  DrawBorder(API);
}

void CWBBox::SetArrangement(WBBOXARRANGEMENT a) {
  Arrangement = a;
  RearrangeChildren();
}

WBBOXARRANGEMENT CWBBox::GetArrangement() { return Arrangement; }

void CWBBox::SetSpacing(int32_t s) {
  Spacing = s;
  RearrangeChildren();
}

bool CWBBox::ApplyStyle(std::string_view prop, std::string_view value,
                        const std::vector<std::string>& pseudo) {
  if (CWBItem::ApplyStyle(prop, value, pseudo)) return true;

  if (prop == _T( "child-layout" )) {
    if (value == _T( "none" )) {
      SetArrangement(WBBOXARRANGEMENT::WB_ARRANGE_NONE);
      return true;
    }
    if (value == _T( "horizontal" )) {
      SetArrangement(WBBOXARRANGEMENT::WB_ARRANGE_HORIZONTAL);
      return true;
    }
    if (value == _T( "vertical" )) {
      SetArrangement(WBBOXARRANGEMENT::WB_ARRANGE_VERTICAL);
      return true;
    }
    LOG_WARN(
        "[gui] %s has invalid parameter: '%s' (none/horizontal/vertical "
        "required)",
        std::string(prop).c_str(), std::string(value).c_str());
    return false;
  }

  if (prop == _T( "child-spacing" )) {
    int32_t dw = std::stoi(std::string(value));
    SetSpacing(dw);
    return true;
  }

  if (prop == _T( "child-align-x" )) {
    if (value == _T( "left" )) {
      SetAlignment(WBBOXAXIS::WB_HORIZONTAL, WB_ALIGN_LEFT);
      return true;
    }
    if (value == _T( "right" )) {
      SetAlignment(WBBOXAXIS::WB_HORIZONTAL, WB_ALIGN_RIGHT);
      return true;
    }
    if (value == _T( "center" )) {
      SetAlignment(WBBOXAXIS::WB_HORIZONTAL, WB_ALIGN_CENTER);
      return true;
    }

    LOG_WARN(
        "[gui] %s has invalid parameter: '%s' (left/center/right required)",
        std::string(prop).c_str(), std::string(value).c_str());

    return true;
  }

  if (prop == _T( "child-align-y" )) {
    if (value == _T( "top" )) {
      SetAlignment(WBBOXAXIS::WB_VERTICAL, WB_ALIGN_TOP);
      return true;
    }
    if (value == _T( "bottom" )) {
      SetAlignment(WBBOXAXIS::WB_VERTICAL, WB_ALIGN_BOTTOM);
      return true;
    }
    if (value == _T( "center" )) {
      SetAlignment(WBBOXAXIS::WB_VERTICAL, WB_ALIGN_CENTER);
      return true;
    }

    LOG_WARN(
        "[gui] %s has invalid parameter: '%s' (top/center/bottom required)",
        std::string(prop).c_str(), std::string(value).c_str());

    return true;
  }

  if (prop == _T( "child-fill-x" )) {
    if (value == _T( "false" )) {
      SetSizing(WBBOXAXIS::WB_HORIZONTAL, WBBOXSIZING::WB_SIZING_KEEP);
      return true;
    }
    if (value == _T( "true" )) {
      SetSizing(WBBOXAXIS::WB_HORIZONTAL, WBBOXSIZING::WB_SIZING_FILL);
      return true;
    }

    LOG_WARN("[gui] %s has invalid parameter: '%s' (true/false required)",
             std::string(prop).c_str(), std::string(value).c_str());

    return true;
  }

  if (prop == _T( "child-fill-y" )) {
    if (value == _T( "false" )) {
      SetSizing(WBBOXAXIS::WB_VERTICAL, WBBOXSIZING::WB_SIZING_KEEP);
      return true;
    }
    if (value == _T( "true" )) {
      SetSizing(WBBOXAXIS::WB_VERTICAL, WBBOXSIZING::WB_SIZING_FILL);
      return true;
    }

    LOG_WARN("[gui] %s has invalid parameter: '%s' (true/false required)",
             std::string(prop).c_str(), std::string(value).c_str());

    return true;
  }

  return false;
}

bool CWBBox::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                WBMESSAGE MessageType) {
  return ClickThrough;
}

void CWBBox::SetAlignment(WBBOXAXIS axis, WBALIGNMENT align) {
  if (axis == WBBOXAXIS::WB_HORIZONTAL) AlignmentX = align;
  if (axis == WBBOXAXIS::WB_VERTICAL) AlignmentY = align;
  RearrangeChildren();
}

void CWBBox::SetSizing(WBBOXAXIS axis, WBBOXSIZING siz) {
  if (axis == WBBOXAXIS::WB_HORIZONTAL) SizingX = siz;
  if (axis == WBBOXAXIS::WB_VERTICAL) SizingY = siz;
  RearrangeChildren();
}

CWBItem* CWBBox::Factory(CWBItem* Root, const CXMLNode& node, CRect& Pos) {
  auto box = CWBBox::Create(Root, Pos);

  if (node.HasAttribute(_T( "clickthrough" ))) {
    int32_t b = 0;
    node.GetAttributeAsInteger(_T( "clickthrough" ), &b);
    box->ClickThrough = b != 0;
  }

  return box.get();
}

void CWBBox::UpdateScrollbarData() {
  if (!ScrollbarsEnabled()) return;

  CRect BRect = CRect(0, 0, 0, 0);

  for (uint32_t x = 0; x < NumChildren(); x++)
    if (!GetChild(x)->IsHidden()) BRect = BRect & GetChild(x)->GetPosition();

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
