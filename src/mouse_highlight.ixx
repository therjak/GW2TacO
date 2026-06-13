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

export constexpr std::array kCGAPalette = {
    Color{0xffaa0000}, Color{0xffff5555}, Color{0xff000000}, Color{0xff555555},
    Color{0xff0000aa}, Color{0xff5555ff}, Color{0xff00aa00}, Color{0xff55ff55},
    Color{0xff00aaaa}, Color{0xff55ffff}, Color{0xffaa00aa}, Color{0xffff55ff},
    Color{0xffaa5500}, Color{0xffffff55}, Color{0xffaaaaaa}, Color{0xffffffff}};

export constexpr std::array kkCGAPaletteNames = {
    "red",   "lightred",   "black",     "gray",      "blue",    "lightblue",
    "green", "lightgreen", "cyan",      "lightcyan", "magenta", "lightmagenta",
    "brown", "yellow",     "lightgray", "white"};

export class GW2MouseHighlight
    : public gui::GuiType<"mousehighlight", gui::Widget> {
 public:
  GW2MouseHighlight();
  static inline GW2MouseHighlight* Create(gui::Widget* parent,
                                          math::Rect position) {
    auto p = std::make_unique<GW2MouseHighlight>();
    p->Initialize(parent, position);
    GW2MouseHighlight* r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }
  ~GW2MouseHighlight() override;

  static gui::Widget* Factory(gui::Widget* root, const XMLNode& node,
                              const math::Rect& pos);

  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::MessageType type) override;

 private:
  void OnDraw(gui::DrawAPI* api) override;

  math::Point last_pos_;
  math::Point last_changed_pos_;
  int num_same_frames_ = 0;
};
