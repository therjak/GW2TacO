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

class CWBTextBox;

class CWBTextBoxHistoryEntry {
  friend class CWBTextBox;

 public:
  CWBTextBoxHistoryEntry();
  virtual ~CWBTextBoxHistoryEntry();

 private:
  bool Remove = false;
  int32_t StartPosition = 0;
  std::string Data;

  int32_t CursorPos_Before = 0;
  int32_t SelectionStart_Before = 0;
  int32_t SelectionEnd_Before = 0;
};

class CWBTextBox : public CWBGuiType<"textbox", CWBItem> {
 public:
  CWBTextBox(int32_t flags);
  static inline CWBTextBox* Create(CWBItem* Parent, const math::Rect& Pos,
                                   int32_t flags = WB_TEXTBOX_SINGLELINE,
                                   std::string_view txt = "") {
    auto p = std::make_unique<CWBTextBox>(flags);
    p->Initialize(Parent, Pos);
    p->SetTextInternal(txt, false, true);
    CWBTextBox* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~CWBTextBox() override;

  bool Initialize(CWBItem* Parent, const math::Rect& Position) override;
  bool ApplyStyle(std::string_view prop, std::string_view value,
                  const std::vector<std::string>& pseudo) override;

  [[nodiscard]] std::string GetText() const { return Text; }
  void SetText(std::string_view val, bool EnableUndo = false);

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          const math::Rect& Pos);

  virtual void SetSelection(int32_t start, int32_t end);
  void SetCursorPos(int32_t pos, bool Selecting);

 protected:
  void OnDraw(CWBDrawAPI* API) override;
  bool MessageProc(const CWBMessage& Message) override;
  void InsertText(int32_t Position, std::string_view Text, int32_t Length,
                  int32_t CursorPosAfter, bool ChangeHistory = true);
  void RemoveText(int32_t Position, int32_t Length, int32_t CursorPosAfter,
                  bool ChangeHistory = true);

  std::string Text;
  std::string OriginalText;  // for escape cancel

 private:
  void DrawCursor(CWBDrawAPI* API, const math::Point& p);
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
  CWBTextBoxHistoryEntry* CreateNewHistoryEntry(bool Remove, int Start,
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

  std::vector<std::unique_ptr<CWBTextBoxHistoryEntry>> History;
  uint32_t HistoryPosition = 0;

  int32_t HiglightStartTime = 0;
  int32_t CursorBlinkStartTime = 0;

  int32_t Flags = 0;
  const char PasswordStar = '*';  // character to display instead of characters

  int32_t CursorPos = 0;
  int32_t DesiredCursorPosXinPixels = 0;  // used when moving up and down

  int32_t SelectionStart = 0, SelectionEnd = 0, SelectionOrigin = 0;

  CWBCSSPropertyBatch Selection;
};

}  // namespace gui
