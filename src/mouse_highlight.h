#pragma once

#include <string_view>
#include <vector>

#include "src/white_board/draw_api.h"
#include "src/white_board/gui_item.h"

enum class MouseColor {
  red,
  lightred,
  black,
  gray,
  blue,
  lightblue,
  green,
  lightgreen,
  cyan,
  lightcyan,
  magenta,
  lightmagenta,
  brown,
  yellow,
  lightgray,
  white
};
constexpr std::array CGAPalette = {
    CColor{0xffaa0000}, CColor{0xffff5555}, CColor{0xff000000},
    CColor{0xff555555}, CColor{0xff0000aa}, CColor{0xff5555ff},
    CColor{0xff00aa00}, CColor{0xff55ff55}, CColor{0xff00aaaa},
    CColor{0xff55ffff}, CColor{0xffaa00aa}, CColor{0xffff55ff},
    CColor{0xffaa5500}, CColor{0xffffff55}, CColor{0xffaaaaaa},
    CColor{0xffffffff}};

constexpr std::array CGAPaletteNames = {
    "red",   "lightred",   "black",     "gray",      "blue",    "lightblue",
    "green", "lightgreen", "cyan",      "lightcyan", "magenta", "lightmagenta",
    "brown", "yellow",     "lightgray", "white"};

class GW2MouseHighlight : public CWBItem {
  math::CPoint lastpos;
  void OnDraw(CWBDrawAPI* API) override;

 public:
  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  GW2MouseHighlight(CWBItem* Parent, math::CRect Position);
  static inline GW2MouseHighlight* Create(CWBItem* Parent,
                                          math::CRect Position) {
    auto p = std::make_shared<GW2MouseHighlight>(Parent, Position);
    GW2MouseHighlight* r = p.get();
    assert(Parent);
    Parent->AddChild(p);
    return r;
  }
  ~GW2MouseHighlight() override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("mousehighlight", CWBItem);

  math::CPoint lastchangedpos;
  int numSameFrames = 0;
};
