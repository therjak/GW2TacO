#pragma once

#include <string_view>
#include <vector>

#include "Bedrock/Whiteboard/DrawAPI.h"
#include "Bedrock/Whiteboard/GuiItem.h"

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
  CPoint lastpos;
  void OnDraw(CWBDrawAPI* API) override;

 public:
  bool IsMouseTransparent(const CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  GW2MouseHighlight(CWBItem* Parent, CRect Position);
  static inline std::shared_ptr<GW2MouseHighlight> Create(CWBItem* Parent,
                                                          CRect Position) {
    auto p = std::make_shared<GW2MouseHighlight>(Parent, Position);
    p->SelfRef = p;
    if (Parent) {
      Parent->AddChild(p);
    }
    return p;
  }
  ~GW2MouseHighlight() override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node, CRect& Pos);
  WB_DECLARE_GUIITEM(_T( "mousehighlight" ), CWBItem);

  CPoint lastchangedpos;
  int numSameFrames = 0;
};
