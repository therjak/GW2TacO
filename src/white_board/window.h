#pragma once
#include <unordered_map>

#include "src/white_board/application.h"

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
  int32_t BorderWidth{3};
  int32_t TitleBarHeight;
  int32_t CornerSelectionSize{15};

  math::CSize MinSize;

  std::string WindowTitle;

  std::unordered_map<WBWINDOWELEMENT, CWBCSSPropertyBatch> Elements;

 protected:
  uint32_t Style;
  uint32_t DragMode{0};

  void OnDraw(CWBDrawAPI* API) override;
  bool MessageProc(const CWBMessage& Message) override;

 public:
  CWBWindow(CWBItem* Parent, const math::CRect& Pos, const TCHAR* txt = "",
            uint32_t style = WB_WINDOW_DEFAULT);
  static inline CWBWindow* Create(CWBItem* Parent, const math::CRect& Pos,
                                  const TCHAR* txt = "",
                                  uint32_t style = WB_WINDOW_DEFAULT) {
    auto p = std::make_unique<CWBWindow>(Parent, Pos, txt, style);
    CWBWindow* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~CWBWindow() override;

  bool Initialize(CWBItem* Parent, const math::CRect& Position) override;
  bool ApplyStyle(std::string_view prop, std::string_view value,
                  const std::vector<std::string>& pseudo) override;

  [[nodiscard]] std::string GetTitle() const { return WindowTitle; }
  void SetTitle(std::string_view val) { WindowTitle = val; }

  uint32_t GetDragMode();
  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);

  math::CRect GetElementPos(WBWINDOWELEMENT Element);
  uint32_t GetBorderSelectionArea(const math::CPoint& mousepos);

  WB_DECLARE_GUIITEM("window", CWBItem);
};
