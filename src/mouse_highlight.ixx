module;
#include <array>
#include <cassert>
#include <memory>
#include <string_view>
#include <vector>

#include "src/base/color.h"

export module taco.mouse_highlight;

import whiteboard;
import xml;
import math;

export enum class MouseColor {
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

export constexpr std::array CGAPalette = {
    CColor{0xffaa0000}, CColor{0xffff5555}, CColor{0xff000000},
    CColor{0xff555555}, CColor{0xff0000aa}, CColor{0xff5555ff},
    CColor{0xff00aa00}, CColor{0xff55ff55}, CColor{0xff00aaaa},
    CColor{0xff55ffff}, CColor{0xffaa00aa}, CColor{0xffff55ff},
    CColor{0xffaa5500}, CColor{0xffffff55}, CColor{0xffaaaaaa},
    CColor{0xffffffff}};

export constexpr std::array CGAPaletteNames = {
    "red",   "lightred",   "black",     "gray",      "blue",    "lightblue",
    "green", "lightgreen", "cyan",      "lightcyan", "magenta", "lightmagenta",
    "brown", "yellow",     "lightgray", "white"};

export class GW2MouseHighlight
    : public gui::CWBGuiType<"mousehighlight", gui::CWBItem> {
 public:
  GW2MouseHighlight();
  static inline GW2MouseHighlight* Create(gui::CWBItem* Parent,
                                          math::Rect Position) {
    auto p = std::make_unique<GW2MouseHighlight>();
    p->Initialize(Parent, Position);
    GW2MouseHighlight* r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~GW2MouseHighlight() override;

  static gui::CWBItem* Factory(gui::CWBItem* Root, const CXMLNode& node,
                               math::Rect& Pos);

  bool IsMouseTransparent(const math::Point& ClientSpacePoint,
                          gui::WBMESSAGE MessageType) override;

 private:
  void OnDraw(gui::CWBDrawAPI* API) override;

  math::Point last_pos;
  math::Point lastchangedpos;
  int numSameFrames = 0;
};
