#pragma once

#include <string_view>
#include <string>

#include "Application.h"

#define WB_TEXTBOX_SINGLELINE 0x0000001
#define WB_TEXTBOX_PASSWORD 0x0000002
#define WB_TEXTBOX_NOSELECTION 0x0000004
#define WB_TEXTBOX_LINENUMS 0x0000008
#define WB_TEXTBOX_DISABLECOPY 0x0000010

class CWBTextBoxHistoryEntry {
  friend class CWBTextBox;

  TBOOL Remove;
  int32_t StartPosition;
  std::string Data;

  int32_t CursorPos_Before, SelectionStart_Before, SelectionEnd_Before;

 public:
  CWBTextBoxHistoryEntry();
  virtual ~CWBTextBoxHistoryEntry();
};

class CWBTextBox : public CWBItem {
  CArray<CWBTextBoxHistoryEntry *> History;
  int32_t HistoryPosition;

  int32_t HiglightStartTime;
  int32_t CursorBlinkStartTime;

  int32_t Flags;
  const char PasswordStar = '*';  // character to display instead of characters

  int32_t CursorPos;
  int32_t DesiredCursorPosXinPixels;  // used when moving up and down

  int32_t SelectionStart, SelectionEnd, SelectionOrigin;

  CWBCSSPropertyBatch Selection;

  INLINE void DrawCursor(CWBDrawAPI *API, CPoint &p);
  void SetCursorPosXpxY(int32_t x, int32_t y, TBOOL Selecting);
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

  CPoint GetTextStartOffset();

  virtual void DoSyntaxHighlight(){};
  virtual void OnTextChange(bool nonHumanInteraction = false);
  CWBTextBoxHistoryEntry *CreateNewHistoryEntry(bool Remove, int Start,
                                                int Length);

  virtual CColor GetTextColor(int32_t Index, CColor &DefaultColor);
  virtual TBOOL GetTextBackground(int32_t Index, CColor &Result) {
    return false;
  }
  virtual TBOOL ColoredText() { return false; }
  virtual void OnCursorPosChange(int32_t CursorPos) {}
  virtual void PostCharInsertion(int32_t CursorPos, int32_t Key){};

  virtual void SelectWord(int32_t CharacterInWord);

  void SetTextInternal(std::string_view val, TBOOL EnableUndo = false,
                       TBOOL nonHumanInteraction = false);

 protected:
  std::string Text;

  std::string OriginalText;  // for escape cancel

  virtual void OnDraw(CWBDrawAPI *API);
  virtual TBOOL MessageProc(CWBMessage &Message);
  void InsertText(int32_t Position, std::string_view Text, int32_t Length,
                  int32_t CursorPosAfter, TBOOL ChangeHistory = true);
  void RemoveText(int32_t Position, int32_t Length, int32_t CursorPosAfter,
                  TBOOL ChangeHistory = true);

 public:
  CWBTextBox();
  CWBTextBox(CWBItem *Parent, const CRect &Pos,
             int32_t flags = WB_TEXTBOX_SINGLELINE,
             std::string_view txt = _T( "" ));
  virtual ~CWBTextBox();

  virtual TBOOL Initialize(CWBItem *Parent, const CRect &Position,
                           int32_t flags = WB_TEXTBOX_SINGLELINE,
                           std::string_view txt = _T( "" ));
  virtual TBOOL ApplyStyle(std::string_view prop, std::string_view value,
                           const std::vector<std::string> &pseudo);

  std::string GetText() const {
    return Text;
  }  // text returned here will always be in unix newline format ('\n' instead
     // of the windows '\r\n')
  void SetText(std::string_view val, TBOOL EnableUndo = false);

  static CWBItem *Factory(CWBItem *Root, CXMLNode &node, CRect &Pos);
  WB_DECLARE_GUIITEM(_T( "textbox" ), CWBItem);

  virtual void SetSelection(int32_t start, int32_t end);
  void SetCursorPos(int32_t pos, TBOOL Selecting);
};
