#include "src/white_board/gui_item.h"

#include "src/base/logger.h"
#include "src/base/string_format.h"
#include "src/white_board/application.h"

static WBGUID WB_GUID_COUNTER = 1337;

//////////////////////////////////////////////////////////////////////////
// Display Descriptor

CWBDisplayState::CWBDisplayState() = default;

CWBDisplayState::~CWBDisplayState() = default;

CColor CWBDisplayState::GetColor(WBITEMVISUALCOMPONENT v) {
  return CColor(Visuals[v]);
}

bool CWBDisplayState::IsSet(WBITEMVISUALCOMPONENT v) { return VisualSet[v]; }

void CWBDisplayState::SetValue(WBITEMVISUALCOMPONENT v, int32_t value) {
  Visuals[v] = value;
  VisualSet[v] = true;
}

WBSKINELEMENTID CWBDisplayState::GetSkin(WBITEMVISUALCOMPONENT v) {
  return Visuals[v];
}

int32_t CWBDisplayState::GetValue(WBITEMVISUALCOMPONENT v) {
  return Visuals[v];
}

CWBDisplayProperties::CWBDisplayProperties() = default;

CWBDisplayProperties::~CWBDisplayProperties() = default;

WBSKINELEMENTID CWBDisplayProperties::GetSkin(WBITEMSTATE s,
                                              WBITEMVISUALCOMPONENT v) {
  // if (States.HasKey(s))
  {
    if (States[s].IsSet(v)) return States[s].GetSkin(v);
  }
  // if (States.HasKey(WB_STATE_NORMAL))
  {
    if (States[WB_STATE_NORMAL].IsSet(v))
      return States[WB_STATE_NORMAL].GetSkin(v);
  }

  return 0xffffffff;
}

CColor CWBDisplayProperties::GetColor(WBITEMSTATE s, WBITEMVISUALCOMPONENT v) {
  // if (States.HasKey(s))
  {
    if (States[s].IsSet(v)) return States[s].GetColor(v);
  }
  // if (States.HasKey(WB_STATE_NORMAL))
  {
    if (States[WB_STATE_NORMAL].IsSet(v))
      return States[WB_STATE_NORMAL].GetColor(v);
  }

  switch (v) {
    case WB_ITEM_BACKGROUNDCOLOR:
      return CColor{0};
    case WB_ITEM_FOREGROUNDCOLOR:
      return CColor{0};
    case WB_ITEM_BORDERCOLOR:
      return CColor{0xff434346};
    case WB_ITEM_FONTCOLOR:
      return CColor{0xffffffff};
  }

  return CColor{0xffffffff};
}

void CWBDisplayProperties::SetValue(WBITEMSTATE s, WBITEMVISUALCOMPONENT v,
                                    int32_t value) {
  States[s].SetValue(v, value);
}

int32_t CWBDisplayProperties::GetValue(WBITEMSTATE s, WBITEMVISUALCOMPONENT v) {
  if (States[s].IsSet(v)) return States[s].GetValue(v);
  return -1;
}

CWBCSSPropertyBatch::CWBCSSPropertyBatch() {
  BorderSizes = CRect(0, 0, 0, 0);
  TextAlignX = WBTEXTALIGNMENTX::WBTA_CENTERX;
  TextAlignY = WBTEXTALIGNMENTY::WBTA_CENTERY;
}

CWBFont* CWBCSSPropertyBatch::GetFont(CWBApplication* App, WBITEMSTATE State) {
  if (Fonts.find(State) != Fonts.end()) {
    return App->GetFont(Fonts[State]);
  }
  if (Fonts.find(WB_STATE_NORMAL) != Fonts.end()) {
    return App->GetFont(Fonts[WB_STATE_NORMAL]);
  }
  return App->GetDefaultFont();
}

bool CWBCSSPropertyBatch::ApplyStyle(CWBItem* Owner, std::string_view prop,
                                     std::string_view value,
                                     const std::vector<std::string>& pseudo) {
  if (Owner->InterpretPositionString(*this, prop, value, pseudo)) return true;
  if (Owner->InterpretDisplayString(*this, prop, value, pseudo)) return true;
  return false;
}

//////////////////////////////////////////////////////////////////////////
// CWBItem

void CWBItem::UpdateScreenRect() {
  const CRect sr = ScreenRect;

  if (Parent) {
    ScreenRect =
        Position + Parent->ClientToScreen(CPoint(0, 0)) + Parent->ContentOffset;
  } else {
    ScreenRect = Position;
  }

  if (sr != ScreenRect) {
    for (auto& c : Children) {
      c->UpdateScreenRect();
    }
  }
}

void CWBItem::HandleHScrollbarClick(WBSCROLLDRAGMODE m) {
  switch (m) {
    case WB_SCROLLDRAG_BUTTON1:
      SetHScrollbarPos(GetHScrollbarPos() - GetScrollbarStep(), true);
      break;
    case WB_SCROLLDRAG_BUTTON2:
      SetHScrollbarPos(GetHScrollbarPos() + GetScrollbarStep(), true);
      break;
    case WB_SCROLLDRAG_UP:
      SetHScrollbarPos(GetHScrollbarPos() - HScrollbar.ViewSize, true);
      break;
    case WB_SCROLLDRAG_DOWN:
      SetHScrollbarPos(GetHScrollbarPos() + HScrollbar.ViewSize, true);
      break;
  }
}

void CWBItem::HandleVScrollbarClick(WBSCROLLDRAGMODE m) {
  switch (m) {
    case WB_SCROLLDRAG_BUTTON1:
      SetVScrollbarPos(GetVScrollbarPos() - GetScrollbarStep(), true);
      break;
    case WB_SCROLLDRAG_BUTTON2:
      SetVScrollbarPos(GetVScrollbarPos() + GetScrollbarStep(), true);
      break;
    case WB_SCROLLDRAG_UP:
      SetVScrollbarPos(GetVScrollbarPos() - VScrollbar.ViewSize, true);
      break;
    case WB_SCROLLDRAG_DOWN:
      SetVScrollbarPos(GetVScrollbarPos() + VScrollbar.ViewSize, true);
      break;
  }
}

bool CWBItem::MessageProc(const CWBMessage& Message) {
  switch (Message.GetMessage()) {
    case WBM_NONE:
      LOG_ERR("[gui] Message Type 0 Encountered. Message Target is %d",
              Message.GetTarget());
      return true;

    case WBM_LEFTBUTTONDOWN:
      // handle scrollbars
      if (App->GetMouseItem() == this) {
        CRect b1, up, th, dn, b2;
        // mouse pos
        CPoint mp = Message.GetPosition();
        if (GetHScrollbarRectangles(b1, up, th, dn, b2)) {
          if (ClientToScreen(b1).Contains(mp))
            HScrollbar.Dragmode = WB_SCROLLDRAG_BUTTON1;
          if (ClientToScreen(up).Contains(mp))
            HScrollbar.Dragmode = WB_SCROLLDRAG_UP;
          if (ClientToScreen(th).Contains(mp))
            HScrollbar.Dragmode = WB_SCROLLDRAG_THUMB;
          if (ClientToScreen(dn).Contains(mp))
            HScrollbar.Dragmode = WB_SCROLLDRAG_DOWN;
          if (ClientToScreen(b2).Contains(mp))
            HScrollbar.Dragmode = WB_SCROLLDRAG_BUTTON2;
          if (HScrollbar.Dragmode != WB_SCROLLDRAG_NONE) {
            HScrollbar.DragStartPosition = HScrollbar.ScrollPos;
            HandleHScrollbarClick(HScrollbar.Dragmode);
            App->SetCapture(this);
            return true;
          }
        }
        if (GetVScrollbarRectangles(b1, up, th, dn, b2)) {
          if (ClientToScreen(b1).Contains(mp))
            VScrollbar.Dragmode = WB_SCROLLDRAG_BUTTON1;
          if (ClientToScreen(up).Contains(mp))
            VScrollbar.Dragmode = WB_SCROLLDRAG_UP;
          if (ClientToScreen(th).Contains(mp))
            VScrollbar.Dragmode = WB_SCROLLDRAG_THUMB;
          if (ClientToScreen(dn).Contains(mp))
            VScrollbar.Dragmode = WB_SCROLLDRAG_DOWN;
          if (ClientToScreen(b2).Contains(mp))
            VScrollbar.Dragmode = WB_SCROLLDRAG_BUTTON2;
          if (VScrollbar.Dragmode != WB_SCROLLDRAG_NONE) {
            VScrollbar.DragStartPosition = VScrollbar.ScrollPos;
            HandleVScrollbarClick(VScrollbar.Dragmode);
            App->SetCapture(this);
            return true;
          }
        }
      }
      return false;
      break;

    case WBM_LEFTBUTTONREPEAT:
      if (App->GetMouseItem() == this) {
        if (HScrollbar.Dragmode != WB_SCROLLDRAG_NONE &&
            HScrollbar.Dragmode != WB_SCROLLDRAG_THUMB) {
          CRect b1, up, th, dn, b2;
          // mouse pos
          CPoint mp = Message.GetPosition();
          if (GetHScrollbarRectangles(b1, up, th, dn, b2)) {
            if (ClientToScreen(b1).Contains(mp) &&
                HScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON1)
              HandleHScrollbarClick(HScrollbar.Dragmode);
            if (ClientToScreen(up).Contains(mp) &&
                HScrollbar.Dragmode == WB_SCROLLDRAG_UP)
              HandleHScrollbarClick(HScrollbar.Dragmode);
            if (ClientToScreen(dn).Contains(mp) &&
                HScrollbar.Dragmode == WB_SCROLLDRAG_DOWN)
              HandleHScrollbarClick(HScrollbar.Dragmode);
            if (ClientToScreen(b2).Contains(mp) &&
                HScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON2)
              HandleHScrollbarClick(HScrollbar.Dragmode);
          }

          return true;
        }

        if (VScrollbar.Dragmode != WB_SCROLLDRAG_NONE &&
            VScrollbar.Dragmode != WB_SCROLLDRAG_THUMB) {
          CRect b1, up, th, dn, b2;
          // mouse pos
          CPoint mp = Message.GetPosition();
          if (GetVScrollbarRectangles(b1, up, th, dn, b2)) {
            if (ClientToScreen(b1).Contains(mp) &&
                VScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON1)
              HandleVScrollbarClick(VScrollbar.Dragmode);
            if (ClientToScreen(up).Contains(mp) &&
                VScrollbar.Dragmode == WB_SCROLLDRAG_UP)
              HandleVScrollbarClick(VScrollbar.Dragmode);
            if (ClientToScreen(dn).Contains(mp) &&
                VScrollbar.Dragmode == WB_SCROLLDRAG_DOWN)
              HandleVScrollbarClick(VScrollbar.Dragmode);
            if (ClientToScreen(b2).Contains(mp) &&
                VScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON2)
              HandleVScrollbarClick(VScrollbar.Dragmode);
          }

          return true;
        }
      }
      break;

    case WBM_MOUSEMOVE:
      // handle scrollbars only
      if (App->GetMouseCaptureItem() == this)
        if (ScrollbarDragged()) {
          // mouse delta
          const CPoint md = Message.GetPosition() - App->GetLeftDownPos();

          if (HScrollbar.Dragmode == WB_SCROLLDRAG_THUMB) {
            const int32_t newpos = CalculateScrollbarMovement(
                HScrollbar, GetClientRect().Width(), md.x);
            HScrollbar.ScrollPos = newpos;
            App->SendMessage(CWBMessage(App, WBM_HSCROLL, GetGuid(), newpos));
          }
          if (VScrollbar.Dragmode == WB_SCROLLDRAG_THUMB) {
            const int32_t newpos = CalculateScrollbarMovement(
                VScrollbar, GetClientRect().Height(), md.y);
            VScrollbar.ScrollPos = newpos;
            App->SendMessage(CWBMessage(App, WBM_VSCROLL, GetGuid(), newpos));
          }

          return true;
        }
      return false;
      break;

    case WBM_LEFTBUTTONUP:
      // handle scrollbars only
      if (App->GetMouseCaptureItem() == this)
        if (ScrollbarDragged()) {
          HScrollbar.Dragmode = WB_SCROLLDRAG_NONE;
          VScrollbar.Dragmode = WB_SCROLLDRAG_NONE;
          App->ReleaseCapture();
          return true;
        }
      return false;
      break;

    case WBM_REPOSITION:
      if (Message.GetTarget() == Guid) {
        ApplyPosition(Message.Rectangle);
        if (Message.Moved) OnMove(Message.Rectangle.TopLeft());
        if (Message.Resized) {
          OnResize(Message.Rectangle.Size());
        }
        for (auto& c : Children) {
          c->CalculateWindowPosition(GetClientRect().Size());
        }

        return true;
      }
      return false;
      break;

      // case WBM_FOCUSLOST:
      //	if (Message.GetTarget() == Guid)
      //	{
      //		if (Parent) Parent->ChildInFocus = NULL;
      //		return true;
      //	}
      //	return false;

    case WBM_HIDE:
      if (Message.GetTarget() == Guid) {
        Hidden = true;
        return true;
      }
      return false;
      break;

    case WBM_UNHIDE:
      if (Message.GetTarget() == Guid) {
        Hidden = false;
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

void CWBItem::DrawBackgroundItem(CWBDrawAPI* API,
                                 CWBDisplayProperties& Descriptor,
                                 const CRect& Pos, WBITEMSTATE i,
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
      CWBSkinElement* e = App->GetSkin()->GetElement(id);
      if (e) {
        const WBALIGNMENT AlignX = static_cast<WBALIGNMENT>(
            Descriptor.GetValue(i, WB_ITEM_BACKGROUNDALIGNMENT_X));
        const WBALIGNMENT AlignY = static_cast<WBALIGNMENT>(
            Descriptor.GetValue(i, WB_ITEM_BACKGROUNDALIGNMENT_Y));
        const CSize elementsize = e->GetElementSize(API);
        CPoint offset = CPoint(0, 0);
        CSize size = Pos.Size();

        if (e->GetBehavior(0) ==
            WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_PIXELCORRECT) {
          if (AlignX == WB_ALIGN_RIGHT) offset.x = Pos.Width() - elementsize.x;
          if (AlignX == WB_ALIGN_CENTER)
            offset.x = (Pos.Width() - elementsize.x) / 2;
          size.x = elementsize.x;
        }

        if (e->GetBehavior(1) ==
            WBSKINELEMENTBEHAVIOR::WB_SKINBEHAVIOR_PIXELCORRECT) {
          if (AlignY == WB_ALIGN_BOTTOM)
            offset.y = Pos.Height() - elementsize.y;
          if (AlignY == WB_ALIGN_MIDDLE)
            offset.y = (Pos.Height() - elementsize.y) / 2;
          size.y = elementsize.y;
        }

        const CRect DisplayRect =
            CRect(Pos.TopLeft() + offset, Pos.TopLeft() + offset + size);

        App->GetSkin()->RenderElement(API, id, DisplayRect);
      }
    } else {
      // mosaic
      App->GetSkin()->RenderElement(API, id, Pos);
    }
    return;
  }
}

void CWBItem::DrawBackground(CWBDrawAPI* API, WBITEMSTATE State) {
  DrawBackground(API, GetWindowRect(), State, CSSProperties);
}

void CWBItem::DrawBackground(CWBDrawAPI* API) {
  DrawBackground(API, GetState());
}

void CWBItem::DrawBackground(CWBDrawAPI* API, const CRect& rect,
                             WBITEMSTATE State, CWBCSSPropertyBatch& cssProps) {
  DrawBackgroundItem(API, cssProps.DisplayDescriptor, rect, State);
}

void CWBItem::DrawBorder(CWBDrawAPI* API) {
  DrawBorder(API, GetWindowRect(), CSSProperties);
}

void CWBItem::DrawBorder(CWBDrawAPI* API, const CRect& r,
                         CWBCSSPropertyBatch& cssProps) {
  const auto& crop = API->GetCropRect();

  const CColor color =
      cssProps.DisplayDescriptor.GetColor(GetState(), WB_ITEM_BORDERCOLOR);

  API->SetCropRect(ClientToScreen(r));

  if (cssProps.BorderSizes.x1 > 0)
    API->DrawRect(
        CRect(r.TopLeft(), r.BottomLeft() + CPoint(cssProps.BorderSizes.x1, 0)),
        color);
  if (cssProps.BorderSizes.y1 > 0)
    API->DrawRect(
        CRect(r.TopLeft(), r.TopRight() + CPoint(0, cssProps.BorderSizes.y1)),
        color);
  if (cssProps.BorderSizes.x2 > 0)
    API->DrawRect(CRect(r.TopRight() - CPoint(cssProps.BorderSizes.x2, 0),
                        r.BottomRight()),
                  color);
  if (cssProps.BorderSizes.y2 > 0)
    API->DrawRect(CRect(r.BottomLeft() - CPoint(0, cssProps.BorderSizes.y2),
                        r.BottomRight()),
                  color);

  API->SetCropRect(crop);
}

void CWBItem::OnDraw(CWBDrawAPI* API) {
  DrawBackground(API);
  DrawBorder(API);
}

void CWBItem::OnPostDraw(CWBDrawAPI* API) {}

int32_t CWBItem::GetScrollbarStep() { return 1; }

void CWBItem::OnMove(const CPoint& p) {}

void CWBItem::OnResize(const CSize& s) {}

void CWBItem::OnMouseEnter() {
  // LOG(LOG_DEBUG,_T("Mouse Entered Item %d"),GetGuid());
}

void CWBItem::OnMouseLeave() {
  // LOG(LOG_DEBUG,_T("Mouse Left Item %d"),GetGuid());
}

void CWBItem::CalculateClientPosition() {
  const CPoint p = ClientToScreen(CPoint(0, 0));

  ClientRect = CSSProperties.PositionDescriptor.GetPadding(
      GetWindowRect().Size(), CSSProperties.BorderSizes);

  HScrollbar.Visible = false;
  VScrollbar.Visible = false;
  AdjustClientAreaToFitScrollbars();

  if (p != ClientToScreen(CPoint(0, 0)) || ScreenRect.Width() < 0) {
    // this updates all child items for the new position
    UpdateScreenRect();
  }
}

void CWBItem::CalculateWindowPosition(const CSize& s) {
  SetPosition(CSSProperties.PositionDescriptor.GetPosition(s, StoredContentSize,
                                                           GetPosition()));
}

void CWBItem::DrawTree(CWBDrawAPI* API) {
  if (Hidden) return;

  CRect PCrop = API->GetParentCropRect();
  CRect Crop = API->GetCropRect();
  API->SetParentCropRect(Crop);
  const CPoint Offset = API->GetOffset();

  API->SetCropRect(ScreenRect);
  API->SetOffset(ClientToScreen(CPoint(0, 0)));

  ApplyOpacity(API);
  OnDraw(API);

  if (VScrollbar.Visible || HScrollbar.Visible) {
    API->SetCropRect(ScreenRect);
    DrawHScrollbar(API);
    DrawVScrollbar(API);
  }

  // crop children to client rect
  API->SetCropRect(ClientToScreen(GetClientRect()));

  for (auto& c : Children) {
    c->DrawTree(API);
  }

  OnPostDraw(API);

  API->SetParentCropRect(PCrop);
  API->SetCropRect(Crop);
  API->SetOffset(Offset);
}

void CWBItem::ApplyPosition(const CRect& Pos) {
  const CRect r = GetScreenRect();

  Position = Pos;
  CalculateClientPosition();
  UpdateScreenRect();

  const CRect r2 = GetScreenRect();

  if (App && r.Contains(App->GetMousePos()) != r2.Contains(App->GetMousePos()))
    App->UpdateMouseItem();
}

bool CWBItem::Focusable() const { return true; }

CWBItem* CWBItem::GetChildInFocus() { return ChildInFocus; }

CWBItem* CWBItem::SetCapture() { return App->SetCapture(this); }

bool CWBItem::ReleaseCapture() const { return App->ReleaseCapture(); }

bool CWBItem::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                 WBMESSAGE MessageType) {
  if (ForceMouseTransparent) return true;
  if (Hidden) return true;
  return false;
}

bool CWBItem::FindItemInParentTree(const CWBItem* Item) {
  CWBItem* i = this;
  while (i) {
    if (i == Item) return true;
    i = i->Parent;
  }
  return false;
}

CWBItem* CWBItem::GetItemUnderMouse(CPoint& Pos, CRect& CropRect,
                                    WBMESSAGE MessageType) {
  CRect OldCropRect = CropRect;
  CropRect = CropRect | ScreenRect;

  if (Hidden || !CropRect.Contains(Pos)) {
    CropRect = OldCropRect;
    return nullptr;
  }

  CropRect = CropRect | ClientToScreen(GetClientRect());

  for (int32_t x = Children.size(); x > 0; x--) {
    CWBItem* Res =
        Children[x - 1]->GetItemUnderMouse(Pos, CropRect, MessageType);
    if (Res) return Res;
  }

  CropRect = OldCropRect;

  if (IsMouseTransparent(ScreenToClient(Pos), MessageType)) return nullptr;

  return this;
}

void CWBItem::SetChildAsTopmost(int32_t Index) {
  auto it = Children.begin() + Index;
  std::rotate(it, it + 1, Children.end());
}

void CWBItem::SetChildAsBottommost(int32_t Index) {
  auto it = Children.begin() + Index;
  std::rotate(Children.begin(), it, it);
}

void CWBItem::SetTopmost() {
  if (!GetParent()) return;
  const int32_t x = GetParent()->GetChildIndex(this);
  if (x < 0) return;
  Parent->SetChildAsTopmost(x);
}

void CWBItem::SetBottommost() {
  if (!GetParent()) return;
  const int32_t x = GetParent()->GetChildIndex(this);
  if (x < 0) return;
  Parent->SetChildAsBottommost(x);
}

CWBItem::CWBItem() : Guid(WB_GUID_COUNTER++) {}

CWBItem::CWBItem(CWBItem* parent, const CRect& position)
    : Guid(WB_GUID_COUNTER++) {
  Initialize(parent, position);
}

CWBItem::~CWBItem() {
  for (auto& c : Children) {
    c->Parent = nullptr;
  }

  // Make sure Children is empty before we delete them, as they try to
  // unregister themselfes.
  std::vector<std::shared_ptr<CWBItem>> c;
  std::swap(c, Children);

  if (Parent) {
    auto& pc = Parent->Children;
    auto it = std::find_if(
        pc.begin(), pc.end(),
        [this](const std::shared_ptr<CWBItem>& i) { return i.get() == this; });
    if (it != pc.end()) {
      pc.erase(it);
    }
    if (Parent->ChildInFocus == this) {
      Parent->ChildInFocus = nullptr;
    }
  }

  if (App) {
    App->UnRegisterItem(this);
  }

  c.clear();
}

void CWBItem::AddChild(const std::shared_ptr<CWBItem>& Item) {
  Children.emplace_back(Item);
}

void CWBItem::RemoveChild(const std::shared_ptr<CWBItem>& Item) {
  auto it = std::find(Children.begin(), Children.end(), Item);
  if (it != Children.end()) {
    if (ChildInFocus == Item.get()) {
      ChildInFocus = nullptr;
    }
    Item->Parent = nullptr;
    Children.erase(it);
  }
}

int32_t CWBItem::GetChildIndex(CWBItem* Item) {
  auto it = std::find_if(
      Children.begin(), Children.end(),
      [Item](std::shared_ptr<CWBItem>& i) { return i.get() == Item; });
  if (it == Children.end()) {
    return -1;
  }
  return std::distance(Children.begin(), it);
}

bool CWBItem::Initialize(CWBItem* parent, const CRect& position) {
  Hidden = false;
  Disabled = false;
  Data = nullptr;
  ChildInFocus = nullptr;
  Parent = parent;
  Scrollbar_ThumbMinimalSize = 4;
  Scrollbar_Size = 16;
  Scrollbar_ButtonSize = 16;
  StoredContentSize = position.Size();
  ContentOffset = CPoint(0, 0);

  SetBorderSizes(0, 0, 0, 0);

  if (Parent) {
    App = Parent->GetApplication();
    if (App) App->RegisterItem(this);
  }

  ApplyPosition(position);

  return true;
}

CRect CWBItem::GetClientRect() const {
  return CRect(0, 0, ClientRect.Width(), ClientRect.Height());
}

CRect CWBItem::GetWindowRect() const {
  return Position - (Position.TopLeft() + ClientRect.TopLeft());
}

CRect CWBItem::GetScreenRect() const { return ScreenRect; }

CPoint CWBItem::ClientToScreen(const CPoint& p) const {
  return ScreenRect.TopLeft() + ClientRect.TopLeft() + p;
}

CRect CWBItem::ClientToScreen(const CRect& p) const {
  return p + ScreenRect.TopLeft() + ClientRect.TopLeft();
}

CPoint CWBItem::ScreenToClient(const CPoint& p) const {
  return p - ScreenRect.TopLeft() - ClientRect.TopLeft();
}

CRect CWBItem::ScreenToClient(const CRect& p) const {
  return p - ScreenRect.TopLeft() - ClientRect.TopLeft();
}

CWBMessage CWBItem::BuildPositionMessage(const CRect& Pos) {
  CWBMessage m(App, WBM_REPOSITION, Guid);
  m.Rectangle = Pos;
  m.Moved = Pos.x1 != Position.x1 || Pos.y1 != Position.y1;
  m.Resized =
      Pos.Width() != Position.Width() || Pos.Height() != Position.Height();
  return m;
}

void CWBItem::SetPosition(const CRect& Pos) {
  const CWBMessage m = BuildPositionMessage(Pos);
  App->SendMessage(m);
}

void CWBItem::SetClientPadding(int32_t left, int32_t top, int32_t right,
                               int32_t bottom) {
  if (left != WBMARGIN_KEEP)
    CSSProperties.PositionDescriptor.SetValue(WBPOSITIONTYPE::WB_PADDING_LEFT,
                                              0, static_cast<float>(left));
  if (right != WBMARGIN_KEEP)
    CSSProperties.PositionDescriptor.SetValue(WBPOSITIONTYPE::WB_PADDING_RIGHT,
                                              0, static_cast<float>(right));
  if (top != WBMARGIN_KEEP)
    CSSProperties.PositionDescriptor.SetValue(WBPOSITIONTYPE::WB_PADDING_TOP, 0,
                                              static_cast<float>(top));
  if (bottom != WBMARGIN_KEEP)
    CSSProperties.PositionDescriptor.SetValue(WBPOSITIONTYPE::WB_PADDING_BOTTOM,
                                              0, static_cast<float>(bottom));

  CalculateClientPosition();
}

bool CWBItem::IsWidthSet() {
  return CSSProperties.PositionDescriptor.IsWidthSet();
}

bool CWBItem::IsHeightSet() {
  return CSSProperties.PositionDescriptor.IsHeightSet();
}

int32_t CWBItem::GetCalculatedWidth(CSize ParentSize) {
  return CSSProperties.PositionDescriptor.GetWidth(ParentSize,
                                                   StoredContentSize);
}

int32_t CWBItem::GetCalculatedHeight(CSize ParentSize) {
  return CSSProperties.PositionDescriptor.GetHeight(ParentSize,
                                                    StoredContentSize);
}

CRect CWBItem::GetPosition() { return Position; }

bool CWBItem::InFocus() {
  if (!Parent) return true;

  CWBItem* fi = App->GetRoot();
  while (fi) {
    if (fi == this) return true;
    fi = fi->ChildInFocus;
  }
  return false;
}

bool CWBItem::InLocalFocus() {
  if (!Parent) return true;
  return Parent->ChildInFocus == this;
}

void CWBItem::SetFocus() {
  const CWBItem* fi = App->GetFocusItem();
  if (fi != this) {
    App->SendMessage(CWBMessage(App, WBM_FOCUSGAINED, GetGuid()));
    if (fi) App->SendMessage(CWBMessage(App, WBM_FOCUSLOST, fi->GetGuid()));
  }

  CWBItem* p = Parent;
  CWBItem* i = this;
  while (p) {
    p->ChildInFocus = i;
    i = p;
    p = i->Parent;
  }
}

void CWBItem::ClearFocus() {
  // if (Parent) Parent->ChildInFocus = NULL;
  App->SendMessage(CWBMessage(App, WBM_FOCUSLOST, GetGuid()));
}

bool CWBItem::MouseOver() {
  CWBItem* mi = App->GetMouseItem();
  while (mi) {
    if (mi == this) return true;
    mi = mi->Parent;
  }
  return false;
}

void CWBItem::SavePosition() { StoredPosition = Position; }

CRect CWBItem::GetSavedPosition() const { return StoredPosition; }

void CWBItem::SetSavedPosition(const CRect& savedPos) {
  StoredPosition = savedPos;
}

uint32_t CWBItem::NumChildren() { return Children.size(); }

CWBItem* CWBItem::GetChild(uint32_t idx) { return Children[idx].get(); }

CSize CWBItem::GetContentSize() { return GetClientRect().Size(); }

void CWBItem::Hide(bool Hide) {
  App->SendMessage(CWBMessage(App, Hide ? WBM_HIDE : WBM_UNHIDE, GetGuid()));
}

bool CWBItem::IsHidden() { return Hidden; }

void CWBItem::SetData(void* data) { Data = data; }

void* CWBItem::GetData() { return Data; }

void CWBItem::MarkForDeletion() {
  auto sr = SelfRef.lock();
  App->AddToTrash(sr);
}

CWBContextMenu* CWBItem::OpenContextMenu(CPoint pos) {
  if (!App) return nullptr;
  auto ctx = CWBContextMenu::Create(
      App->GetRoot(), CRect(pos, pos + CPoint(10, 10)), GetGuid());
  App->ApplyStyle(ctx.get());
  return ctx.get();
}

void CWBItem::ScrollbardisplayHelperFunct(CWBScrollbarParams& s, int32_t& a1,
                                          int32_t& a2, int32_t& thumbsize,
                                          int32_t& thumbpos) {
  a1 += Scrollbar_ButtonSize;
  a2 -= Scrollbar_ButtonSize;

  const int32_t mi = s.MinScroll;
  const int32_t ma = s.MaxScroll;

  const float scrollsize = static_cast<float>(ma - mi);
  const float rs = std::max(0.0f, std::min(1.0f, s.ViewSize / scrollsize));
  float rp = std::max(
      0.0f, std::min(1.0f, (s.ScrollPos - mi) / (scrollsize - s.ViewSize)));

  thumbsize = static_cast<int32_t>(
      std::max(static_cast<float>(Scrollbar_ThumbMinimalSize), rs * (a2 - a1)));
  thumbpos = static_cast<int32_t>((a2 - thumbsize - a1) * rp) + a1;
}

int32_t CWBItem::CalculateScrollbarMovement(CWBScrollbarParams& s,
                                            int32_t scrollbarsize,
                                            int32_t delta) {
  int32_t a1 = 0;
  int32_t a2 = scrollbarsize;
  int32_t thumbsize = 0;
  int32_t thumbpos = 0;
  ScrollbardisplayHelperFunct(s, a1, a2, thumbsize, thumbpos);

  const int32_t mi = s.MinScroll;
  const int32_t ma = s.MaxScroll;
  const float scrollsize = static_cast<float>(ma - mi);

  const float sp = std::max(
      0.0f,
      std::min(1.0f, (s.DragStartPosition - mi) / (scrollsize - s.ViewSize)));
  const int32_t thumbposstart =
      static_cast<int32_t>((a2 - thumbsize - a1) * sp);

  const int32_t thumbposdelta =
      std::max(0, std::min(a2 - thumbsize - a1, thumbposstart + delta));
  int32_t newscrollpos =
      static_cast<int32_t>(
          (thumbposdelta / static_cast<float>(a2 - thumbsize - a1)) *
          (scrollsize - s.ViewSize)) +
      mi;

  // invalid state
  if (a2 - thumbsize - a1 == 0) {
    newscrollpos = s.ScrollPos;

    if (s.ScrollPos < s.MinScroll && s.ScrollPos + s.ViewSize > s.MaxScroll) {
      newscrollpos = s.MinScroll;
    } else if (s.ScrollPos < s.MinScroll) {
      newscrollpos = s.MinScroll;
    } else if (s.ScrollPos + s.ViewSize > s.MaxScroll) {
      newscrollpos = s.MaxScroll - s.ViewSize;
    }
    if (newscrollpos < s.MinScroll) {
      newscrollpos = s.MinScroll;
    }
  }

  return newscrollpos;
}

WBITEMSTATE CWBItem::GetScrollbarState(WBITEMVISUALCOMPONENT Component,
                                       CRect r) {
  // mouse not over item, early exit if dragging is not in effect
  const bool HBar =
      Component == WB_ITEM_SCROLL_HBAR || Component == WB_ITEM_SCROLL_HTHUMB ||
      Component == WB_ITEM_SCROLL_LEFT || Component == WB_ITEM_SCROLL_RIGHT;
  if (!MouseOver() && ((HBar && HScrollbar.Dragmode == WB_SCROLLDRAG_NONE) ||
                       (!HBar && VScrollbar.Dragmode == WB_SCROLLDRAG_NONE)))
    return WB_STATE_NORMAL;

  CPoint MousePos = App->GetMousePos();

  const CRect ScreenRect = ClientToScreen(r);
  const bool Hover = ScreenRect.Contains(MousePos);

  // don't highlight if something else uses the mouse (including this item)
  if (App->GetMouseCaptureItem())
    if (HScrollbar.Dragmode == WB_SCROLLDRAG_NONE &&
        VScrollbar.Dragmode == WB_SCROLLDRAG_NONE &&
        !App->GetMouseCaptureItem()->AllowMouseHighlightWhileCaptureItem())
      return WB_STATE_NORMAL;

  // scrollbar not clicked, return hover/normal mode
  if (HBar && HScrollbar.Dragmode == WB_SCROLLDRAG_NONE)
    return Hover ? WB_STATE_HOVER : WB_STATE_NORMAL;
  if (!HBar && VScrollbar.Dragmode == WB_SCROLLDRAG_NONE)
    return Hover ? WB_STATE_HOVER : WB_STATE_NORMAL;

  // handle clicked modes
  switch (Component) {
    case WB_ITEM_SCROLL_UP:
      if (VScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON1)
        return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
      break;
    case WB_ITEM_SCROLL_DOWN:
      if (VScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON2)
        return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
      break;
    case WB_ITEM_SCROLL_LEFT:
      if (HScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON1)
        return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
      break;
    case WB_ITEM_SCROLL_RIGHT:
      if (HScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON2)
        return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
      break;
    case WB_ITEM_SCROLL_HBAR:
      if (HScrollbar.Dragmode == WB_SCROLLDRAG_UP ||
          HScrollbar.Dragmode == WB_SCROLLDRAG_DOWN)
        return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
      if (HScrollbar.Dragmode == WB_SCROLLDRAG_THUMB) return WB_STATE_ACTIVE;
      break;
    case WB_ITEM_SCROLL_VBAR:
      if (VScrollbar.Dragmode == WB_SCROLLDRAG_UP ||
          VScrollbar.Dragmode == WB_SCROLLDRAG_DOWN)
        return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
      if (VScrollbar.Dragmode == WB_SCROLLDRAG_THUMB) return WB_STATE_ACTIVE;
      break;
    case WB_ITEM_SCROLL_HTHUMB:
      if (HScrollbar.Dragmode == WB_SCROLLDRAG_THUMB) return WB_STATE_ACTIVE;
      break;
    case WB_ITEM_SCROLL_VTHUMB:
      if (VScrollbar.Dragmode == WB_SCROLLDRAG_THUMB) return WB_STATE_ACTIVE;
      break;
    default:
      break;
  }

  return WB_STATE_NORMAL;
}

bool CWBItem::GetHScrollbarRectangles(CRect& button1, CRect& Scrollup,
                                      CRect& Thumb, CRect& Scrolldown,
                                      CRect& button2) {
  if (!HScrollbar.Enabled || !HScrollbar.Visible) return false;

  CRect r = CRect(GetClientRect().BottomLeft(),
                  GetClientRect().BottomRight() + CPoint(0, Scrollbar_Size));
  button1 =
      CRect(r.TopLeft(), r.BottomLeft() + CPoint(Scrollbar_ButtonSize, 0));
  button2 =
      CRect(r.TopRight() - CPoint(Scrollbar_ButtonSize, 0), r.BottomRight());

  int32_t thumbsize, thumbpos;
  ScrollbardisplayHelperFunct(HScrollbar, r.x1, r.x2, thumbsize, thumbpos);
  if (ScrollbarRequired(HScrollbar)) {
    Scrollup = CRect(r.x1, r.y1, thumbpos, r.y2);
    Thumb = CRect(thumbpos, r.y1, thumbpos + thumbsize, r.y2);
    Scrolldown = CRect(thumbpos + thumbsize, r.y1, r.x2, r.y2);
  } else
    Scrollup = Thumb = Scrolldown = CRect(1, 1, -1, -1);

  return true;
}

void CWBItem::DrawScrollbarButton(CWBDrawAPI* API, CWBScrollbarParams& s,
                                  CRect& r, WBITEMVISUALCOMPONENT Button) {
  const WBITEMSTATE State = GetScrollbarState(Button, r);
  const WBSKINELEMENTID ButtonSkin =
      CSSProperties.DisplayDescriptor.GetSkin(State, Button);

  if (ButtonSkin == 0xffffffff) {
    CColor color{0xff999999};
    if (State == WB_STATE_HOVER) color = CColor{0xff1c97ea};
    if (State == WB_STATE_ACTIVE) color = CColor{0xff007acc};
    if (!ScrollbarRequired(s)) color = CColor{0xff555558};

    const CPoint margin = CPoint(4, 4);
    API->DrawRect(CRect(r.TopLeft() + margin, r.BottomRight() - margin), color);
  } else
    App->GetSkin()->RenderElement(API, ButtonSkin, r);
}

void CWBItem::DrawHScrollbar(CWBDrawAPI* API) {
  if (!HScrollbar.Enabled || !HScrollbar.Visible) return;
  CRect b1, su, th, sd, b2;
  GetHScrollbarRectangles(b1, su, th, sd, b2);

  CRect pr = API->GetParentCropRect();
  const CSize RealClientRectSize =
      CSSProperties.PositionDescriptor
          .GetPadding(GetWindowRect().Size(), CSSProperties.BorderSizes)
          .Size();
  CRect RealClientRect =
      ClientToScreen(CRect(0, 0, RealClientRectSize.x, RealClientRectSize.y)) |
      pr;
  API->SetParentCropRect(RealClientRect);
  API->SetCropRect(RealClientRect);

  // draw background
  const WBITEMSTATE BackgroundState =
      GetScrollbarState(WB_ITEM_SCROLL_HBAR, th);
  const CRect BackgroundRect = CRect(b1.TopLeft(), b2.BottomRight());
  const WBSKINELEMENTID Background = CSSProperties.DisplayDescriptor.GetSkin(
      BackgroundState, WB_ITEM_SCROLL_HBAR);

  if (Background == 0xffffffff)
    API->DrawRect(BackgroundRect, CColor{0xff3e3e42});
  else
    App->GetSkin()->RenderElement(API, Background, BackgroundRect);

  // draw thumb
  const WBITEMSTATE ThumbState = GetScrollbarState(WB_ITEM_SCROLL_HTHUMB, th);
  const WBSKINELEMENTID Thumb = CSSProperties.DisplayDescriptor.GetSkin(
      ThumbState, WB_ITEM_SCROLL_HTHUMB);

  if (Thumb == 0xffffffff) {
    CColor color = CColor{0xff686868};
    if (ThumbState == WB_STATE_HOVER) color = CColor{0xff9e9e9e};
    if (ThumbState == WB_STATE_ACTIVE) color = CColor{0xffefebef};

    const CPoint thumbmargin = CPoint(0, 4);
    API->DrawRect(
        CRect(th.TopLeft() + thumbmargin, th.BottomRight() - thumbmargin),
        color);
  } else
    App->GetSkin()->RenderElement(API, Thumb, th);

  // draw buttons
  DrawScrollbarButton(API, HScrollbar, b2, WB_ITEM_SCROLL_RIGHT);
  DrawScrollbarButton(API, HScrollbar, b1, WB_ITEM_SCROLL_LEFT);

  API->SetParentCropRect(pr);
}

bool CWBItem::GetVScrollbarRectangles(CRect& button1, CRect& Scrollup,
                                      CRect& Thumb, CRect& Scrolldown,
                                      CRect& button2) {
  if (!VScrollbar.Enabled || !VScrollbar.Visible) return false;

  CRect r = CRect(GetClientRect().TopRight(),
                  GetClientRect().BottomRight() + CPoint(Scrollbar_Size, 0));
  button1 = CRect(r.TopLeft(), r.TopRight() + CPoint(0, Scrollbar_ButtonSize));
  button2 =
      CRect(r.BottomLeft() - CPoint(0, Scrollbar_ButtonSize), r.BottomRight());

  int32_t thumbsize, thumbpos;
  ScrollbardisplayHelperFunct(VScrollbar, r.y1, r.y2, thumbsize, thumbpos);
  if (ScrollbarRequired(VScrollbar)) {
    Scrollup = CRect(r.x1, r.y1, r.x2, thumbpos);
    Thumb = CRect(r.x1, thumbpos, r.x2, thumbpos + thumbsize);
    Scrolldown = CRect(r.x1, thumbpos + thumbsize, r.x2, r.y2);
  } else
    Scrollup = Thumb = Scrolldown = CRect(1, 1, -1, -1);

  return true;
}

void CWBItem::DrawVScrollbar(CWBDrawAPI* API) {
  if (!VScrollbar.Enabled || !VScrollbar.Visible) return;
  CRect b1, su, th, sd, b2;
  GetVScrollbarRectangles(b1, su, th, sd, b2);

  CRect pr = API->GetParentCropRect();
  const CSize RealClientRectSize =
      CSSProperties.PositionDescriptor
          .GetPadding(GetWindowRect().Size(), CSSProperties.BorderSizes)
          .Size();
  CRect RealClientRect =
      ClientToScreen(CRect(0, 0, RealClientRectSize.x, RealClientRectSize.y)) |
      pr;
  API->SetParentCropRect(RealClientRect);
  API->SetCropRect(RealClientRect);

  // draw background
  const WBITEMSTATE BackgroundState =
      GetScrollbarState(WB_ITEM_SCROLL_VBAR, th);
  const CRect BackgroundRect = CRect(b1.TopLeft(), b2.BottomRight());
  const WBSKINELEMENTID Background = CSSProperties.DisplayDescriptor.GetSkin(
      BackgroundState, WB_ITEM_SCROLL_VBAR);

  if (Background == 0xffffffff)
    API->DrawRect(BackgroundRect, CColor{0xff3e3e42});
  else
    App->GetSkin()->RenderElement(API, Background, BackgroundRect);

  // draw thumb
  const WBITEMSTATE ThumbState = GetScrollbarState(WB_ITEM_SCROLL_VTHUMB, th);
  const WBSKINELEMENTID Thumb = CSSProperties.DisplayDescriptor.GetSkin(
      ThumbState, WB_ITEM_SCROLL_VTHUMB);

  if (Thumb == 0xffffffff) {
    CColor color = CColor{0xff686868};
    if (ThumbState == WB_STATE_HOVER) color = CColor{0xff9e9e9e};
    if (ThumbState == WB_STATE_ACTIVE) color = CColor{0xffefebef};

    const CPoint thumbmargin = CPoint(4, 0);
    API->DrawRect(
        CRect(th.TopLeft() + thumbmargin, th.BottomRight() - thumbmargin),
        color);
  } else
    App->GetSkin()->RenderElement(API, Thumb, th);

  // draw buttons
  DrawScrollbarButton(API, VScrollbar, b2, WB_ITEM_SCROLL_DOWN);
  DrawScrollbarButton(API, VScrollbar, b1, WB_ITEM_SCROLL_UP);

  API->SetParentCropRect(pr);
}

bool CWBItem::ScrollbarDragged() {
  return HScrollbar.Dragmode != WB_SCROLLDRAG_NONE ||
         VScrollbar.Dragmode != WB_SCROLLDRAG_NONE;
}

void CWBItem::ScrollbarHelperFunct(CWBScrollbarParams& s, int32_t& r,
                                   bool ScrollbarNeeded) {
  if (!s.Enabled) {
    if (s.Visible) {
      // remove scrollbar
      r += Scrollbar_Size;
      s.Visible = false;
    }
  } else {
    if (!s.Dynamic) {
      if (!s.Visible) {
        // add scrollbar
        r -= Scrollbar_Size;
        s.Visible = true;
      }
    } else {
      if (ScrollbarNeeded != s.Visible) {
        if (s.Visible) {
          // remove scrollbar
          r += Scrollbar_Size;
        } else {
          // add scrollbar
          r -= Scrollbar_Size;
        }
        s.Visible = !s.Visible;
      }
    }
  }
}

bool CWBItem::ScrollbarRequired(CWBScrollbarParams& s) {
  return s.ViewSize < s.MaxScroll - s.MinScroll ||
         (s.ScrollPos < s.MinScroll &&
          s.ScrollPos + s.ViewSize < s.MaxScroll) ||
         (s.ScrollPos > s.MaxScroll - s.ViewSize && s.ScrollPos > s.MinScroll);
}

void CWBItem::AdjustClientAreaToFitScrollbars() {
  CRect crect = ClientRect;

  // x axis
  ScrollbarHelperFunct(VScrollbar, crect.x2, ScrollbarRequired(VScrollbar));

  // y axis
  ScrollbarHelperFunct(HScrollbar, crect.y2, ScrollbarRequired(HScrollbar));

  if (App && crect != ClientRect)
    App->SendMessage(CWBMessage(App, WBM_CLIENTAREACHANGED, GetGuid()));

  ClientRect = crect;
}

void CWBItem::EnableHScrollbar(bool Enabled, bool Dynamic) {
  HScrollbar.Enabled = Enabled;
  HScrollbar.Dynamic = Dynamic;
  AdjustClientAreaToFitScrollbars();
}

void CWBItem::EnableVScrollbar(bool Enabled, bool Dynamic) {
  VScrollbar.Enabled = Enabled;
  VScrollbar.Dynamic = Dynamic;
  AdjustClientAreaToFitScrollbars();
}

bool CWBItem::IsHScrollbarEnabled() { return HScrollbar.Enabled; }

bool CWBItem::IsVScrollbarEnabled() { return VScrollbar.Enabled; }

void CWBItem::SetHScrollbarParameters(int32_t MinScroll, int32_t MaxScroll,
                                      int32_t ViewSize) {
  const bool Changed = HScrollbar.MinScroll != MinScroll ||
                       HScrollbar.MaxScroll != MaxScroll ||
                       HScrollbar.ViewSize != ViewSize;
  HScrollbar.MinScroll = MinScroll;
  HScrollbar.MaxScroll = MaxScroll;
  HScrollbar.ViewSize = ViewSize;
  if (Changed) CalculateClientPosition();
}

void CWBItem::SetVScrollbarParameters(int32_t MinScroll, int32_t MaxScroll,
                                      int32_t ViewSize) {
  const bool Changed = VScrollbar.MinScroll != MinScroll ||
                       VScrollbar.MaxScroll != MaxScroll ||
                       VScrollbar.ViewSize != ViewSize;
  VScrollbar.MinScroll = MinScroll;
  VScrollbar.MaxScroll = MaxScroll;
  VScrollbar.ViewSize = ViewSize;
  if (Changed) CalculateClientPosition();
}

void CWBItem::GetHScrollbarParameters(int32_t& MinScroll, int32_t& MaxScroll,
                                      int32_t& ViewSize) {
  MinScroll = HScrollbar.MinScroll;
  MaxScroll = HScrollbar.MaxScroll;
  ViewSize = HScrollbar.ViewSize;
}

void CWBItem::GetVScrollbarParameters(int32_t& MinScroll, int32_t& MaxScroll,
                                      int32_t& ViewSize) {
  MinScroll = VScrollbar.MinScroll;
  MaxScroll = VScrollbar.MaxScroll;
  ViewSize = VScrollbar.ViewSize;
}

void CWBItem::SetHScrollbarPos(int32_t ScrollPos, bool Clamp) {
  int32_t sc = ScrollPos;
  if (Clamp)
    sc = std::max(HScrollbar.MinScroll,
                  std::min(sc, HScrollbar.MaxScroll - HScrollbar.ViewSize));
  if (sc != HScrollbar.ScrollPos) {
    HScrollbar.ScrollPos = sc;
    App->SendMessage(CWBMessage(App, WBM_HSCROLL, GetGuid(), sc));
    CalculateClientPosition();
  }
}

void CWBItem::SetVScrollbarPos(int32_t ScrollPos, bool Clamp) {
  int32_t sc = ScrollPos;
  if (Clamp)
    sc = std::max(VScrollbar.MinScroll,
                  std::min(sc, VScrollbar.MaxScroll - VScrollbar.ViewSize));
  if (sc != VScrollbar.ScrollPos) {
    VScrollbar.ScrollPos = sc;
    App->SendMessage(CWBMessage(App, WBM_VSCROLL, GetGuid(), sc));
    CalculateClientPosition();
  }
}

void CWBItem::ApplyRelativePosition() {
  if (Parent)
    SetPosition(CSSProperties.PositionDescriptor.GetPosition(
        Parent->GetClientRect().Size(), StoredContentSize, GetPosition()));
}

void CWBItem::VisualStyleApplicator(CWBDisplayProperties& desc,
                                    WBITEMVISUALCOMPONENT TargetComponent,
                                    int32_t Value,
                                    const std::vector<std::string>& pseudo) {
  int32_t StateCount = 0;
  for (size_t x = 1; x < pseudo.size(); x++) {
    const auto p = Trim(pseudo[x]);
    if (p == _T( "active" ) || p == _T( "hover" ) || p == _T( "disabled" ) ||
        p == _T( "disabled-active" ) || p == _T( "normal" ))
      StateCount++;
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
      if (p == _T( "active" )) {
        desc.SetValue(WB_STATE_ACTIVE, TargetComponent, Value);
        continue;
      }
      if (p == _T( "hover" )) {
        desc.SetValue(WB_STATE_HOVER, TargetComponent, Value);
        continue;
      }
      if (p == _T( "disabled" )) {
        desc.SetValue(WB_STATE_DISABLED, TargetComponent, Value);
        continue;
      }
      if (p == _T( "disabled-active" )) {
        desc.SetValue(WB_STATE_DISABLED_ACTIVE, TargetComponent, Value);
        continue;
      }
      if (p == _T( "normal" )) {
        desc.SetValue(WB_STATE_NORMAL, TargetComponent, Value);
        continue;
      }
    }
  }
}

bool CWBItem::InterpretPositionString(CWBCSSPropertyBatch& props,
                                      std::string_view prop,
                                      std::string_view value,
                                      const std::vector<std::string>& pseudo) {
  if (prop == _T( "left" )) {
    PositionApplicator(props.PositionDescriptor, WBPOSITIONTYPE::WB_MARGIN_LEFT,
                       value);
    return true;
  }

  if (prop == _T( "right" )) {
    PositionApplicator(props.PositionDescriptor,
                       WBPOSITIONTYPE::WB_MARGIN_RIGHT, value);
    return true;
  }

  if (prop == _T( "top" )) {
    PositionApplicator(props.PositionDescriptor, WBPOSITIONTYPE::WB_MARGIN_TOP,
                       value);
    return true;
  }

  if (prop == _T( "bottom" )) {
    PositionApplicator(props.PositionDescriptor,
                       WBPOSITIONTYPE::WB_MARGIN_BOTTOM, value);
    return true;
  }

  if (prop == _T( "width" )) {
    PositionApplicator(props.PositionDescriptor, WBPOSITIONTYPE::WB_WIDTH,
                       value);
    return true;
  }

  if (prop == _T( "height" )) {
    PositionApplicator(props.PositionDescriptor, WBPOSITIONTYPE::WB_HEIGHT,
                       value);
    return true;
  }

  if (prop == _T( "padding-left" )) {
    PositionApplicator(props.PositionDescriptor,
                       WBPOSITIONTYPE::WB_PADDING_LEFT, value);
    return true;
  }

  if (prop == _T( "padding-right" )) {
    PositionApplicator(props.PositionDescriptor,
                       WBPOSITIONTYPE::WB_PADDING_RIGHT, value);
    return true;
  }

  if (prop == _T( "padding-top" )) {
    PositionApplicator(props.PositionDescriptor, WBPOSITIONTYPE::WB_PADDING_TOP,
                       value);
    return true;
  }

  if (prop == _T( "padding-bottom" )) {
    PositionApplicator(props.PositionDescriptor,
                       WBPOSITIONTYPE::WB_PADDING_BOTTOM, value);
    return true;
  }

  if (prop == _T( "margin" )) {
    PositionApplicator(props.PositionDescriptor, WBPOSITIONTYPE::WB_MARGIN_LEFT,
                       value);
    PositionApplicator(props.PositionDescriptor,
                       WBPOSITIONTYPE::WB_MARGIN_RIGHT, value);
    PositionApplicator(props.PositionDescriptor, WBPOSITIONTYPE::WB_MARGIN_TOP,
                       value);
    PositionApplicator(props.PositionDescriptor,
                       WBPOSITIONTYPE::WB_MARGIN_BOTTOM, value);
    return true;
  }

  if (prop == _T( "padding" )) {
    PositionApplicator(props.PositionDescriptor,
                       WBPOSITIONTYPE::WB_PADDING_LEFT, value);
    PositionApplicator(props.PositionDescriptor,
                       WBPOSITIONTYPE::WB_PADDING_RIGHT, value);
    PositionApplicator(props.PositionDescriptor, WBPOSITIONTYPE::WB_PADDING_TOP,
                       value);
    PositionApplicator(props.PositionDescriptor,
                       WBPOSITIONTYPE::WB_PADDING_BOTTOM, value);
    return true;
  }

  int32_t dw = 0;

  if (prop == _T( "border" )) {
    if (ScanPXValue(value, dw, prop)) props.BorderSizes = CRect(dw, dw, dw, dw);
    return true;
  }

  if (prop == _T( "border-left" )) {
    if (ScanPXValue(value, dw, prop)) props.BorderSizes.x1 = dw;
    return true;
  }

  if (prop == _T( "border-top" )) {
    if (ScanPXValue(value, dw, prop)) props.BorderSizes.y1 = dw;
    return true;
  }

  if (prop == _T( "border-right" )) {
    if (ScanPXValue(value, dw, prop)) props.BorderSizes.x2 = dw;
    return true;
  }

  if (prop == _T( "border-bottom" )) {
    if (ScanPXValue(value, dw, prop)) props.BorderSizes.y2 = dw;
    return true;
  }

  return false;
}

std::vector<std::string> CWBItem::ExplodeValueWithoutSplittingParameters(
    std::string_view String) {
  std::vector<std::string> aOut;
  int nPrevious = 0;

#ifndef UNICODE
#define SPACE isspace
#define SPACETYPE unsigned char
#else
#define SPACE iswspace
#define SPACETYPE wchar_t
#endif

  unsigned int x = 0;
  int32_t bracketcnt = 0;

  while (x < String.size()) {
    while (x < String.size() && SPACE(static_cast<SPACETYPE>(String[x]))) x++;

    if (String[x] == _T('(')) bracketcnt++;
    if (String[x] == _T(')') && bracketcnt) bracketcnt--;

    nPrevious = x;
    while (x < String.size() &&
           (bracketcnt || (!SPACE(static_cast<SPACETYPE>(String[x]))))) {
      if (String[x] == _T('(')) bracketcnt++;
      if (String[x] == _T(')') && bracketcnt) bracketcnt--;
      x++;
    }

    aOut.emplace_back(String.substr(nPrevious, x - nPrevious));
  }

  return aOut;
}

bool CWBItem::InterpretDisplayString(CWBCSSPropertyBatch& props,
                                     std::string_view prop,
                                     std::string_view value,
                                     const std::vector<std::string>& pseudo) {
  if (prop == _T( "background" )) {
    auto Attribs = ExplodeValueWithoutSplittingParameters(value);

    for (const auto& attrib : Attribs) {
      uint32_t dw = 0;
      if (std::sscanf(attrib.c_str(), _T( "#%x" ), &dw) == 1)
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_BACKGROUNDCOLOR,
                              dw | 0xff000000, pseudo);
      if (attrib == (_T( "none" ))) {
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_BACKGROUNDCOLOR,
                              0, pseudo);
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_BACKGROUNDIMAGE,
                              0xffffffff, pseudo);
      }

      if (attrib == (_T( "left" )))
        VisualStyleApplicator(props.DisplayDescriptor,
                              WB_ITEM_BACKGROUNDALIGNMENT_X, WB_ALIGN_LEFT,
                              pseudo);
      if (attrib == (_T( "center" )))
        VisualStyleApplicator(props.DisplayDescriptor,
                              WB_ITEM_BACKGROUNDALIGNMENT_X, WB_ALIGN_CENTER,
                              pseudo);
      if (attrib == (_T( "right" )))
        VisualStyleApplicator(props.DisplayDescriptor,
                              WB_ITEM_BACKGROUNDALIGNMENT_X, WB_ALIGN_RIGHT,
                              pseudo);
      if (attrib == (_T( "top" )))
        VisualStyleApplicator(props.DisplayDescriptor,
                              WB_ITEM_BACKGROUNDALIGNMENT_Y, WB_ALIGN_TOP,
                              pseudo);
      if (attrib == (_T( "middle" )))
        VisualStyleApplicator(props.DisplayDescriptor,
                              WB_ITEM_BACKGROUNDALIGNMENT_Y, WB_ALIGN_MIDDLE,
                              pseudo);
      if (attrib == (_T( "bottom" )))
        VisualStyleApplicator(props.DisplayDescriptor,
                              WB_ITEM_BACKGROUNDALIGNMENT_Y, WB_ALIGN_BOTTOM,
                              pseudo);

      if (attrib.find(_T( "rgba(" )) == 0) {
        CColor col;
        if (!ParseRGBA(attrib, col)) {
          LOG_WARN("[gui] CSS rgba() description invalid, skipping: %s",
                   attrib.c_str());
          continue;
        }
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_BACKGROUNDCOLOR,
                              col.argb(), pseudo);
      }

      WBSKINELEMENTID id;
      if (ScanSkinValue(attrib, id, prop))
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_BACKGROUNDIMAGE,
                              id, pseudo);
    }

    return true;
  }

  if (prop == _T( "background-color" )) {
    auto Attribs = ExplodeValueWithoutSplittingParameters(value);

    for (const auto& attrib : Attribs) {
      uint32_t dw = 0;
      if (std::sscanf(attrib.c_str(), _T( "#%x" ), &dw) == 1)
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_BACKGROUNDCOLOR,
                              dw | 0xff000000, pseudo);
      if (attrib == (_T( "none" )))
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_BACKGROUNDCOLOR,
                              0, pseudo);

      if (attrib.find(_T( "rgba(" )) == 0) {
        CColor col;
        if (!ParseRGBA(attrib, col)) {
          LOG_WARN("[gui] CSS rgba() description invalid, skipping: %s",
                   attrib.c_str());
          continue;
        }
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_BACKGROUNDCOLOR,
                              col.argb(), pseudo);
      }
    }

    return true;
  }

  if (prop == _T( "foreground-color" )) {
    auto Attribs = ExplodeValueWithoutSplittingParameters(value);

    for (const auto& attrib : Attribs) {
      uint32_t dw = 0;
      if (std::sscanf(attrib.c_str(), _T( "#%x" ), &dw) == 1)
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_FOREGROUNDCOLOR,
                              dw | 0xff000000, pseudo);
      if (attrib == (_T( "none" )))
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_FOREGROUNDCOLOR,
                              0, pseudo);

      if (attrib.find(_T( "rgba(" )) == 0) {
        CColor col;
        if (!ParseRGBA(attrib, col)) {
          LOG_WARN("[gui] CSS rgba() description invalid, skipping: %s",
                   attrib.c_str());
          continue;
        }
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_FOREGROUNDCOLOR,
                              col.argb(), pseudo);
      }
    }

    return true;
  }

  if (prop == _T( "background-position" )) {
    auto Attribs = SplitByWhitespace(value);

    for (const auto& attrib : Attribs) {
      if (attrib == (_T( "left" )))
        VisualStyleApplicator(props.DisplayDescriptor,
                              WB_ITEM_BACKGROUNDALIGNMENT_X, WB_ALIGN_LEFT,
                              pseudo);
      if (attrib == (_T( "center" )))
        VisualStyleApplicator(props.DisplayDescriptor,
                              WB_ITEM_BACKGROUNDALIGNMENT_X, WB_ALIGN_CENTER,
                              pseudo);
      if (attrib == (_T( "right" )))
        VisualStyleApplicator(props.DisplayDescriptor,
                              WB_ITEM_BACKGROUNDALIGNMENT_X, WB_ALIGN_RIGHT,
                              pseudo);
      if (attrib == (_T( "top" )))
        VisualStyleApplicator(props.DisplayDescriptor,
                              WB_ITEM_BACKGROUNDALIGNMENT_Y, WB_ALIGN_TOP,
                              pseudo);
      if (attrib == (_T( "middle" )))
        VisualStyleApplicator(props.DisplayDescriptor,
                              WB_ITEM_BACKGROUNDALIGNMENT_Y, WB_ALIGN_MIDDLE,
                              pseudo);
      if (attrib == (_T( "bottom" )))
        VisualStyleApplicator(props.DisplayDescriptor,
                              WB_ITEM_BACKGROUNDALIGNMENT_Y, WB_ALIGN_BOTTOM,
                              pseudo);
    }

    return true;
  }

  if (prop == _T( "background-image" )) {
    auto Attribs = ExplodeValueWithoutSplittingParameters(value);

    for (const auto& attrib : Attribs) {
      if (attrib == (_T( "none" )))
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_BACKGROUNDIMAGE,
                              0xffffffff, pseudo);

      WBSKINELEMENTID id;
      if (ScanSkinValue(attrib, id, prop))
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_BACKGROUNDIMAGE,
                              id, pseudo);
    }

    return true;
  }

  if (prop == _T( "border-color" )) {
    uint32_t dw = 0;
    std::string v(value);
    std::sscanf(v.c_str(), _T( "#%x" ), &dw);
    VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_BORDERCOLOR,
                          dw | 0xff000000, pseudo);
    return true;
  }

  if (prop == _T( "opacity" )) {
    float dw = 0;
    std::string v(value);
    std::sscanf(v.c_str(), _T( "%f" ), &dw);

    const int32_t o =
        static_cast<int32_t>(std::max(0.f, std::min(255.f, dw * 255)));

    VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_OPACITY,
                          o * 0x01010101, pseudo);
    return true;
  }

  // quick check for scrollbar related stuff
  if (prop[0] == _T('s')) {
    WBSKINELEMENTID id;

    if (prop == _T( "scrollbar-up" )) {
      if (ScanSkinValue(value, id, prop))
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_SCROLL_UP, id,
                              pseudo);
      return true;
    }
    if (prop == _T( "scrollbar-down" )) {
      if (ScanSkinValue(value, id, prop))
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_SCROLL_DOWN, id,
                              pseudo);
      return true;
    }
    if (prop == _T( "scrollbar-left" )) {
      if (ScanSkinValue(value, id, prop))
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_SCROLL_LEFT, id,
                              pseudo);
      return true;
    }
    if (prop == _T( "scrollbar-right" )) {
      if (ScanSkinValue(value, id, prop))
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_SCROLL_RIGHT, id,
                              pseudo);
      return true;
    }
    if (prop == _T( "scrollbar-background-horizontal" )) {
      if (ScanSkinValue(value, id, prop))
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_SCROLL_HBAR, id,
                              pseudo);
      return true;
    }
    if (prop == _T( "scrollbar-background-vertical" )) {
      if (ScanSkinValue(value, id, prop))
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_SCROLL_VBAR, id,
                              pseudo);
      return true;
    }
    if (prop == _T( "scrollbar-thumb-horizontal" )) {
      if (ScanSkinValue(value, id, prop))
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_SCROLL_HTHUMB,
                              id, pseudo);
      return true;
    }
    if (prop == _T( "scrollbar-thumb-vertical" )) {
      if (ScanSkinValue(value, id, prop))
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_SCROLL_VTHUMB,
                              id, pseudo);
      return true;
    }
  }

  if (InterpretFontString(props, prop, value, pseudo)) return true;

  return false;
}

bool CWBItem::InterpretFontString(CWBCSSPropertyBatch& props,
                                  std::string_view prop, std::string_view value,
                                  const std::vector<std::string>& pseudo) {
  if (prop == _T( "font-color" )) {
    uint32_t dw = 0;
    std::string v(value);
    if (std::sscanf(v.c_str(), _T( "#%x" ), &dw) == 1) {
      VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_FONTCOLOR,
                            dw | 0xff000000, pseudo);
      return true;
    }

    CColor col;
    if (ParseRGBA(value, col))
      VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_FONTCOLOR,
                            col.argb(), pseudo);

    return true;
  }

  if (prop == _T( "text-transform" )) {
    if (value == (_T( "none" )))
      VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_TEXTTRANSFORM,
                            static_cast<int32_t>(WBTEXTTRANSFORM::WBTT_NONE),
                            pseudo);
    if (value == (_T( "capitalize" )))
      VisualStyleApplicator(
          props.DisplayDescriptor, WB_ITEM_TEXTTRANSFORM,
          static_cast<int32_t>(WBTEXTTRANSFORM::WBTT_CAPITALIZE), pseudo);
    if (value == (_T( "uppercase" )))
      VisualStyleApplicator(
          props.DisplayDescriptor, WB_ITEM_TEXTTRANSFORM,
          static_cast<int32_t>(WBTEXTTRANSFORM::WBTT_UPPERCASE), pseudo);
    if (value == (_T( "lowercase" )))
      VisualStyleApplicator(
          props.DisplayDescriptor, WB_ITEM_TEXTTRANSFORM,
          static_cast<int32_t>(WBTEXTTRANSFORM::WBTT_LOWERCASE), pseudo);

    return true;
  }

  if (prop == _T( "font" )) {
    auto Attribs = ExplodeValueWithoutSplittingParameters(value);

    for (const auto& attrib : Attribs) {
      // try to apply as color
      uint32_t dw = 0;
      if (std::sscanf(attrib.c_str(), _T( "#%x" ), &dw) == 1) {
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_FONTCOLOR,
                              dw | 0xff000000, pseudo);
        continue;
      }

      CColor col;
      if (ParseRGBA(attrib, col)) {
        VisualStyleApplicator(props.DisplayDescriptor, WB_ITEM_FONTCOLOR,
                              col.argb(), pseudo);
        continue;
      }

      // if failed apply as font
      FontStyleApplicator(props, pseudo, attrib);
    }

    return true;
  }

  if (prop == _T( "font-family" )) {
    FontStyleApplicator(props, pseudo, value);
    return true;
  }

  if (prop == _T( "text-align" )) {
    if (value == _T( "left" )) props.TextAlignX = WBTEXTALIGNMENTX::WBTA_LEFT;
    if (value == _T( "center" ))
      props.TextAlignX = WBTEXTALIGNMENTX::WBTA_CENTERX;
    if (value == _T( "right" )) props.TextAlignX = WBTEXTALIGNMENTX::WBTA_RIGHT;
    return true;
  }

  if (prop == _T( "vertical-align" )) {
    if (value == _T( "top" )) props.TextAlignY = WBTEXTALIGNMENTY::WBTA_TOP;
    if (value == _T( "middle" ))
      props.TextAlignY = WBTEXTALIGNMENTY::WBTA_CENTERY;
    if (value == _T( "bottom" ))
      props.TextAlignY = WBTEXTALIGNMENTY::WBTA_BOTTOM;
    return true;
  }

  return false;
}

bool CWBItem::ApplyStyle(std::string_view prop, std::string_view value,
                         const std::vector<std::string>& pseudo) {
  if (InterpretPositionString(CSSProperties, prop, value, pseudo)) {
    ContentChanged();
    return true;
  }
  if (InterpretDisplayString(CSSProperties, prop, value, pseudo)) {
    ContentChanged();
    return true;
  }

  if (prop == _T( "visibility" )) {
    if (value == _T( "hidden" )) {
      Hidden = true;
      return true;
    }
    if (value == _T( "visible" )) {
      Hidden = false;
      return true;
    }
    LOG_WARN("[guiitem] Item style error: invalid visibility value '%s'",
             std::string(value).c_str());
    return true;
  }

  if (prop == _T( "overflow" )) {
    if (value == _T( "hidden" )) {
      EnableHScrollbar(false, false);
      EnableVScrollbar(false, false);
      return true;
    }

    if (value == _T( "auto" )) {
      EnableHScrollbar(true, true);
      EnableVScrollbar(true, true);
      return true;
    }

    if (value == _T( "scroll" )) {
      EnableHScrollbar(true, false);
      EnableVScrollbar(true, false);
      return true;
    }
  }

  if (prop == _T( "overflow-x" )) {
    if (value == _T( "hidden" )) {
      EnableHScrollbar(false, false);
      return true;
    }

    if (value == _T( "auto" )) {
      EnableHScrollbar(true, true);
      return true;
    }

    if (value == _T( "scroll" )) {
      EnableHScrollbar(true, false);
      return true;
    }
  }

  if (prop == _T( "overflow-y" )) {
    if (value == _T( "hidden" )) {
      EnableVScrollbar(false, false);
      return true;
    }

    if (value == _T( "auto" )) {
      EnableVScrollbar(true, true);
      return true;
    }

    if (value == _T( "scroll" )) {
      EnableVScrollbar(true, false);
      return true;
    }
  }

  // quick check for scrollbar related stuff
  if (prop[0] == _T('s')) {
    int32_t dw = 0;

    if (prop == _T( "scrollbar-size" )) {
      if (ScanPXValue(value, dw, prop)) Scrollbar_Size = dw;
      return true;
    }

    if (prop == _T( "scrollbar-button-size" )) {
      if (ScanPXValue(value, dw, prop)) Scrollbar_ButtonSize = dw;
      return true;
    }

    if (prop == _T( "scrollbar-thumb-minimum-size" )) {
      if (ScanPXValue(value, dw, prop)) Scrollbar_ThumbMinimalSize = dw;
      return true;
    }
  }

  return false;
}

void CWBItem::PositionApplicator(CWBPositionDescriptor& pos,
                                 WBPOSITIONTYPE Type, std::string_view value) {
  if (Type == WBPOSITIONTYPE::WB_WIDTH || Type == WBPOSITIONTYPE::WB_HEIGHT) {
    if (value == _T( "none" )) {
      pos.ClearMetrics(Type);
      return;
    }
    if (value == _T( "auto" )) {
      pos.ClearMetrics(Type);
      pos.SetAutoSize(Type);
      return;
    }

  } else if (value == _T( "auto" )) {
    pos.ClearMetrics(Type);
    return;
  }

  const bool px = value.find("px") != std::string_view::npos;
  const bool pc = value.find('%') != std::string_view::npos;

  float pxv = 0;
  float pcv = 0;

  if (!pc && !px) {
    LOG_WARN("[guiitem] Item style error: missing 'px' or '%%' in '%s' value",
             std::string(value).c_str());
    return;
  }

  std::string v(value);
  if (px && !pc) {
    if (std::sscanf(v.c_str(), _T( "%fpx" ), &pxv) != 1) {
      LOG_WARN("[guiitem] Item style error: invalid value '%s' (px)",
               v.c_str());
      return;
    }
    pos.ClearMetrics(Type);
    pos.SetMetric(Type, WBMETRICTYPE::WB_PIXELS, pxv);
    return;
  }

  if (pc && !px) {
    if (std::sscanf(v.c_str(), _T( "%f%%" ), &pcv) != 1) {
      LOG_WARN("[guiitem] Item style error: invalid value '%s' (%%)",
               v.c_str());
      return;
    }
    pos.ClearMetrics(Type);
    pos.SetMetric(Type, WBMETRICTYPE::WB_RELATIVE, pcv / 100.0f);
    return;
  }

  if (std::sscanf(v.c_str(), _T( "%fpx%f%%" ), &pxv, &pcv) != 2)
    if (std::sscanf(v.c_str(), _T( "%f%%%fpx" ), &pcv, &pxv) != 2) {
      LOG_WARN("[guiitem] Item style error: invalid value '%s' (px, %%)",
               v.c_str());
      return;
    }

  pos.ClearMetrics(Type);
  pos.SetMetric(Type, WBMETRICTYPE::WB_PIXELS, pxv);
  pos.SetMetric(Type, WBMETRICTYPE::WB_RELATIVE, pcv / 100.0f);
}

CWBItem* CWBItem::FindChildByID(std::string_view value, std::string_view type) {
  CWBItem* i = ChildSearcherFunct(value, type);
  return i;
}

CWBItem* CWBItem::FindParentByID(std::string_view value,
                                 std::string_view type) {
  CWBItem* i = GetParent();
  while (i) {
    if (i->GetID() == value) {
      if (type.empty()) return i;
      if (i->InstanceOf(type)) return i;
    }
    i = i->GetParent();
  }
  return nullptr;
}

CWBItem* CWBItem::ChildSearcherFunct(std::string_view value,
                                     std::string_view type) {
  if (GetID() == value) {
    if (type.empty()) {
      return this;
    }
    if (InstanceOf(type)) {
      return this;
    }
  }

  for (auto& c : Children) {
    CWBItem* i = c->ChildSearcherFunct(value, type);
    if (i) {
      return i;
    }
  }

  return nullptr;
}

void CWBItem::ApplyStyleDeclarations(std::string_view String) {
  if (!App) return;
  App->StyleManager.ApplyStylesFromDeclarations(this, String);
}

WBITEMSTATE CWBItem::GetState() {
  WBITEMSTATE i = WB_STATE_NORMAL;
  if (Disabled) i = WB_STATE_DISABLED;
  if (MouseOver() && !Disabled) i = WB_STATE_HOVER;

  if (i == WB_STATE_HOVER && App->GetMouseCaptureItem())
    if (App->GetMouseCaptureItem() != this &&
        !App->GetMouseCaptureItem()->AllowMouseHighlightWhileCaptureItem())
      i = WB_STATE_NORMAL;

  if (InFocus()) i = Disabled ? WB_STATE_DISABLED_ACTIVE : WB_STATE_ACTIVE;
  return i;
}

void CWBItem::SetBorderSizes(char Left, char Top, char Right, char Bottom) {
  CSSProperties.BorderSizes = CRect(Left, Top, Right, Bottom);
}

void CWBItem::SetDisplayProperty(WBITEMSTATE s, WBITEMVISUALCOMPONENT v,
                                 int32_t value) {
  CSSProperties.DisplayDescriptor.SetValue(s, v, value);
}

CWBFont* CWBItem::GetFont(WBITEMSTATE State) {
  return CSSProperties.GetFont(App, State);
}

void CWBItem::ApplyOpacity(CWBDrawAPI* API) {
  CColor o =
      CSSProperties.DisplayDescriptor.GetColor(GetState(), WB_ITEM_OPACITY);
  API->SetOpacity(static_cast<uint8_t>(o.A() * OpacityMultiplier));
}

CWBPositionDescriptor& CWBItem::GetPositionDescriptor() {
  return CSSProperties.PositionDescriptor;
}

CSize CWBItem::GetClientWindowSizeDifference() {
  const CRect w = GetWindowRect();
  const CRect c = GetClientRect();

  return CSize(w.Width() - c.Width(), w.Height() - c.Height());
}

void CWBItem::Enable(bool Enabled) { Disabled = !Enabled; }

bool CWBItem::IsEnabled() { return !Disabled; }

void CWBItem::SetChildInFocus(CWBItem* i) {
  if (!i) {
    ChildInFocus = nullptr;
    return;
  }
  if (i->Parent == this) ChildInFocus = i;
}

bool CWBItem::ParseRGBA(std::string_view description, CColor& output) {
  auto Params = Split(description, _T( "," ));
  if (Params.size() < 3 || Params.size() > 4) return false;

  int32_t result = 0;
  int32_t c[3];
  result += std::sscanf(Params[0].c_str(), _T( "rgba(%d" ), &c[0]);
  result += std::sscanf(Params[1].c_str(), _T( "%d" ), &c[1]);
  result += std::sscanf(Params[2].c_str(), _T( "%d" ), &c[2]);

  if (result < 3) return false;

  float a = 1;

  if (Params.size() == 4)
    if (std::sscanf(Params[3].c_str(), _T( "%f" ), &a) != 1) return false;

  uint8_t Colors[3];
  for (int32_t y = 0; y < 3; y++) Colors[y] = std::max(0, std::min(255, c[y]));
  const uint8_t Alpha =
      static_cast<int32_t>(std::max(0.f, std::min(1.f, a)) * 255.f);

  output = CColor(Colors[0], Colors[1], Colors[2], Alpha);
  return true;
}

void CWBItem::FontStyleApplicator(CWBCSSPropertyBatch& desc,
                                  const std::vector<std::string>& pseudo,
                                  std::string_view name) {
  if (pseudo.size() <= 1) {
    desc.Fonts.clear();
    desc.Fonts[WB_STATE_NORMAL] = name;
  } else {
    for (size_t y = 1; y < pseudo.size(); y++) {
      const auto p = Trim(pseudo[y]);
      if (p == _T( "active" )) {
        desc.Fonts[WB_STATE_ACTIVE] = name;
        continue;
      }
      if (p == _T( "hover" )) {
        desc.Fonts[WB_STATE_HOVER] = name;
        continue;
      }
      if (p == _T( "disabled" )) {
        desc.Fonts[WB_STATE_DISABLED] = name;
        continue;
      }
      if (p == _T( "disabled-active" )) {
        desc.Fonts[WB_STATE_DISABLED_ACTIVE] = name;
        continue;
      }
      if (p == _T( "normal" )) {
        desc.Fonts[WB_STATE_NORMAL] = name;
        continue;
      }
    }
  }
}

bool CWBItem::ScanPXValue(std::string_view Value, int32_t& Result,
                          std::string_view PropName) {
  Result = 0;
  std::string v(Value);
  if (std::sscanf(v.c_str(), _T( "%dpx" ), &Result) != 1) {
    LOG_WARN("[guiitem] Item style error: invalid %s value '%s' (px)",
             std::string(PropName).c_str(), std::string(Value).c_str());
    return false;
  }
  return true;
}

bool CWBItem::ScanSkinValue(std::string_view Value, WBSKINELEMENTID& Result,
                            std::string_view PropName) {
  if (Value.find(_T( "skin(" )) == 0) {
    const int32_t i = Value.find(')');
    if (i != std::string_view::npos) {
      // Value.GetPointer()[ i ] = 0;
      Result = App->GetSkin()->GetElementID(Value.substr(5, i - 5));
      if (Result == 0xffffffff) {
        LOG_WARN("[gui] Skin element not found: %s",
                 std::string(Value.substr(5, i - 5)).c_str());
        return false;
      }

      return true;
    }
  }
  return false;
}

void CWBItem::SetFont(WBITEMSTATE State, std::string_view Font) {
  CSSProperties.Fonts[State] = Font;
}

void CWBItem::ContentChanged() {
  if (!CSSProperties.PositionDescriptor.IsAutoResizer()) return;

  const CSize ParentSize = GetWindowRect().Size();
  const CSize ClientSize =
      CSSProperties.PositionDescriptor
          .GetPadding(GetWindowRect().Size(), CSSProperties.BorderSizes)
          .Size();

  StoredContentSize = GetContentSize() + ParentSize - ClientSize;
  CWBMessage m = BuildPositionMessage(CRect(
      GetPosition().TopLeft(), GetPosition().TopLeft() + StoredContentSize));
  m.Resized = true;
  m.Moved = true;
  App->SendMessage(m);
}

void CWBItem::ChangeContentOffset(CPoint ContentOff) {
  if (ContentOff == ContentOffset) return;
  ContentOffset = ContentOff;
  for (uint32_t x = 0; x < NumChildren(); x++) GetChild(x)->UpdateScreenRect();
}

void CWBItem::ChangeContentOffsetX(int32_t OffsetX) {
  App->SendMessage(CWBMessage(App, WBM_CONTENTOFFSETCHANGE, GetGuid(), OffsetX,
                              ContentOffset.y));
}

void CWBItem::ChangeContentOffsetY(int32_t OffsetY) {
  App->SendMessage(CWBMessage(App, WBM_CONTENTOFFSETCHANGE, GetGuid(),
                              ContentOffset.x, OffsetY));
}

bool CWBItem::ScrollbarsEnabled() {
  return HScrollbar.Enabled || VScrollbar.Enabled;
}

void CWBItem::SetTreeOpacityMultiplier(float OpacityMul) {
  OpacityMultiplier = OpacityMul;
  for (auto& c : Children) {
    c->SetTreeOpacityMultiplier(OpacityMul);
  }
}

float CWBItem::GetTreeOpacityMultiplier() { return OpacityMultiplier; }

void CWBItem::ReapplyStyles() {
  App->StyleManager.ApplyStyles(this);
  CWBMessage m = BuildPositionMessage(GetPosition());
  m.Resized = true;
  MessageProc(m);
}

void CWBItem::SetForcedMouseTransparency(bool transparent) {
  ForceMouseTransparent = transparent;
}

bool CWBItem::MarkedForDeletion() {
  std::scoped_lock l(App->TrashMutex);
  auto it = std::find_if(
      App->Trash.begin(), App->Trash.end(),
      [this](const std::shared_ptr<CWBItem>& i) { return i.get() == this; });
  return (it != App->Trash.end());
}
