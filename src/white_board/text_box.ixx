module;
#include <tchar.h>

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "src/base/color.h"

export module whiteboard:text_box;

import :application;
import :css_item;
import :gui_item;

export namespace gui {

constexpr uint32_t WB_TEXTBOX_SINGLELINE = 0x0000001;
constexpr uint32_t WB_TEXTBOX_PASSWORD = 0x0000002;
constexpr uint32_t WB_TEXTBOX_NOSELECTION = 0x0000004;
constexpr uint32_t WB_TEXTBOX_LINENUMS = 0x0000008;
constexpr uint32_t WB_TEXTBOX_DISABLECOPY = 0x0000010;

class TextBox;

class TextBoxHistoryEntry {
  friend class TextBox;

 public:
  TextBoxHistoryEntry();
  virtual ~TextBoxHistoryEntry();

 private:
  bool remove_ = false;
  int32_t start_position_ = 0;
  std::string data_;

  int32_t cursor_pos_before_ = 0;
  int32_t selection_start_before_ = 0;
  int32_t selection_end_before_ = 0;
};

class TextBox : public GuiType<"textbox", Widget> {
 public:
  TextBox(int32_t flags);
  static inline TextBox* Create(Widget* Parent, const math::Rect& Pos,
                                int32_t flags = WB_TEXTBOX_SINGLELINE,
                                std::string_view txt = "") {
    auto p = std::make_unique<TextBox>(flags);
    p->Initialize(Parent, Pos);
    p->SetTextInternal(txt, false, true);
    TextBox* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~TextBox() override;

  bool Initialize(Widget* Parent, const math::Rect& Position) override;
  bool ApplyStyle(std::string_view prop, std::string_view value,
                  const std::vector<std::string>& pseudo) override;

  [[nodiscard]] std::string GetText() const { return text_; }
  void SetText(std::string_view val, bool EnableUndo = false);

  static Widget* Factory(Widget* Root, const XMLNode& node,
                         const math::Rect& Pos);

  virtual void SetSelection(int32_t start, int32_t end);
  void SetCursorPos(int32_t pos, bool Selecting);

 protected:
  void OnDraw(DrawAPI* API) override;
  bool MessageProc(const Message& message) override;
  void InsertText(int32_t Position, std::string_view Text, int32_t Length,
                  int32_t CursorPosAfter, bool ChangeHistory = true);
  void RemoveText(int32_t Position, int32_t Length, int32_t CursorPosAfter,
                  bool ChangeHistory = true);

  std::string text_;
  std::string original_text_;  // for escape cancel

 private:
  void DrawCursor(DrawAPI* API, const math::Point& p);
  void SetCursorPosXpxY(int32_t x, int32_t y, bool Selecting);
  void RemoveSelectedText();
  void Copy();
  void Cut();
  void Paste();
  void Undo();
  void Redo();
  int32_t GetCursorX();
  int32_t GetCursorXinPixels();
  int32_t GetCursorY();
  int32_t GetLineSize();
  int32_t GetCursorPosMouse();
  int32_t GetLineLeadingWhiteSpaceSize();

  math::Point GetTextStartOffset();

  virtual void DoSyntaxHighlight(){};
  virtual void OnTextChange(bool nonHumanInteraction = false);
  TextBoxHistoryEntry* CreateNewHistoryEntry(bool Remove, int Start,
                                             int Length);

  virtual CColor GetTextColor(int32_t Index, const CColor& DefaultColor);
  virtual bool GetTextBackground(int32_t Index, CColor* Result) {
    return false;
  }
  virtual bool ColoredText() { return false; }
  virtual void OnCursorPosChange(int32_t CursorPos) {}
  virtual void PostCharInsertion(int32_t CursorPos, int32_t Key){};

  virtual void SelectWord(int32_t CharacterInWord);

  void SetTextInternal(std::string_view val, bool EnableUndo = false,
                       bool nonHumanInteraction = false);

  std::vector<std::unique_ptr<TextBoxHistoryEntry>> history_;
  uint32_t history_position_ = 0;

  int32_t higlight_start_time_ = 0;
  int32_t cursor_blink_start_time_ = 0;

  int32_t flags_ = 0;
  // character to display instead of characters
  const char password_star_ = '*';

  int32_t cursor_pos_ = 0;
  int32_t desired_cursor_pos_x_in_pixels_ = 0;  // used when moving up and down

  int32_t selection_start_ = 0, selection_end_ = 0, selection_origin_ = 0;

  CssPropertyBatch selection_;
};

}  // namespace gui
