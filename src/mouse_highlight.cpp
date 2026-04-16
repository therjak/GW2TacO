module;

#include <windows.h>

module taco.mouse_highlight;

import taco.overlay_config;
import whiteboard;

using math::Point;
using math::Rect;

void GW2MouseHighlight::OnDraw(gui::CWBDrawAPI* API) {
  if (!GetConfigValue("MouseHighlightVisible")) return;

  POINT pos;
  GetCursorPos(&pos);
  ::ScreenToClient(App->GetHandle(), &pos);
  Point cp(pos.x, pos.y);

  if ((GetKeyState(VK_RBUTTON) & 0x100) != 0) {
    cp = lastpos;
  } else {
    cp = ScreenToClient(cp);
  }

  lastpos = cp;

  if (cp == lastchangedpos) {
    numSameFrames++;
  } else {
    numSameFrames = 0;
    lastchangedpos = cp;
  }

  Rect cl = GetClientRect();

  int Color = GetConfigValue("MouseHighlightColor");

  if (GetConfigValue("MouseHighlightOutline")) {
    API->DrawRect(Rect(cp.x - 1, cl.y1, cp.x + 2, cl.y2), CColor{0xff000000});
    API->DrawRect(Rect(cl.x1, cp.y - 1, cl.x2, cp.y + 2), CColor{0xff000000});
  }

  API->DrawRect(Rect(cp.x, cl.y1, cp.x + 1, cl.y2), CGAPalette[Color]);
  API->DrawRect(Rect(cl.x1, cp.y, cl.x2, cp.y + 1), CGAPalette[Color]);
}

GW2MouseHighlight::GW2MouseHighlight() : CWBGuiType() {}

GW2MouseHighlight::~GW2MouseHighlight() = default;

gui::CWBItem* GW2MouseHighlight::Factory(gui::CWBItem* Root,
                                         const CXMLNode& node, Rect& Pos) {
  return GW2MouseHighlight::Create(Root, Pos);
}

bool GW2MouseHighlight::IsMouseTransparent(const Point& ClientSpacePoint,
                                           gui::WBMESSAGE MessageType) {
  return true;
}
