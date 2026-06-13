module;
#include <tchar.h>
#include <windows.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <memory>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "src/base/color.h"
#include "src/base/logger.h"
#include "src/base/timer.h"
#include "src/core2/window_handler.h"

module whiteboard;

import :application;
import :text_box;

using math::Point;
using math::Rect;

namespace gui {

void TextBox::DrawCursor(DrawAPI* API, const Point& p) {
  const ItemState s = GetState();
  if (!(((global_timer.GetTime() - cursor_blink_start_time_) / 500) % 2)) {
    API->DrawRect(
        Rect(p + Point(0, 1), p + Point(1, GetFont(s)->GetLineHeight() - 1)),
        Color{0xffffffff});
  }
}

void TextBox::OnDraw(DrawAPI* API) {
  // background
  DrawBackground(API);
  const ItemState i = GetState();
  Font* Font = GetFont(i);
  const auto text_transform = static_cast<TextTransform>(
      GetDisplayDescriptor().GetValue(i, WB_ITEM_TEXTTRANSFORM));
  const int32_t TabWidth = Font->GetWidth(' ') * 4;

  Point Pos = Point(0, 0);
  const Point Offset =
      -Point(GetHScrollbarPos(), GetVScrollbarPos()) + GetTextStartOffset();

  const bool Focus = InFocus() && !GetChildInFocus();  // speedup

  API->SetCropToClient(this);

  Color color = GetDisplayDescriptor().GetColor(i, WB_ITEM_FONTCOLOR);
  Color BackgroundColor(0x00000000);
  Color SelectionColor = Lerp(
      selection_.display_descriptor_.GetColor(WB_STATE_HOVER,
                                              WB_ITEM_BACKGROUNDCOLOR),
      selection_.display_descriptor_.GetColor(WB_STATE_ACTIVE,
                                              WB_ITEM_BACKGROUNDCOLOR),
      std::min(1.0f, (global_timer.GetTime() - higlight_start_time_) / 300.0f));
  if (!InFocus()) {
    SelectionColor = selection_.display_descriptor_.GetColor(
        WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR);
  }

  for (int32_t x = 0; x < static_cast<int32_t>(text_.size()); x++) {
    if (ColoredText()) {
      // color coding, to be implemented in child classes
      color = GetTextColor(x, color);
    }

    const auto Char =
        flags_ & WB_TEXTBOX_PASSWORD
            ? password_star_
            : Font->ApplyTextTransform(text_.c_str(), text_.c_str() + x,
                                       text_transform);
    int32_t Width = Font->GetWidth(Char);

    if (Char == '\t') {
      Width = (static_cast<int32_t>(Pos.x + TabWidth) / TabWidth) * TabWidth -
              Pos.x;
    }

    // draw selection
    if (!(flags_ & WB_TEXTBOX_NOSELECTION) && x >= selection_start_ &&
        x < selection_end_) {
      const Rect Display =
          Rect(Pos, Point(Pos.x + Width, Pos.y + Font->GetLineHeight())) +
          Offset;
      if (Display.Intersects(GetClientRect())) {
        API->DrawRect(Display, SelectionColor);
      }
    }

    // draw background highlight

    if (GetTextBackground(x, &BackgroundColor)) {
      const Rect Display =
          Rect(Pos, Point(Pos.x + Width, Pos.y + Font->GetLineHeight())) +
          Offset;
      if (Display.Intersects(GetClientRect())) {
        API->DrawRect(Display, BackgroundColor);
      }
    }

    const Point CPos = Pos;

    // special characters
    if (Char == '\n' || Char == '\t') {
      // line feed
      if (Char == '\n' && !(flags_ & WB_TEXTBOX_SINGLELINE)) {
        Pos.x = 0;
        Pos.y += Font->GetLineHeight();
      }
      if (Char == '\t') {
        Pos.x = (static_cast<int32_t>(Pos.x + TabWidth) / TabWidth) * TabWidth;
      }
    } else {
      // draw next character
      const Rect Display =
          Rect(Pos, Point(Pos.x + Width, Pos.y + Font->GetLineHeight())) +
          Offset;
      if (Display.Intersects(GetClientRect())) {
        Font->WriteChar(API, Char, Pos + Offset, color);
      }
      Pos.x += Width;
    }

    // draw cursor if needed
    if (Focus && cursor_pos_ == x) DrawCursor(API, CPos + Offset);
  }

  // cursor at the end of text
  if (Focus && cursor_pos_ == text_.size()) DrawCursor(API, Pos + Offset);

  DrawBorder(API);

  if (GetMouseCaptureItem() && GetMouseCaptureItem() != this) return;
  if (GetMouseItem() == this &&
      ClientToScreen(GetClientRect()).Contains(GetMousePos())) {
    SelectMouseCursor(renderer::MouseCursor::kText);
  }
}

TextBox::TextBox(int32_t flags)
    : GuiType(),
      cursor_blink_start_time_(global_timer.GetTime()),
      flags_(flags) {}

TextBox::~TextBox() = default;

bool TextBox::Initialize(Widget* Parent, const Rect& Position) {
  selection_.display_descriptor_.SetValue(WB_STATE_NORMAL,
                                          WB_ITEM_BACKGROUNDCOLOR, 0);
  selection_.display_descriptor_.SetValue(WB_STATE_ACTIVE,
                                          WB_ITEM_BACKGROUNDCOLOR, 0xff2b537d);
  selection_.display_descriptor_.SetValue(WB_STATE_HOVER,
                                          WB_ITEM_BACKGROUNDCOLOR, 0xff3399ff);

  SetDisplayProperty(WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, 0xff1e1e1e);
  SetDisplayProperty(WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR, 0xff1e1e1e);
  SetDisplayProperty(WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR, 0xff1e1e1e);
  SetDisplayProperty(WB_STATE_DISABLED, WB_ITEM_BACKGROUNDCOLOR, 0xff1e1e1e);
  SetDisplayProperty(WB_STATE_DISABLED_ACTIVE, WB_ITEM_BACKGROUNDCOLOR,
                     0xff1e1e1e);

  if (!Widget::Initialize(Parent, Position)) return false;

  EnableHScrollbar(true, true);
  EnableVScrollbar(true, true);

  return true;
}

void TextBox::SetCursorPos(int32_t pos, bool Selecting) {
  Font* Font = GetFont(GetState());
  const int32_t TabWidth = Font->GetWidth(' ') * 4;

  cursor_blink_start_time_ = global_timer.GetTime();

  if (!Selecting) {
    selection_start_ = selection_end_ = selection_origin_ = cursor_pos_ = pos;
  } else {
    if (selection_start_ == selection_end_)
      selection_origin_ = selection_start_;
    cursor_pos_ = pos;
    selection_start_ = std::min(cursor_pos_, selection_origin_);
    selection_end_ = std::max(cursor_pos_, selection_origin_);
  }

  // adjust scrollbars so cursor is visible

  // determine cursor position in pixels
  Point CPos;
  CPos.y = GetCursorY() * Font->GetLineHeight();
  CPos.x = 0;
  const int32_t Cx = GetCursorX();

  for (int32_t x = 0; x < Cx; x++) {
    const TCHAR Char = text_[cursor_pos_ - Cx + x];
    int32_t Width = Font->GetWidth(Char);

    if (Char == '\t') {
      Width = (static_cast<int32_t>(CPos.x + TabWidth) / TabWidth) * TabWidth -
              CPos.x;
    }

    CPos.x += Width;
  }

  int32_t CurrCharWidth = Font->GetWidth(' ');
  if (cursor_pos_ != text_.size())
    CurrCharWidth = Font->GetWidth(text_[cursor_pos_]);

  OnCursorPosChange(cursor_pos_);

  if (flags_ & WB_TEXTBOX_SINGLELINE) return;

  const Rect VisibleRect =
      GetClientRect() + Point(GetHScrollbarPos(), GetVScrollbarPos());
  if (VisibleRect.Contains(CPos) &&
      VisibleRect.Contains(CPos +
                           Point(CurrCharWidth, Font->GetLineHeight()))) {
    return;  // no need to adjust
  }

  if (CPos.x < VisibleRect.x1) {
    SetHScrollbarPos(GetHScrollbarPos() - (VisibleRect.x1 - CPos.x));
  }
  if (CPos.x + CurrCharWidth > VisibleRect.x2) {
    SetHScrollbarPos(GetHScrollbarPos() +
                     (CPos.x + CurrCharWidth - VisibleRect.x2));
  }

  if (CPos.y < VisibleRect.y1) {
    SetVScrollbarPos(GetVScrollbarPos() - (VisibleRect.y1 - CPos.y));
  }
  if (CPos.y + Font->GetLineHeight() > VisibleRect.y2) {
    SetVScrollbarPos(GetVScrollbarPos() +
                     (CPos.y + Font->GetLineHeight() - VisibleRect.y2));
  }
}

void TextBox::SetCursorPosXpxY(int32_t x, int32_t y, bool Selecting) {
  int32_t p = 0;
  const ItemState i = GetState();
  Font* Font = GetFont(i);
  const auto text_transform = static_cast<TextTransform>(
      GetDisplayDescriptor().GetValue(i, WB_ITEM_TEXTTRANSFORM));
  const int32_t TabWidth = Font->GetWidth(' ') * 4;

  if (!(y < 0 || (y == 0 && x < 0))) {
    int32_t yp = 0;
    while (yp < y) {
      if (p == static_cast<int32_t>(text_.size())) {
        SetCursorPos(p, Selecting);
        return;
      }
      if (text_[p] == '\n') yp++;
      p++;
    }
  }

  int32_t xp = 0;
  while (xp < x) {
    if (p == text_.size() || text_[p] == '\n') {
      // end of line
      SetCursorPos(p, Selecting);
      return;
    }

    const auto Char =
        flags_ & WB_TEXTBOX_PASSWORD
            ? password_star_
            : Font->ApplyTextTransform(text_.c_str(), text_.c_str() + p,
                                       text_transform);
    int32_t Width = Font->GetWidth(Char);

    if (Char == '\t') {
      Width = (static_cast<int32_t>(xp + TabWidth) / TabWidth) * TabWidth - xp;
    }

    xp += Width;
    if (xp > x) break;
    p++;
  }

  SetCursorPos(p, Selecting);
}

void TextBox::RemoveSelectedText() {
  if (selection_start_ != selection_end_) {
    RemoveText(selection_start_, selection_end_ - selection_start_,
               selection_start_);
  }
}

void TextBox::Cut() {
  if (flags_ & WB_TEXTBOX_NOSELECTION) return;

  Copy();
  RemoveSelectedText();
  desired_cursor_pos_x_in_pixels_ = GetCursorXinPixels();
  OnTextChange();
}

void TextBox::Copy() {
  if (flags_ & WB_TEXTBOX_NOSELECTION) return;
  if (flags_ & WB_TEXTBOX_DISABLECOPY) return;

  const int32_t strt = selection_start_;
  const int32_t end = selection_end_;

  if (selection_start_ == selection_end_) {
    // copy whole line...
  }

  if (strt == end) return;

  if (OpenClipboard(GetApplication()->GetHandle())) {
    EmptyClipboard();

    auto winnewline = std::make_unique<char[]>(end - strt + 1);
    memcpy(winnewline.get(), text_.c_str() + strt, (end - strt));
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

    higlight_start_time_ = global_timer.GetTime();
  } else {
    Log_Warn("[gui] Failed to open clipboard");
  }
}

void TextBox::Paste() {
  if (OpenClipboard(GetApplication()->GetHandle())) {
    HANDLE Handle = GetClipboardData(CF_UNICODETEXT);

    auto buffer = static_cast<wchar_t*>(GlobalLock(Handle));
    if (buffer) {
      std::string s;
#ifndef UNICODE
      const int32_t len = wcslen(buffer);
      auto b2 = std::make_unique<char[]>(len + 1);
      memset(b2.get(), 0, len + 1);
      for (int32_t x = 0; x < len; x++) {
        if (buffer[x] >= 0 && buffer[x] <= 255) {
          b2[x] = static_cast<char>(buffer[x]);
        } else {
          b2[x] = '?';
        }
      }
      s = std::string(b2.get());
#else
      // s = CString(buffer); // CString might not be available without MFC/ATL
      // For now, let's assume a similar conversion if UNICODE is defined,
      // or just use the same logic if CString is not available.
      // Given the context, let's keep it simple or use what's likely available.
      int len =
          WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
      if (len > 0) {
        auto b2 = std::make_unique<char[]>(len);
        WideCharToMultiByte(CP_UTF8, 0, buffer, -1, b2.get(), len, NULL, NULL);
        s = std::string(b2.get());
      }
#endif
      s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());

      RemoveSelectedText();
      InsertText(cursor_pos_, s, s.size(), cursor_pos_ + s.size());
      GlobalUnlock(Handle);
      desired_cursor_pos_x_in_pixels_ = GetCursorXinPixels();
    } else {
      Log_Warn("[gui] Failed to retrieve clipboard data ({:x})",
               GetLastError());
    }

    CloseClipboard();
    OnTextChange();
  } else {
    Log_Warn("[gui] Failed to open clipboard");
  }
}

TextBoxHistoryEntry* TextBox::CreateNewHistoryEntry(bool Remove, int Start,
                                                    int Length) {
  for (; history_.size() > history_position_;) {
    history_.pop_back();
  }

  auto entry = std::make_unique<TextBoxHistoryEntry>();
  entry->remove_ = Remove;
  entry->start_position_ = Start;
  entry->data_ = text_.substr(Start, Length);
  history_.emplace_back(std::move(entry));

  history_position_ = history_.size();

  return history_.back().get();
}

void TextBox::Undo() {
  if (history_position_ == 0) {
    return;
  }
  history_position_--;

  auto& e = history_[history_position_];

  if (e->remove_) {
    InsertText(e->start_position_, e->data_,
               static_cast<int32_t>(e->data_.size()), e->cursor_pos_before_,
               false);
  } else {
    RemoveText(e->start_position_, static_cast<int32_t>(e->data_.size()),
               e->cursor_pos_before_, false);
  }

  selection_start_ = e->selection_start_before_;
  selection_end_ = e->selection_end_before_;

  OnTextChange();
}

void TextBox::Redo() {
  if (history_position_ >= history_.size()) {
    return;
  }

  auto& e = history_[history_position_++];

  if (e->remove_) {
    RemoveText(e->start_position_, static_cast<int32_t>(e->data_.size()),
               e->start_position_, false);
  } else {
    InsertText(e->start_position_, e->data_,
               static_cast<int32_t>(e->data_.size()),
               e->start_position_ + e->data_.size(), false);
  }

  selection_start_ = cursor_pos_;
  selection_end_ = cursor_pos_;

  OnTextChange();
}

int32_t TextBox::GetCursorX() {
  int32_t cnt = 0;
  for (int32_t x = cursor_pos_ - 1; x >= 0; x--) {
    if (text_[x] == '\n') return cnt;
    cnt++;
  }
  return cnt;
}

int32_t TextBox::GetCursorY() {
  int32_t cnt = 0;
  for (int32_t x = cursor_pos_ - 1; x >= 0; x--) {
    if (text_[x] == '\n') cnt++;
  }
  return cnt;
}

int32_t TextBox::GetCursorXinPixels() {
  const ItemState i = GetState();
  Font* Font = GetFont(i);
  const auto text_transform = static_cast<TextTransform>(
      GetDisplayDescriptor().GetValue(i, WB_ITEM_TEXTTRANSFORM));
  const int32_t TabWidth = Font->GetWidth(' ') * 4;

  const int32_t c = GetCursorX();
  int32_t Pixels = 0;
  for (int32_t x = cursor_pos_ - c; x < cursor_pos_; x++) {
    const auto Char =
        flags_ & WB_TEXTBOX_PASSWORD
            ? password_star_
            : Font->ApplyTextTransform(text_.c_str(), text_.c_str() + x,
                                       text_transform);
    int32_t Width = Font->GetWidth(Char);

    if (Char == '\t') {
      Width = (static_cast<int32_t>(Pixels + TabWidth) / TabWidth) * TabWidth -
              Pixels;
    }

    Pixels += Width;
  }

  return Pixels;
}

int32_t TextBox::GetLineSize() {
  const int32_t ls = cursor_pos_ - GetCursorX();
  int32_t cnt = 0;
  for (int32_t x = ls; x < static_cast<int32_t>(text_.size()); x++) {
    if (text_[x] == '\n') return cnt;
    cnt++;
  }
  return cnt;
}

int32_t TextBox::GetLineLeadingWhiteSpaceSize() {
  const int32_t ls = cursor_pos_ - GetCursorX();
  int32_t cnt = 0;
  for (int32_t x = ls; x < static_cast<int32_t>(text_.size()); x++) {
    if (text_[x] == '\n' || !std::isspace(static_cast<unsigned char>(text_[x])))
      return cnt;
    cnt++;
  }
  return cnt;
}

Point TextBox::GetTextStartOffset() {
  Point pos(0, 0);
  if (!(flags_ & WB_TEXTBOX_SINGLELINE)) return pos;

  const ItemState i = GetState();
  Font* Font = GetFont(i);
  const auto text_transform = static_cast<TextTransform>(
      GetDisplayDescriptor().GetValue(i, WB_ITEM_TEXTTRANSFORM));
  const Point TextPos = Font->GetTextPosition(
      text_, GetClientRect(), GetCSSProperties().text_align_x_,
      GetCSSProperties().text_align_y_, text_transform);

  if (!IsHScrollbarEnabled()) pos.x = TextPos.x;
  if (!IsVScrollbarEnabled()) pos.y = TextPos.y;

  return pos;
}

int32_t TextBox::GetCursorPosMouse() {
  const ItemState i = GetState();
  Font* Font = GetFont(i);
  const auto text_transform = static_cast<TextTransform>(
      GetDisplayDescriptor().GetValue(i, WB_ITEM_TEXTTRANSFORM));
  const int32_t TabWidth = Font->GetWidth(' ') * 4;

  const Point mp = ScreenToClient(GetMousePos());
  Point Pos = Point(0, 0);
  const Point Offset =
      -Point(GetHScrollbarPos(), GetVScrollbarPos()) + GetTextStartOffset();
  const Rect cr = GetClientRect();

  if (mp.y < Pos.y + Offset.y) return 0;

  for (int32_t x = 0; x < static_cast<int32_t>(text_.size()); x++) {
    if ((Pos.y + Font->GetLineHeight() + Offset.y >= 0 &&
         Pos.y + Offset.y < cr.Height()) ||
        flags_ & WB_TEXTBOX_SINGLELINE) {
      const auto Char =
          flags_ & WB_TEXTBOX_PASSWORD
              ? password_star_
              : Font->ApplyTextTransform(text_.c_str(), text_.c_str() + x,
                                         text_transform);
      int32_t Width = Font->GetWidth(Char);

      if (Char == '\t') {
        Width = (static_cast<int32_t>(Pos.x + TabWidth) / TabWidth) * TabWidth -
                Pos.x;
      }

      if ((Pos.y + Offset.y <= mp.y &&
           mp.y < Pos.y + Font->GetLineHeight() + Offset.y) ||
          flags_ & WB_TEXTBOX_SINGLELINE) {
        // we're in the correct line
        if (mp.x < Pos.x + Offset.x) return x;  // for line starts
        if (Pos.x + Offset.x <= mp.x && mp.x < Pos.x + Width + Offset.x) {
          // we're in the correct column
          return x;
        }
      }

      Pos.x += Width;
    }

    if (text_[x] == '\n') {
      if (Pos.y + Offset.y <= mp.y &&
          mp.y < Pos.y + Font->GetLineHeight() + Offset.y) {
        return x;
      }
      Pos.x = 0;
      Pos.y += Font->GetLineHeight();
    }
  }
  return text_.size();
}

bool TextBox::MessageProc(const Message& message) {
  switch (message.Get()) {
    default:
      break;
    case WBM_FOCUSGAINED:
      if (message.GetTarget() == GetGuid()) {
        original_text_ = text_;
        if (GetChildInFocus()) GetChildInFocus()->ClearFocus();
        return true;
      }
      break;

    case WBM_FOCUSLOST:
      if (message.GetTarget() == GetGuid()) {
        if (GetParent() && GetParent()->GetChildInFocus() == this) {
          GetParent()->SetChildInFocus(nullptr);
        }
        return true;
      }
      break;

    case WBM_REPOSITION: {
      const bool b = Widget::MessageProc(message);
      int32_t mi = 0, ma = 0, vi = 0;
      GetHScrollbarParameters(&mi, &ma, &vi);
      SetHScrollbarParameters(mi, ma, GetClientRect().Width());
      GetVScrollbarParameters(&mi, &ma, &vi);
      SetVScrollbarParameters(mi, ma, GetClientRect().Height());
      SetCursorPos(cursor_pos_, false);
      return b;
    } break;

    case WBM_LEFTBUTTONDOWN:
      if (GetMouseItem() != this) break;
      if (Widget::MessageProc(message)) return true;
      SetCapture();
      SetCursorPos(GetCursorPosMouse(), GetApplication()->GetShiftState());
      desired_cursor_pos_x_in_pixels_ = GetCursorXinPixels();
      return true;

    case WBM_LEFTBUTTONDBLCLK:
      if (GetMouseItem() != this) break;
      if (Widget::MessageProc(message)) return true;
      if (GetApplication()->GetShiftState()) return true;
      SelectWord(GetCursorPosMouse());
      return true;

    case WBM_MOUSEMOVE:
      if (Widget::MessageProc(message)) return true;
      if (GetMouseCaptureItem() == this) {
        SetCursorPos(GetCursorPosMouse(), true);
        desired_cursor_pos_x_in_pixels_ = GetCursorXinPixels();
      }
      return true;

    case WBM_MOUSEWHEEL: {
      Font* Font = GetFont(GetState());
      SetVScrollbarPos(
          GetVScrollbarPos() - message.Data() * 3 * Font->GetLineHeight(),
          true);
    }
      return true;

    case WBM_LEFTBUTTONUP:
      if (Widget::MessageProc(message)) return true;
      GetApplication()->ReleaseCapture();
      return true;

    case WBM_KEYDOWN: {
      if (!InFocus() || GetChildInFocus()) break;
      if (message.KeyboardState() & WB_KBSTATE_ALT &&
          message.KeyboardState() & WB_KBSTATE_CTRL) {
        break;  // altgr
      }

      Font* Font = GetFont(GetState());

      // handle cursor movement keys
      switch (message.Key()) {
        case VK_LEFT:
          SetCursorPos(std::max(0, cursor_pos_ - 1),
                       message.KeyboardState() & WB_KBSTATE_SHIFT);
          desired_cursor_pos_x_in_pixels_ = GetCursorXinPixels();
          return true;

        case VK_RIGHT:
          SetCursorPos(
              std::min(static_cast<int32_t>(text_.size()), cursor_pos_ + 1),
              message.KeyboardState() & WB_KBSTATE_SHIFT);
          desired_cursor_pos_x_in_pixels_ = GetCursorXinPixels();
          return true;

        case VK_UP:
          SetCursorPosXpxY(desired_cursor_pos_x_in_pixels_, GetCursorY() - 1,
                           message.KeyboardState() & WB_KBSTATE_SHIFT);
          return true;

        case VK_DOWN:
          SetCursorPosXpxY(desired_cursor_pos_x_in_pixels_, GetCursorY() + 1,
                           message.KeyboardState() & WB_KBSTATE_SHIFT);
          return true;

        case VK_PRIOR:  // page up
          SetVScrollbarPos(GetVScrollbarPos() -
                               (static_cast<int32_t>(GetClientRect().Height() /
                                                     Font->GetLineHeight())) *
                                   Font->GetLineHeight(),
                           true);
          SetCursorPosXpxY(desired_cursor_pos_x_in_pixels_,
                           GetVScrollbarPos()
                               ? GetCursorY() - GetClientRect().Height() /
                                                    Font->GetLineHeight()
                               : 0,
                           message.KeyboardState() & WB_KBSTATE_SHIFT);
          return true;

        case VK_NEXT:  // page down
          SetVScrollbarPos(GetVScrollbarPos() +
                               (static_cast<int32_t>(GetClientRect().Height() /
                                                     Font->GetLineHeight())) *
                                   Font->GetLineHeight(),
                           true);
          SetCursorPosXpxY(
              desired_cursor_pos_x_in_pixels_,
              GetCursorY() + GetClientRect().Height() / Font->GetLineHeight(),
              message.KeyboardState() & WB_KBSTATE_SHIFT);
          return true;

        case VK_HOME:
          SetHScrollbarPos(0);
          if (message.KeyboardState() & WB_KBSTATE_CTRL) {
            SetCursorPos(0, message.KeyboardState() & WB_KBSTATE_SHIFT);
            desired_cursor_pos_x_in_pixels_ = GetCursorXinPixels();
            return true;
          }

          // skip to the start of the line
          if (GetCursorX() == GetLineLeadingWhiteSpaceSize()) {
            SetCursorPos(cursor_pos_ - GetCursorX(),
                         message.KeyboardState() & WB_KBSTATE_SHIFT);
          } else {
            SetCursorPos(
                cursor_pos_ - GetCursorX() + GetLineLeadingWhiteSpaceSize(),
                message.KeyboardState() & WB_KBSTATE_SHIFT);
          }
          desired_cursor_pos_x_in_pixels_ = GetCursorXinPixels();
          return true;

        case VK_END:
          if (message.KeyboardState() & WB_KBSTATE_CTRL) {
            SetCursorPos(static_cast<int32_t>(text_.size()),
                         message.KeyboardState() & WB_KBSTATE_SHIFT);
            desired_cursor_pos_x_in_pixels_ = GetCursorXinPixels();
            return true;
          }

          // skip to the end of the line
          for (; cursor_pos_ < static_cast<int32_t>(text_.size());
               cursor_pos_++) {
            if (text_[cursor_pos_] == '\n') {
              SetCursorPos(cursor_pos_,
                           message.KeyboardState() & WB_KBSTATE_SHIFT);
              desired_cursor_pos_x_in_pixels_ = GetCursorXinPixels();
              break;
            }
          }
          SetCursorPos(cursor_pos_, message.KeyboardState() & WB_KBSTATE_SHIFT);
          desired_cursor_pos_x_in_pixels_ = GetCursorXinPixels();
          return true;

        case VK_RETURN:
          if (flags_ & WB_TEXTBOX_SINGLELINE) {
            GetApplication()->Send(
                Message(GetApplication(), WBM_COMMAND, GetGuid()));
            ClearFocus();
            return true;
          }
          break;

        case VK_DELETE:
          if (message.KeyboardState() & WB_KBSTATE_SHIFT) {
            Cut();
          } else {
            if (cursor_pos_ < 0 ||
                cursor_pos_ >= static_cast<int32_t>(text_.size())) {
              RemoveSelectedText();
              OnTextChange();
              return true;
            }

            if (selection_start_ == selection_end_) {
              RemoveText(cursor_pos_, 1, cursor_pos_);
            } else {
              RemoveSelectedText();
            }
            desired_cursor_pos_x_in_pixels_ = GetCursorXinPixels();
            OnTextChange();
          }
          return true;

        case VK_ESCAPE:
          if ((flags_ & WB_TEXTBOX_SINGLELINE)) {
            SetTextInternal(original_text_, false, false);
            GetApplication()->Send(
                Message(GetApplication(), WBM_COMMAND, GetGuid()));
            ClearFocus();
            return true;
          }

          return true;

        case VK_INSERT:
          if (message.KeyboardState() & WB_KBSTATE_CTRL) {
            Copy();
          } else if (message.KeyboardState() & WB_KBSTATE_SHIFT) {
            Paste();
          }
          return true;

        case 'X':
          if (message.KeyboardState() & WB_KBSTATE_CTRL) Cut();
          return true;

        case 'C':
          if (message.KeyboardState() & WB_KBSTATE_CTRL) Copy();
          return true;

        case 'V':
          if (message.KeyboardState() & WB_KBSTATE_CTRL) Paste();
          return true;

        case 'A':  // select all
          if (message.KeyboardState() & WB_KBSTATE_CTRL) {
            SetCursorPos(0, false);
            SetCursorPos(static_cast<int32_t>(text_.size()), true);
          }
          return true;

        case 'Z':
          if (message.KeyboardState() & WB_KBSTATE_CTRL) Undo();
          return true;

        case 'Y':
          if (message.KeyboardState() & WB_KBSTATE_CTRL) Redo();
          return true;
      }

      return false;
    }
    case WBM_CHAR:
      if (!InFocus() || GetChildInFocus()) break;
      if (!(message.KeyboardState() & WB_KBSTATE_CTRL &&
            message.KeyboardState() & WB_KBSTATE_ALT)) {  // altgr
        if (message.KeyboardState() & (WB_KBSTATE_ALT | WB_KBSTATE_CTRL)) {
          return true;  // these should be handled by the keydown messages
        }
      }

      if (message.Key() == VK_BACK) {
        if (selection_start_ == selection_end_) {
          if (!cursor_pos_) return true;
          RemoveText(cursor_pos_ - 1, 1, cursor_pos_ - 1);
        } else {
          RemoveSelectedText();
        }
        OnTextChange();

        return true;
      }

      if (message.Key() == VK_ESCAPE) {
        selection_start_ = selection_end_ = selection_origin_ = cursor_pos_;
        return true;  // shouldn't produce text
      }

      if ((flags_ & WB_TEXTBOX_SINGLELINE) && message.Key() == VK_RETURN) {
        return true;  // already handled in the keydown message
      }

      // translate VK_RETURN to '\n':

      int32_t Key = message.Key();

      if (message.Key() == VK_RETURN) Key = '\n';

      // insert character
      RemoveSelectedText();
      InsertText(cursor_pos_, reinterpret_cast<TCHAR*>(&Key), 1,
                 cursor_pos_ + 1);

      PostCharInsertion(cursor_pos_, message.Key());

      desired_cursor_pos_x_in_pixels_ = GetCursorXinPixels();
      OnTextChange();

      return true;  // this captures all character inputs which should be fine
  }

  return Widget::MessageProc(message);
}

void TextBox::SetText(std::string_view val, bool EnableUndo) {
  SetTextInternal(val, EnableUndo, true);
}

void TextBox::SetTextInternal(std::string_view val, bool EnableUndo,
                              bool nonHumanInteraction) {
  if (!EnableUndo) {
    original_text_ = text_ = val;
    text_.erase(std::remove(text_.begin(), text_.end(), '\r'), text_.end());
    history_.clear();
    history_position_ = 0;
  } else {
    selection_start_ = 0;
    selection_end_ = static_cast<int32_t>(text_.size());
    RemoveSelectedText();
    std::string s(val);
    s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
    InsertText(0, s, static_cast<int32_t>(s.size()),
               static_cast<int32_t>(s.size()));
  }

  SetCursorPos(static_cast<int32_t>(text_.size()), false);
  OnTextChange(nonHumanInteraction != 0);
}

void TextBox::OnTextChange(bool nonHumanInteraction /* = false*/) {
  // calculate new scrollbar data
  const ItemState i = GetState();
  Font* Font = GetFont(i);
  const auto text_transform = static_cast<TextTransform>(
      GetDisplayDescriptor().GetValue(i, WB_ITEM_TEXTTRANSFORM));
  const int32_t TabWidth = Font->GetWidth(' ') * 4;

  GetApplication()->Send(
      Message(GetApplication(), WBM_TEXTCHANGED, GetGuid(),
              GetFocusItem() == this && !nonHumanInteraction));

  Point Size = Point(0, Font->GetLineHeight());
  int32_t XSize = 0;

  for (int32_t x = 0; x < static_cast<int32_t>(text_.size()); x++) {
    const auto Char =
        flags_ & WB_TEXTBOX_PASSWORD
            ? password_star_
            : Font->ApplyTextTransform(text_.c_str(), text_.c_str() + x,
                                       text_transform);
    int32_t Width = Font->GetWidth(Char);

    if (Char == '\t') {
      Width = (static_cast<int32_t>(XSize + TabWidth) / TabWidth) * TabWidth -
              XSize;
    }

    XSize += Width;

    if (text_[x] == '\n') {
      Size.x = std::max(Size.x, XSize);
      Size.y += Font->GetLineHeight();
      XSize = 0;
    }
  }

  SetHScrollbarParameters(0, std::max(XSize, Size.x), GetClientRect().Width());
  SetVScrollbarParameters(0, Size.y, GetClientRect().Height());

  if (GetClientRect().Width() >= std::max(XSize, Size.x)) {
    SetHScrollbarPos(0, true);
  }
  if (GetClientRect().Height() >= Size.y) SetVScrollbarPos(0, true);

  DoSyntaxHighlight();
}

void TextBox::InsertText(int32_t Position, std::string_view txt, int32_t Length,
                         int32_t CursorPosAfter, bool ChangeHistory) {
  if (ChangeHistory) {
    TextBoxHistoryEntry* e = CreateNewHistoryEntry(false, Position, Length);
    e->cursor_pos_before_ = cursor_pos_;
    e->selection_start_before_ = selection_start_;
    e->selection_end_before_ = selection_end_;
    e->data_ = std::string(txt.substr(0, Length));
  }

  text_.insert(Position, txt);

  SetCursorPos(CursorPosAfter, false);
}

void TextBox::RemoveText(int32_t Position, int32_t Length,
                         int32_t CursorPosAfter, bool ChangeHistory) {
  if (ChangeHistory) {
    TextBoxHistoryEntry* e = CreateNewHistoryEntry(true, Position, Length);
    e->cursor_pos_before_ = cursor_pos_;
    e->selection_start_before_ = selection_start_;
    e->selection_end_before_ = selection_end_;
    e->data_ = text_.substr(Position, Length);
  }

  text_.erase(Position, Length);

  SetCursorPos(CursorPosAfter, false);
}

void TextBox::SetSelection(int32_t start, int32_t end) {
  selection_start_ = std::max(0, std::min(start, end));
  selection_end_ =
      std::min(static_cast<int32_t>(text_.size()), std::max(start, end));
}

Color TextBox::GetTextColor(int32_t Index, const Color& DefaultColor) {
  return DefaultColor;
}

bool TextBox::ApplyStyle(std::string_view prop, std::string_view value,
                         const std::vector<std::string>& pseudo) {
  bool ElementTarget = false;

  for (size_t x = 1; x < pseudo.size(); x++) {
    if (pseudo[x] == "selection") {
      ElementTarget = true;
      break;
    }
  }

  if (!ElementTarget) return Widget::ApplyStyle(prop, value, pseudo);

  bool Handled = false;

  for (size_t x = 1; x < pseudo.size(); x++) {
    if (pseudo[x] == "selection") {
      Handled |= selection_.ApplyStyle(this, prop, value, pseudo);
      continue;
    }
  }

  return Handled;
}

Widget* TextBox::Factory(Widget* Root, const XMLNode& node, const Rect& Pos) {
  int32_t Flags = 0;
  if (node.HasAttribute("singleline")) {
    int32_t b = 0;
    node.GetAttributeAsInteger("singleline", &b);
    Flags |= b * WB_TEXTBOX_SINGLELINE;
  }

  if (node.HasAttribute("password")) {
    int32_t b = 0;
    node.GetAttributeAsInteger("password", &b);
    Flags |= b * WB_TEXTBOX_PASSWORD;
  }

  if (node.HasAttribute("linenumbers")) {
    int32_t b = 0;
    node.GetAttributeAsInteger("linenumbers", &b);
    Flags |= b * WB_TEXTBOX_LINENUMS;
  }

  if (node.HasAttribute("selection")) {
    int32_t b = 0;
    node.GetAttributeAsInteger("selection", &b);
    Flags |= (!b) * WB_TEXTBOX_NOSELECTION;
  }

  auto textbox = TextBox::Create(Root, Pos, Flags);
  if (node.HasAttribute("text")) {
    textbox->SetTextInternal(node.GetAttribute("text"), false, true);
  }

  if (Flags & WB_TEXTBOX_SINGLELINE) {
    textbox->EnableHScrollbar(false, false);
    textbox->EnableVScrollbar(false, false);
  }

  return textbox;
}

void TextBox::SelectWord(int32_t CharacterInWord) {
  if (CharacterInWord < 0 ||
      CharacterInWord >= static_cast<int32_t>(text_.size())) {
    return;
  }

  const bool IsWhiteSpace =
      std::isspace(static_cast<unsigned char>(text_[CharacterInWord]));
  const bool IsAlNum =
      std::isalnum(static_cast<unsigned char>(text_[CharacterInWord]));
  if (!IsWhiteSpace && !IsAlNum) {
    SetCursorPos(CharacterInWord, false);
    SetCursorPos(CharacterInWord + 1, true);
    return;
  }

  int32_t Start = CharacterInWord;
  int32_t End = CharacterInWord;

  if (IsWhiteSpace) {
    while (Start >= 0 &&
           std::isspace(static_cast<unsigned char>(text_[Start])) &&
           text_[Start] != '\n' && text_[Start] != '\r') {
      Start--;
    }
    while (End < static_cast<int32_t>(text_.size()) &&
           std::isspace(static_cast<unsigned char>(text_[End])) &&
           text_[End] != '\n' && text_[End] != '\r') {
      End++;
    }
    End = std::min(static_cast<int32_t>(text_.size()) - 1, End);
  } else {
    while ((Start >= 0 &&
            std::isalnum(static_cast<unsigned char>(text_[Start]))) ||
           text_[Start] == '_') {
      Start--;
    }
    while ((End < static_cast<int32_t>(text_.size()) &&
            std::isalnum(static_cast<unsigned char>(text_[End]))) ||
           text_[End] == '_') {
      End++;
    }
    End = std::min(static_cast<int32_t>(text_.size()) - 1, End);
  }

  Start++;
  SetCursorPos(Start, false);
  SetCursorPos(End, true);
}

TextBoxHistoryEntry::TextBoxHistoryEntry() = default;

TextBoxHistoryEntry::~TextBoxHistoryEntry() = default;

}  // namespace gui
