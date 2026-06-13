module;

#include <windows.h>

#include "src/base/color.h"

module taco.mouse_highlight;

import taco.overlay_config;
import whiteboard;

using math::Point;
using math::Rect;

void GW2MouseHighlight::OnDraw(gui::DrawAPI* api) {
  if (!GetConfigValue("MouseHighlightVisible")) return;

  POINT pos;
  GetCursorPos(&pos);
  ::ScreenToClient(GetApplication()->GetHandle(), &pos);
  Point cp(pos.x, pos.y);

  if ((GetKeyState(VK_RBUTTON) & 0x100) != 0) {
    cp = last_pos_;
  } else {
    cp = ScreenToClient(cp);
  }

  last_pos_ = cp;

  if (cp == last_changed_pos_) {
    num_same_frames_++;
  } else {
    num_same_frames_ = 0;
    last_changed_pos_ = cp;
  }

  Rect cl = GetClientRect();

  int color = GetConfigValue("MouseHighlightColor");

  if (GetConfigValue("MouseHighlightOutline")) {
    api->DrawRect(Rect(cp.x - 1, cl.y1, cp.x + 2, cl.y2), Color{0xff000000});
    api->DrawRect(Rect(cl.x1, cp.y - 1, cl.x2, cp.y + 2), Color{0xff000000});
  }

  api->DrawRect(Rect(cp.x, cl.y1, cp.x + 1, cl.y2), kCGAPalette[color]);
  api->DrawRect(Rect(cl.x1, cp.y, cl.x2, cp.y + 1), kCGAPalette[color]);
}

GW2MouseHighlight::GW2MouseHighlight() : GuiType() {}

GW2MouseHighlight::~GW2MouseHighlight() = default;

gui::Widget* GW2MouseHighlight::Factory(gui::Widget* root, const XMLNode& node,
                                        const Rect& pos) {
  return GW2MouseHighlight::Create(root, pos);
}

bool GW2MouseHighlight::IsMouseTransparent(const Point& client_space_point,
                                           gui::MessageType type) {
  return true;
}
