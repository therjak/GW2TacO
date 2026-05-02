module;
#include <array>
#include <cassert>
#include <memory>
#include <string_view>
#include <utility>
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
  static inline GW2MouseHighlight* Create(gui::CWBItem* parent,
                                          math::Rect position) {
    auto p = std::make_unique<GW2MouseHighlight>();
    p->Initialize(parent, position);
    GW2MouseHighlight* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }
  ~GW2MouseHighlight() override;

  static gui::CWBItem* Factory(gui::CWBItem* root, const CXMLNode& node,
                               math::Rect& pos);

  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;

 private:
  void OnDraw(gui::CWBDrawAPI* api) override;

  math::Point last_pos_;
  math::Point last_changed_pos_;
  int num_same_frames_ = 0;
};
