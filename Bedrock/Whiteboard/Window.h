#pragma once
#include <unordered_map>

#include "Application.h"

#define WB_WINDOW_CLOSEABLE 0x00000001
#define WB_WINDOW_MOVEABLE 0x00000002
#define WB_WINDOW_RESIZABLE 0x00000004
#define WB_WINDOW_ALWAYSONTOP 0x00000008
#define WB_WINDOW_TITLE 0x00000010

#define WB_WINDOW_DEFAULT                                           \
  (WB_WINDOW_CLOSEABLE | WB_WINDOW_RESIZABLE | WB_WINDOW_MOVEABLE | \
   WB_WINDOW_TITLE)

#define WB_DRAGMODE_TOP 0x00000001
#define WB_DRAGMODE_BOTTOM 0x00000002
#define WB_DRAGMODE_LEFT 0x00000004
#define WB_DRAGMODE_RIGHT 0x00000008

#define WB_DRAGMASK 0x0000000F

#define WB_DRAGMODE_CLOSEBUTTON 0x00000010
#define WB_DRAGMODE_MOVE 0x00000020

enum class WBWINDOWELEMENT : char {
  WB_WINELEMENT_CLOSE = 0,
  WB_WINELEMENT_MINIMIZE,
  WB_WINELEMENT_INFO,
  WB_WINELEMENT_TITLE,
};

class CWBWindow : public CWBItem {
  int32_t BorderWidth;
  int32_t TitleBarHeight;
  int32_t CornerSelectionSize;

  CSize MinSize;

  std::string WindowTitle;

  std::unordered_map<WBWINDOWELEMENT, CWBCSSPropertyBatch> Elements;

 protected:
  uint32_t Style;
  uint32_t DragMode;

  void OnDraw(CWBDrawAPI* API) override;
  bool MessageProc(CWBMessage& Message) override;

 public:
  CWBWindow(CWBItem* Parent, const CRect& Pos, const TCHAR* txt = _T( "" ),
            uint32_t style = WB_WINDOW_DEFAULT);
  static inline std::shared_ptr<CWBWindow> Create(
      CWBItem* Parent, const CRect& Pos, const TCHAR* txt = _T( "" ),
      uint32_t style = WB_WINDOW_DEFAULT) {
    auto p = std::make_shared<CWBWindow>(Parent, Pos, txt, style);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~CWBWindow() override;

  virtual bool Initialize(CWBItem* Parent, const CRect& Position,
                          const TCHAR* txt = _T( "" ),
                          uint32_t style = WB_WINDOW_DEFAULT);
  bool ApplyStyle(std::string_view prop, std::string_view value,
                  const std::vector<std::string>& pseudo) override;

  std::string GetTitle() const { return WindowTitle; }
  void SetTitle(std::string_view val) { WindowTitle = val; }

  uint32_t GetDragMode();
  static CWBItem* Factory(CWBItem* Root, CXMLNode& node, CRect& Pos);

  CRect GetElementPos(WBWINDOWELEMENT Element);
  uint32_t GetBorderSelectionArea(CPoint& mousepos);

  WB_DECLARE_GUIITEM(_T( "window" ), CWBItem);
};
