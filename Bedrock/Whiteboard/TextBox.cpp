#include "TextBox.h"

#include <algorithm>
#include <regex>

#include "../BaseLib/Timer.h"

INLINE void CWBTextBox::DrawCursor(CWBDrawAPI* API, CPoint& p) {
  WBITEMSTATE s = GetState();
  if (!(((globalTimer.GetTime() - CursorBlinkStartTime) / 500) % 2))
    API->DrawRect(
        CRect(p + CPoint(0, 1), p + CPoint(1, GetFont(s)->GetLineHeight() - 1)),
        CColor{0xffffffff});
}

void CWBTextBox::OnDraw(CWBDrawAPI* API) {
  // background
  DrawBackground(API);
  WBITEMSTATE i = GetState();
  CWBFont* Font = GetFont(i);
  WBTEXTTRANSFORM TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));
  int32_t TabWidth = Font->GetWidth(_T(' ')) * 4;

  CPoint Pos = CPoint(0, 0);
  CPoint Offset =
      -CPoint(GetHScrollbarPos(), GetVScrollbarPos()) + GetTextStartOffset();

  bool Focus = InFocus() && !GetChildInFocus();  // speedup

  API->SetCropToClient(this);

  CColor Color = CSSProperties.DisplayDescriptor.GetColor(i, WB_ITEM_FONTCOLOR);
  CColor BackgroundColor(0x00000000);
  CColor SelectionColor =
      Lerp(Selection.DisplayDescriptor.GetColor(WB_STATE_HOVER,
                                                WB_ITEM_BACKGROUNDCOLOR),
           Selection.DisplayDescriptor.GetColor(WB_STATE_ACTIVE,
                                                WB_ITEM_BACKGROUNDCOLOR),
           min(1.0f, (globalTimer.GetTime() - HiglightStartTime) / 300.0f));
  if (!InFocus())
    SelectionColor = Selection.DisplayDescriptor.GetColor(
        WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR);

  for (int32_t x = 0; x < static_cast<int32_t>(Text.size()); x++) {
    if (ColoredText())
      Color = GetTextColor(
          x, Color);  // color coding, to be implemented in child classes

    auto Char = Flags & WB_TEXTBOX_PASSWORD
                    ? PasswordStar
                    : Font->ApplyTextTransform(Text.c_str(), Text.c_str() + x,
                                               TextTransform);
    int32_t Width = Font->GetWidth(Char);

    if (Char == '\t')
      Width = (static_cast<int32_t>(Pos.x + TabWidth) / TabWidth) * TabWidth -
              Pos.x;

    // draw selection
    if (!(Flags & WB_TEXTBOX_NOSELECTION) && x >= SelectionStart &&
        x < SelectionEnd) {
      CRect Display =
          CRect(Pos, CPoint(Pos.x + Width, Pos.y + Font->GetLineHeight())) +
          Offset;
      if (Display.Intersects(GetClientRect()))
        API->DrawRect(Display, SelectionColor);
    }

    // draw background highlight

    if (GetTextBackground(x, BackgroundColor)) {
      CRect Display =
          CRect(Pos, CPoint(Pos.x + Width, Pos.y + Font->GetLineHeight())) +
          Offset;
      if (Display.Intersects(GetClientRect()))
        API->DrawRect(Display, BackgroundColor);
    }

    CPoint CPos = Pos;

    if (Char == '\n' || Char == '\t')  // special characters
    {
      // line feed
      if (Char == '\n' && !(Flags & WB_TEXTBOX_SINGLELINE)) {
        Pos.x = 0;
        Pos.y += Font->GetLineHeight();
      }
      if (Char == '\t') {
        Pos.x = (static_cast<int32_t>(Pos.x + TabWidth) / TabWidth) * TabWidth;
      }
    } else {
      // draw next character
      CRect Display =
          CRect(Pos, CPoint(Pos.x + Width, Pos.y + Font->GetLineHeight())) +
          Offset;
      if (Display.Intersects(GetClientRect()))
        Font->WriteChar(API, Char, Pos + Offset, Color);
      Pos.x += Width;
    }

    // draw cursor if needed
    if (Focus && CursorPos == x) DrawCursor(API, CPos + Offset);
  }

  // cursor at the end of text
  if (Focus && CursorPos == Text.size()) DrawCursor(API, Pos + Offset);

  DrawBorder(API);

  if (App->GetMouseCaptureItem() && App->GetMouseCaptureItem() != this) return;
  if (App->GetMouseItem() == this &&
      ClientToScreen(GetClientRect()).Contains(App->GetMousePos()))
    App->SelectMouseCursor(CM_TEXT);
}

CWBTextBox::CWBTextBox(CWBItem* Parent, const CRect& Pos, int32_t flags,
                       std::string_view Txt)
    : CWBItem() {
  Initialize(Parent, Pos, flags, Txt);
}

CWBTextBox::~CWBTextBox() = default;

bool CWBTextBox::Initialize(CWBItem* Parent, const CRect& Position,
                            int32_t flags, std::string_view Txt) {
  Flags = flags;
  Selection.DisplayDescriptor.SetValue(WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR,
                                       0);
  Selection.DisplayDescriptor.SetValue(WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR,
                                       0xff2b537d);
  Selection.DisplayDescriptor.SetValue(WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR,
                                       0xff3399ff);

  SetDisplayProperty(WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, 0xff1e1e1e);
  SetDisplayProperty(WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR, 0xff1e1e1e);
  SetDisplayProperty(WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR, 0xff1e1e1e);
  SetDisplayProperty(WB_STATE_DISABLED, WB_ITEM_BACKGROUNDCOLOR, 0xff1e1e1e);
  SetDisplayProperty(WB_STATE_DISABLED_ACTIVE, WB_ITEM_BACKGROUNDCOLOR,
                     0xff1e1e1e);

  CursorBlinkStartTime = globalTimer.GetTime();
  HiglightStartTime = 0;

  CursorPos = 0;
  SelectionStart = 0;
  SelectionEnd = 0;
  SelectionOrigin = 0;
  DesiredCursorPosXinPixels = 0;

  HistoryPosition = 0;

  if (!CWBItem::Initialize(Parent, Position)) return false;

  EnableHScrollbar(true, true);
  EnableVScrollbar(true, true);

  SetTextInternal(Txt, false, true);
  return true;
}

void CWBTextBox::SetCursorPos(int32_t pos, bool Selecting) {
  CWBFont* Font = GetFont(GetState());
  int32_t TabWidth = Font->GetWidth(_T(' ')) * 4;

  CursorBlinkStartTime = globalTimer.GetTime();

  if (!Selecting)
    SelectionStart = SelectionEnd = SelectionOrigin = CursorPos = pos;
  else {
    if (SelectionStart == SelectionEnd) SelectionOrigin = SelectionStart;
    CursorPos = pos;
    SelectionStart = min(CursorPos, SelectionOrigin);
    SelectionEnd = max(CursorPos, SelectionOrigin);
  }

  // adjust scrollbars so cursor is visible

  // determine cursor position in pixels
  CPoint CPos;
  CPos.y = GetCursorY() * Font->GetLineHeight();
  CPos.x = 0;
  int32_t Cx = GetCursorX();

  for (int32_t x = 0; x < Cx; x++) {
    TCHAR Char = Text[CursorPos - Cx + x];
    int32_t Width = Font->GetWidth(Char);

    if (Char == '\t')
      Width = (static_cast<int32_t>(CPos.x + TabWidth) / TabWidth) * TabWidth -
              CPos.x;

    CPos.x += Width;
  }

  int32_t CurrCharWidth = Font->GetWidth(_T(' '));
  if (CursorPos != Text.size()) CurrCharWidth = Font->GetWidth(Text[CursorPos]);

  OnCursorPosChange(CursorPos);

  if (Flags & WB_TEXTBOX_SINGLELINE) return;

  CRect VisibleRect =
      GetClientRect() + CPoint(GetHScrollbarPos(), GetVScrollbarPos());
  if (VisibleRect.Contains(CPos) &&
      VisibleRect.Contains(CPos + CPoint(CurrCharWidth, Font->GetLineHeight())))
    return;  // no need to adjust

  if (CPos.x < VisibleRect.x1)
    SetHScrollbarPos(GetHScrollbarPos() - (VisibleRect.x1 - CPos.x));
  if (CPos.x + CurrCharWidth > VisibleRect.x2)
    SetHScrollbarPos(GetHScrollbarPos() +
                     (CPos.x + CurrCharWidth - VisibleRect.x2));

  if (CPos.y < VisibleRect.y1)
    SetVScrollbarPos(GetVScrollbarPos() - (VisibleRect.y1 - CPos.y));
  if (CPos.y + Font->GetLineHeight() > VisibleRect.y2)
    SetVScrollbarPos(GetVScrollbarPos() +
                     (CPos.y + Font->GetLineHeight() - VisibleRect.y2));
}

void CWBTextBox::SetCursorPosXpxY(int32_t x, int32_t y, bool Selecting) {
  int32_t p = 0;
  WBITEMSTATE i = GetState();
  CWBFont* Font = GetFont(i);
  WBTEXTTRANSFORM TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));
  int32_t TabWidth = Font->GetWidth(_T(' ')) * 4;

  if (!(y < 0 || (y == 0 && x < 0))) {
    int32_t yp = 0;
    while (yp < y) {
      if (p == Text.size()) {
        SetCursorPos(p, Selecting);
        return;
      }
      if (Text[p] == '\n') yp++;
      p++;
    }
  }

  int32_t pd = p;
  int32_t xp = 0;
  while (xp < x) {
    if (p == Text.size() || Text[p] == '\n') {
      // end of line
      SetCursorPos(p, Selecting);
      return;
    }

    auto Char = Flags & WB_TEXTBOX_PASSWORD
                    ? PasswordStar
                    : Font->ApplyTextTransform(Text.c_str(), Text.c_str() + p,
                                               TextTransform);
    int32_t Width = Font->GetWidth(Char);

    if (Char == '\t')
      Width = (static_cast<int32_t>(xp + TabWidth) / TabWidth) * TabWidth - xp;

    xp += Width;
    if (xp > x) break;
    p++;
  }

  // LOG(LOG_DEBUG,_T("[gui] Set Cursor Pos: %d (%d)"),xp,p-pd);

  SetCursorPos(p, Selecting);
}

void CWBTextBox::RemoveSelectedText() {
  if (SelectionStart != SelectionEnd) {
    RemoveText(SelectionStart, SelectionEnd - SelectionStart, SelectionStart);
  }
}

void CWBTextBox::Cut() {
  if (Flags & WB_TEXTBOX_NOSELECTION) return;

  Copy();
  RemoveSelectedText();
  DesiredCursorPosXinPixels = GetCursorXinPixels();
  OnTextChange();
}

void CWBTextBox::Copy() {
  if (Flags & WB_TEXTBOX_NOSELECTION) return;
  if (Flags & WB_TEXTBOX_DISABLECOPY) return;

  int32_t strt = SelectionStart;
  int32_t end = SelectionEnd;

  if (SelectionStart == SelectionEnd) {
    // copy whole line...
  }

  if (strt == end) return;

  if (OpenClipboard((HWND)App->GetHandle())) {
    EmptyClipboard();

    auto winnewline = std::make_unique<char[]>(end - strt + 1);
    memcpy(winnewline.get(), Text.c_str() + strt, (end - strt));
    winnewline[end - strt] = 0;
    std::string out(winnewline.get());
    out.erase(std::remove(out.begin(), out.end(), '\r'), out.end());
    out = std::regex_replace(out, std::regex("\n"), std::string{'\n', '\r'});
    winnewline.reset();

    HGLOBAL clipbuffer = GlobalAlloc(GMEM_DDESHARE | GHND, (out.size() + 1));

    if (clipbuffer) {
      auto buffer = static_cast<char*>(GlobalLock(clipbuffer));
      if (buffer) {
        memcpy(buffer, out.c_str(), out.size());
        buffer[out.size()] = 0;
      }

      GlobalUnlock(clipbuffer);

#ifndef UNICODE
      SetClipboardData(CF_TEXT, clipbuffer);
#else
      SetClipboardData(CF_UNICODETEXT, clipbuffer);
#endif
    }

    CloseClipboard();

    HiglightStartTime = globalTimer.GetTime();
  } else
    LOG(LOG_WARNING, _T( "[gui] Failed to open clipboard" ));
}

void CWBTextBox::Paste() {
  if (OpenClipboard((HWND)App->GetHandle())) {
    HANDLE Handle = GetClipboardData(CF_UNICODETEXT);

    auto buffer = static_cast<wchar_t*>(GlobalLock(Handle));
    if (buffer) {
#ifndef UNICODE
      int32_t len = wcslen(buffer);
      auto b2 = std::make_unique<char[]>(len + 1);
      memset(b2.get(), 0, len + 1);
      for (int32_t x = 0; x < len; x++) {
        if (buffer[x] >= 0 && buffer[x] <= 255)
          b2[x] = static_cast<char>(buffer[x]);
        else
          b2[x] = '?';
      }
      std::string s(b2.get());
#else
      s = CString(buffer);
#endif
      s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());

      RemoveSelectedText();
      InsertText(CursorPos, s.c_str(), s.size(), CursorPos + s.size());
      GlobalUnlock(Handle);
      DesiredCursorPosXinPixels = GetCursorXinPixels();
    } else {
      LOG(LOG_WARNING, _T( "[gui] Failed to retrieve clipboard data (%x)" ),
          GetLastError());
    }

    CloseClipboard();
    OnTextChange();
  } else
    LOG(LOG_WARNING, _T( "[gui] Failed to open clipboard" ));
}

CWBTextBoxHistoryEntry* CWBTextBox::CreateNewHistoryEntry(bool Remove,
                                                          int Start,
                                                          int Length) {
  for (; History.size() > HistoryPosition;) {
    History.pop_back();
  }

  auto entry = std::make_unique<CWBTextBoxHistoryEntry>();
  entry->Remove = Remove;
  entry->StartPosition = Start;
  entry->Data = Text.substr(Start, Length);
  History.emplace_back(std::move(entry));

  HistoryPosition = History.size();

  return History.back().get();
}

void CWBTextBox::Undo() {
  if (HistoryPosition == 0) {
    return;
  }
  HistoryPosition--;

  auto& e = History[HistoryPosition];

  if (e->Remove)
    InsertText(e->StartPosition, e->Data, e->Data.size(), e->CursorPos_Before,
               false);
  else
    RemoveText(e->StartPosition, e->Data.size(), e->CursorPos_Before, false);

  SelectionStart = e->SelectionStart_Before;
  SelectionEnd = e->SelectionEnd_Before;

  OnTextChange();
}

void CWBTextBox::Redo() {
  if (HistoryPosition >= History.size()) {
    return;
  }

  auto& e = History[HistoryPosition++];

  if (e->Remove)
    RemoveText(e->StartPosition, e->Data.size(), e->StartPosition, false);
  else
    InsertText(e->StartPosition, e->Data, e->Data.size(),
               e->StartPosition + e->Data.size(), false);

  SelectionStart = CursorPos;
  SelectionEnd = CursorPos;

  OnTextChange();
}

int32_t CWBTextBox::GetCursorX() {
  int32_t cnt = 0;
  for (int32_t x = CursorPos - 1; x >= 0; x--) {
    if (Text[x] == '\n') return cnt;
    cnt++;
  }
  return cnt;
}

int32_t CWBTextBox::GetCursorY() {
  int32_t cnt = 0;
  for (int32_t x = CursorPos - 1; x >= 0; x--)
    if (Text[x] == '\n') cnt++;
  return cnt;
}

int32_t CWBTextBox::GetCursorXinPixels() {
  WBITEMSTATE i = GetState();
  CWBFont* Font = GetFont(i);
  WBTEXTTRANSFORM TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));
  int32_t TabWidth = Font->GetWidth(_T(' ')) * 4;

  int32_t c = GetCursorX();
  int32_t Pixels = 0;
  for (int32_t x = CursorPos - c; x < CursorPos; x++) {
    auto Char = Flags & WB_TEXTBOX_PASSWORD
                    ? PasswordStar
                    : Font->ApplyTextTransform(Text.c_str(), Text.c_str() + x,
                                               TextTransform);
    int32_t Width = Font->GetWidth(Char);

    if (Char == '\t')
      Width = (static_cast<int32_t>(Pixels + TabWidth) / TabWidth) * TabWidth -
              Pixels;

    Pixels += Width;
  }

  return Pixels;
}

int32_t CWBTextBox::GetLineSize() {
  int32_t ls = CursorPos - GetCursorX();
  int32_t cnt = 0;
  for (int32_t x = ls; x < static_cast<int32_t>(Text.size()); x++) {
    if (Text[x] == '\n') return cnt;
    cnt++;
  }
  return cnt;
}

int32_t CWBTextBox::GetLineLeadingWhiteSpaceSize() {
  int32_t ls = CursorPos - GetCursorX();
  int32_t cnt = 0;
  for (int32_t x = ls; x < static_cast<int32_t>(Text.size()); x++) {
    if (Text[x] == '\n' || !_istspace(Text[x])) return cnt;
    cnt++;
  }
  return cnt;
}

CPoint CWBTextBox::GetTextStartOffset() {
  CPoint pos(0, 0);
  if (!(Flags & WB_TEXTBOX_SINGLELINE)) return pos;

  WBITEMSTATE i = GetState();
  CWBFont* Font = GetFont(i);
  WBTEXTTRANSFORM TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));
  CPoint TextPos =
      Font->GetTextPosition(Text, GetClientRect(), CSSProperties.TextAlignX,
                            CSSProperties.TextAlignY, TextTransform);

  if (!IsHScrollbarEnabled()) pos.x = TextPos.x;
  if (!IsVScrollbarEnabled()) pos.y = TextPos.y;

  return pos;
}

int32_t CWBTextBox::GetCursorPosMouse() {
  WBITEMSTATE i = GetState();
  CWBFont* Font = GetFont(i);
  WBTEXTTRANSFORM TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));
  int32_t TabWidth = Font->GetWidth(_T(' ')) * 4;

  CPoint mp = ScreenToClient(App->GetMousePos());
  CPoint Pos = CPoint(0, 0);
  CPoint Offset =
      -CPoint(GetHScrollbarPos(), GetVScrollbarPos()) + GetTextStartOffset();
  CRect cr = GetClientRect();

  if (mp.y < Pos.y + Offset.y) return 0;

  for (int32_t x = 0; x < static_cast<int32_t>(Text.size()); x++) {
    if ((Pos.y + Font->GetLineHeight() + Offset.y >= 0 &&
         Pos.y + Offset.y < cr.Height()) ||
        Flags & WB_TEXTBOX_SINGLELINE) {
      auto Char = Flags & WB_TEXTBOX_PASSWORD
                      ? PasswordStar
                      : Font->ApplyTextTransform(Text.c_str(), Text.c_str() + x,
                                                 TextTransform);
      int32_t Width = Font->GetWidth(Char);

      if (Char == '\t')
        Width = (static_cast<int32_t>(Pos.x + TabWidth) / TabWidth) * TabWidth -
                Pos.x;

      if ((Pos.y + Offset.y <= mp.y &&
           mp.y < Pos.y + Font->GetLineHeight() + Offset.y) ||
          Flags & WB_TEXTBOX_SINGLELINE)  // we're in the correct line
      {
        if (mp.x < Pos.x + Offset.x) return x;  // for line starts
        if (Pos.x + Offset.x <= mp.x &&
            mp.x < Pos.x + Width + Offset.x)  // we're in the correct column
          return x;
      }

      Pos.x += Width;
    }

    if (Text[x] == '\n') {
      if (Pos.y + Offset.y <= mp.y &&
          mp.y < Pos.y + Font->GetLineHeight() + Offset.y)
        return x;
      Pos.x = 0;
      Pos.y += Font->GetLineHeight();
    }
  }
  return Text.size();
}

bool CWBTextBox::MessageProc(const CWBMessage& Message) {
  switch (Message.GetMessage()) {
    case WBM_FOCUSGAINED:
      if (Message.GetTarget() == GetGuid()) {
        OriginalText = Text;
        if (GetChildInFocus()) GetChildInFocus()->ClearFocus();
        return true;
      }
      break;

    case WBM_FOCUSLOST:
      if (Message.GetTarget() == GetGuid()) {
        if (GetParent() && GetParent()->GetChildInFocus() == this)
          GetParent()->SetChildInFocus(nullptr);
        return true;
      }
      break;

    case WBM_REPOSITION: {
      bool b = CWBItem::MessageProc(Message);
      int32_t mi, ma, vi;
      GetHScrollbarParameters(mi, ma, vi);
      SetHScrollbarParameters(mi, ma, GetClientRect().Width());
      GetVScrollbarParameters(mi, ma, vi);
      SetVScrollbarParameters(mi, ma, GetClientRect().Height());
      SetCursorPos(CursorPos, false);
      return b;
    } break;

    case WBM_LEFTBUTTONDOWN:
      if (App->GetMouseItem() != this) break;
      if (CWBItem::MessageProc(Message)) return true;
      App->SetCapture(this);
      SetCursorPos(GetCursorPosMouse(), App->GetShiftState());
      DesiredCursorPosXinPixels = GetCursorXinPixels();
      return true;

    case WBM_LEFTBUTTONDBLCLK:
      if (App->GetMouseItem() != this) break;
      if (CWBItem::MessageProc(Message)) return true;
      if (App->GetShiftState()) return true;
      SelectWord(GetCursorPosMouse());
      return true;

    case WBM_MOUSEMOVE:
      if (CWBItem::MessageProc(Message)) return true;
      if (App->GetMouseCaptureItem() == this) {
        SetCursorPos(GetCursorPosMouse(), true);
        DesiredCursorPosXinPixels = GetCursorXinPixels();
      }
      return true;

    case WBM_MOUSEWHEEL: {
      CWBFont* Font = GetFont(GetState());
      int32_t TabWidth = Font->GetWidth(_T(' ')) * 4;
      SetVScrollbarPos(
          GetVScrollbarPos() - Message.Data * 3 * Font->GetLineHeight(), true);
    }
      return true;

    case WBM_LEFTBUTTONUP:
      if (CWBItem::MessageProc(Message)) return true;
      App->ReleaseCapture();
      return true;

    case WBM_KEYDOWN: {
      if (!InFocus() || GetChildInFocus()) break;
      if (Message.KeyboardState & WB_KBSTATE_ALT &&
          Message.KeyboardState & WB_KBSTATE_CTRL)
        break;  // altgr

      CWBFont* Font = GetFont(GetState());
      int32_t TabWidth = Font->GetWidth(_T(' ')) * 4;

      // handle cursor movement keys
      switch (Message.Key) {
        case VK_LEFT:
          SetCursorPos(max(0, CursorPos - 1),
                       Message.KeyboardState & WB_KBSTATE_SHIFT);
          DesiredCursorPosXinPixels = GetCursorXinPixels();
          return true;

        case VK_RIGHT:
          SetCursorPos(min((int32_t)Text.size(), CursorPos + 1),
                       Message.KeyboardState & WB_KBSTATE_SHIFT);
          DesiredCursorPosXinPixels = GetCursorXinPixels();
          return true;

        case VK_UP:
          SetCursorPosXpxY(DesiredCursorPosXinPixels, GetCursorY() - 1,
                           Message.KeyboardState & WB_KBSTATE_SHIFT);
          return true;

        case VK_DOWN:
          SetCursorPosXpxY(DesiredCursorPosXinPixels, GetCursorY() + 1,
                           Message.KeyboardState & WB_KBSTATE_SHIFT);
          return true;

        case VK_PRIOR:  // page up
          SetVScrollbarPos(GetVScrollbarPos() -
                               (static_cast<int32_t>(GetClientRect().Height() /
                                                     Font->GetLineHeight())) *
                                   Font->GetLineHeight(),
                           true);
          SetCursorPosXpxY(DesiredCursorPosXinPixels,
                           GetVScrollbarPos()
                               ? GetCursorY() - GetClientRect().Height() /
                                                    Font->GetLineHeight()
                               : 0,
                           Message.KeyboardState & WB_KBSTATE_SHIFT);
          return true;

        case VK_NEXT:  // page down
          SetVScrollbarPos(GetVScrollbarPos() +
                               (static_cast<int32_t>(GetClientRect().Height() /
                                                     Font->GetLineHeight())) *
                                   Font->GetLineHeight(),
                           true);
          SetCursorPosXpxY(
              DesiredCursorPosXinPixels,
              GetCursorY() + GetClientRect().Height() / Font->GetLineHeight(),
              Message.KeyboardState & WB_KBSTATE_SHIFT);
          return true;

        case VK_HOME:
          SetHScrollbarPos(0);
          if (Message.KeyboardState & WB_KBSTATE_CTRL) {
            SetCursorPos(0, Message.KeyboardState & WB_KBSTATE_SHIFT);
            DesiredCursorPosXinPixels = GetCursorXinPixels();
            return true;
          }

          // skip to the start of the line
          if (GetCursorX() == GetLineLeadingWhiteSpaceSize())
            SetCursorPos(CursorPos - GetCursorX(),
                         Message.KeyboardState & WB_KBSTATE_SHIFT);
          else
            SetCursorPos(
                CursorPos - GetCursorX() + GetLineLeadingWhiteSpaceSize(),
                Message.KeyboardState & WB_KBSTATE_SHIFT);
          DesiredCursorPosXinPixels = GetCursorXinPixels();
          return true;

        case VK_END:
          if (Message.KeyboardState & WB_KBSTATE_CTRL) {
            SetCursorPos(Text.size(), Message.KeyboardState & WB_KBSTATE_SHIFT);
            DesiredCursorPosXinPixels = GetCursorXinPixels();
            return true;
          }

          // skip to the end of the line
          for (; CursorPos < static_cast<int32_t>(Text.size()); CursorPos++)
            if (Text[CursorPos] == _T('\n')) {
              SetCursorPos(CursorPos, Message.KeyboardState & WB_KBSTATE_SHIFT);
              DesiredCursorPosXinPixels = GetCursorXinPixels();
              break;
            }
          SetCursorPos(CursorPos, Message.KeyboardState & WB_KBSTATE_SHIFT);
          DesiredCursorPosXinPixels = GetCursorXinPixels();
          return true;

        case VK_RETURN:
          if (Flags & WB_TEXTBOX_SINGLELINE) {
            App->SendMessage(CWBMessage(App, WBM_COMMAND, GetGuid()));
            ClearFocus();
            return true;
          }
          break;

        case VK_DELETE:
          if (Message.KeyboardState & WB_KBSTATE_SHIFT)
            Cut();
          else {
            if (CursorPos < 0 ||
                CursorPos >= static_cast<int32_t>(Text.size())) {
              RemoveSelectedText();
              OnTextChange();
              return true;
            }

            if (SelectionStart == SelectionEnd)
              RemoveText(CursorPos, 1, CursorPos);
            else
              RemoveSelectedText();
            DesiredCursorPosXinPixels = GetCursorXinPixels();
            OnTextChange();
          }
          return true;

        case VK_ESCAPE:
          if ((Flags & WB_TEXTBOX_SINGLELINE)) {
            SetTextInternal(OriginalText, false, false);
            App->SendMessage(CWBMessage(App, WBM_COMMAND, GetGuid()));
            ClearFocus();
            return true;
          }

          return true;

        case VK_INSERT:
          if (Message.KeyboardState & WB_KBSTATE_CTRL)
            Copy();
          else if (Message.KeyboardState & WB_KBSTATE_SHIFT)
            Paste();
          return true;

        case 'X':
          if (Message.KeyboardState & WB_KBSTATE_CTRL) Cut();
          return true;

        case 'C':
          if (Message.KeyboardState & WB_KBSTATE_CTRL) Copy();
          return true;

        case 'V':
          if (Message.KeyboardState & WB_KBSTATE_CTRL) Paste();
          return true;

        case 'A':  // select all
          if (Message.KeyboardState & WB_KBSTATE_CTRL) {
            SetCursorPos(0, false);
            SetCursorPos(Text.size(), true);
          }
          return true;

        case 'Z':
          if (Message.KeyboardState & WB_KBSTATE_CTRL) Undo();
          return true;

        case 'Y':
          if (Message.KeyboardState & WB_KBSTATE_CTRL) Redo();
          return true;
      }

      return false;

      return true;  // this captures all keydowns. might not be a good idea all
                    // the time
    }
    case WBM_CHAR:
      if (!InFocus() || GetChildInFocus()) break;
      if (!(Message.KeyboardState & WB_KBSTATE_CTRL &&
            Message.KeyboardState & WB_KBSTATE_ALT))  // altgr
        if (Message.KeyboardState & (WB_KBSTATE_ALT | WB_KBSTATE_CTRL))
          return true;  // these should be handled by the keydown messages

      if (Message.Key == VK_BACK) {
        if (SelectionStart == SelectionEnd) {
          if (!CursorPos) return true;
          RemoveText(CursorPos - 1, 1, CursorPos - 1);
        } else
          RemoveSelectedText();
        OnTextChange();

        return true;
      }

      if (Message.Key == VK_ESCAPE) {
        SelectionStart = SelectionEnd = SelectionOrigin = CursorPos;
        return true;  // shouldn't produce text
      }

      if ((Flags & WB_TEXTBOX_SINGLELINE) && Message.Key == VK_RETURN)
        return true;  // already handled in the keydown message

      // translate VK_RETURN to '\n':

      int32_t Key = Message.Key;

      if (Message.Key == VK_RETURN) Key = _T('\n');

      // insert character
      RemoveSelectedText();
      InsertText(CursorPos, reinterpret_cast<TCHAR*>(&Key), 1, CursorPos + 1);

      PostCharInsertion(CursorPos, Message.Key);

      DesiredCursorPosXinPixels = GetCursorXinPixels();
      OnTextChange();

      return true;  // this captures all character inputs which should be fine
  }

  return CWBItem::MessageProc(Message);
}

void CWBTextBox::SetText(std::string_view val, bool EnableUndo) {
  SetTextInternal(val, EnableUndo, true);
}

void CWBTextBox::SetTextInternal(std::string_view val, bool EnableUndo,
                                 bool nonHumanInteraction) {
  if (!EnableUndo) {
    OriginalText = Text = val;
    Text.erase(std::remove(Text.begin(), Text.end(), '\r'), Text.end());
    History.clear();
    HistoryPosition = 0;
  } else {
    SelectionStart = 0;
    SelectionEnd = Text.size();
    RemoveSelectedText();
    std::string s(val);
    s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
    InsertText(0, s, s.size(), s.size());
  }

  SetCursorPos(Text.size(), false);
  OnTextChange(nonHumanInteraction != 0);
}

void CWBTextBox::OnTextChange(bool nonHumanInteraction /* = false*/) {
  // calculate new scrollbar data
  WBITEMSTATE i = GetState();
  CWBFont* Font = GetFont(i);
  WBTEXTTRANSFORM TextTransform = static_cast<WBTEXTTRANSFORM>(
      CSSProperties.DisplayDescriptor.GetValue(i, WB_ITEM_TEXTTRANSFORM));
  int32_t TabWidth = Font->GetWidth(_T(' ')) * 4;

  App->SendMessage(
      CWBMessage(App, WBM_TEXTCHANGED, GetGuid(),
                 App->GetFocusItem() == this && !nonHumanInteraction));

  CPoint Size = CPoint(0, Font->GetLineHeight());
  int32_t XSize = 0;

  for (int32_t x = 0; x < static_cast<int32_t>(Text.size()); x++) {
    auto Char = Flags & WB_TEXTBOX_PASSWORD
                    ? PasswordStar
                    : Font->ApplyTextTransform(Text.c_str(), Text.c_str() + x,
                                               TextTransform);
    int32_t Width = Font->GetWidth(Char);

    if (Char == '\t')
      Width = (static_cast<int32_t>(XSize + TabWidth) / TabWidth) * TabWidth -
              XSize;

    XSize += Width;

    if (Text[x] == '\n') {
      Size.x = max(Size.x, XSize);
      Size.y += Font->GetLineHeight();
      XSize = 0;
    }
  }

  SetHScrollbarParameters(0, max(XSize, Size.x), GetClientRect().Width());
  SetVScrollbarParameters(0, Size.y, GetClientRect().Height());

  if (GetClientRect().Width() >= max(XSize, Size.x)) SetHScrollbarPos(0, true);
  if (GetClientRect().Height() >= Size.y) SetVScrollbarPos(0, true);

  DoSyntaxHighlight();
}

void CWBTextBox::InsertText(int32_t Position, std::string_view txt,
                            int32_t Length, int32_t CursorPosAfter,
                            bool ChangeHistory) {
  if (ChangeHistory) {
    CWBTextBoxHistoryEntry* e = CreateNewHistoryEntry(false, Position, Length);
    e->CursorPos_Before = CursorPos;
    e->SelectionStart_Before = SelectionStart;
    e->SelectionEnd_Before = SelectionEnd;
    e->Data = std::string(txt.substr(0, Length));
  }

  Text.insert(Position, txt);

  SetCursorPos(CursorPosAfter, false);
}

void CWBTextBox::RemoveText(int32_t Position, int32_t Length,
                            int32_t CursorPosAfter, bool ChangeHistory) {
  if (ChangeHistory) {
    CWBTextBoxHistoryEntry* e = CreateNewHistoryEntry(true, Position, Length);
    e->CursorPos_Before = CursorPos;
    e->SelectionStart_Before = SelectionStart;
    e->SelectionEnd_Before = SelectionEnd;
    e->Data = Text.substr(Position, Length);
  }

  Text.erase(Position, Length);

  SetCursorPos(CursorPosAfter, false);
}

void CWBTextBox::SetSelection(int32_t start, int32_t end) {
  SelectionStart = max(0, min(start, end));
  SelectionEnd = min((int32_t)Text.size(), max(start, end));
}

CColor CWBTextBox::GetTextColor(int32_t Index, CColor& DefaultColor) {
  return DefaultColor;
}

bool CWBTextBox::ApplyStyle(std::string_view prop, std::string_view value,
                            const std::vector<std::string>& pseudo) {
  bool ElementTarget = false;

  for (size_t x = 1; x < pseudo.size(); x++) {
    if (pseudo[x] == _T( "selection" )) {
      ElementTarget = true;
      break;
    }
  }

  if (!ElementTarget) return CWBItem::ApplyStyle(prop, value, pseudo);

  bool Handled = false;

  for (size_t x = 1; x < pseudo.size(); x++) {
    if (pseudo[x] == _T( "selection" )) {
      Handled |= Selection.ApplyStyle(this, prop, value, pseudo);
      continue;
    }
  }

  return Handled;
}

CWBItem* CWBTextBox::Factory(CWBItem* Root, CXMLNode& node, CRect& Pos) {
  int32_t Flags = 0;
  if (node.HasAttribute(_T( "singleline" ))) {
    int32_t b = 0;
    node.GetAttributeAsInteger(_T( "singleline" ), &b);
    Flags |= b * WB_TEXTBOX_SINGLELINE;
  }

  if (node.HasAttribute(_T( "password" ))) {
    int32_t b = 0;
    node.GetAttributeAsInteger(_T( "password" ), &b);
    Flags |= b * WB_TEXTBOX_PASSWORD;
  }

  if (node.HasAttribute(_T( "linenumbers" ))) {
    int32_t b = 0;
    node.GetAttributeAsInteger(_T( "linenumbers" ), &b);
    Flags |= b * WB_TEXTBOX_LINENUMS;
  }

  if (node.HasAttribute(_T( "selection" ))) {
    int32_t b = 0;
    node.GetAttributeAsInteger(_T( "selection" ), &b);
    Flags |= (!b) * WB_TEXTBOX_NOSELECTION;
  }

  auto textbox = CWBTextBox::Create(Root, Pos, Flags);
  if (node.HasAttribute(_T( "text" )))
    textbox->SetTextInternal(node.GetAttribute(_T( "text" )), false, true);

  if (Flags & WB_TEXTBOX_SINGLELINE) {
    textbox->EnableHScrollbar(false, false);
    textbox->EnableVScrollbar(false, false);
  }

  return textbox.get();
}

void CWBTextBox::SelectWord(int32_t CharacterInWord) {
  if (CharacterInWord < 0 ||
      CharacterInWord >= static_cast<int32_t>(Text.size()))
    return;

  bool IsWhiteSpace = _istspace(Text[CharacterInWord]);
  bool IsAlNum = _istalnum(Text[CharacterInWord]);
  if (!IsWhiteSpace && !IsAlNum) {
    SetCursorPos(CharacterInWord, false);
    SetCursorPos(CharacterInWord + 1, true);
    return;
  }

  int32_t Start = CharacterInWord;
  int32_t End = CharacterInWord;

  if (IsWhiteSpace) {
    while (Start >= 0 && _istspace(Text[Start]) && Text[Start] != _T('\n') &&
           Text[Start] != _T('\r')) {
      Start--;
    }
    while (End < static_cast<int32_t>(Text.size()) && _istspace(Text[End]) &&
           Text[End] != _T('\n') && Text[End] != _T('\r')) {
      End++;
    }
    End = min((int32_t)Text.size() - 1, End);
  } else {
    while ((Start >= 0 && _istalnum(Text[Start])) || Text[Start] == _T('_')) {
      Start--;
    }
    while ((End < static_cast<int32_t>(Text.size()) && _istalnum(Text[End])) ||
           Text[End] == _T('_')) {
      End++;
    }
    End = min((int32_t)Text.size() - 1, End);
  }

  Start++;
  SetCursorPos(Start, false);
  SetCursorPos(End, true);
}

CWBTextBoxHistoryEntry::CWBTextBoxHistoryEntry() = default;

CWBTextBoxHistoryEntry::~CWBTextBoxHistoryEntry() = default;
