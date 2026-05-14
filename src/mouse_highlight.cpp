module;

#include <windows.h>

#include "src/base/color.h"

module taco.mouse_highlight;

import taco.overlay_config;
import whiteboard;

using math::Point;
using math::Rect;

void GW2MouseHighlight::OnDraw(gui::CWBDrawAPI* api) {
  if (!GetConfigValue("MouseHighlightVisible")) return;

  POINT pos;
  GetCursorPos(&pos);
  ::ScreenToClient(App->GetHandle(), &pos);
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
    api->DrawRect(Rect(cp.x - 1, cl.y1, cp.x + 2, cl.y2), CColor{0xff000000});
    api->DrawRect(Rect(cl.x1, cp.y - 1, cl.x2, cp.y + 2), CColor{0xff000000});
  }

  api->DrawRect(Rect(cp.x, cl.y1, cp.x + 1, cl.y2), CGAPalette[color]);
  api->DrawRect(Rect(cl.x1, cp.y, cl.x2, cp.y + 1), CGAPalette[color]);
}

GW2MouseHighlight::GW2MouseHighlight() : CWBGuiType() {}

GW2MouseHighlight::~GW2MouseHighlight() = default;

gui::CWBItem* GW2MouseHighlight::Factory(gui::CWBItem* root,
                                         const CXMLNode& node,
                                         const Rect& pos) {
  return GW2MouseHighlight::Create(root, pos);
}

bool GW2MouseHighlight::IsMouseTransparent(const Point& client_space_point,
                                           gui::WBMESSAGE message_type) {
  return true;
}
