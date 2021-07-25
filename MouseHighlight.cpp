#include "MouseHighlight.h"

#include "Bedrock/Whiteboard/Application.h"
#include "OverlayConfig.h"

CColor CGAPalette[] = {
    CColor{0xffaa0000}, CColor{0xffff5555}, CColor{0xff000000},
    CColor{0xff555555}, CColor{0xff0000aa}, CColor{0xff5555ff},
    CColor{0xff00aa00}, CColor{0xff55ff55}, CColor{0xff00aaaa},
    CColor{0xff55ffff}, CColor{0xffaa00aa}, CColor{0xffff55ff},
    CColor{0xffaa5500}, CColor{0xffffff55}, CColor{0xffaaaaaa},
    CColor{0xffffffff}};

std::vector<std::string_view> CGAPaletteNames = {
    "red",   "lightred",   "black",     "gray",      "blue",    "lightblue",
    "green", "lightgreen", "cyan",      "lightcyan", "magenta", "lightmagenta",
    "brown", "yellow",     "lightgray", "white"};

void GW2MouseHighlight::OnDraw(CWBDrawAPI* API) {
  if (!HasConfigValue("MouseHighlightVisible"))
    SetConfigValue("MouseHighlightVisible", 0);

  if (!HasConfigValue("MouseHighlightColor"))
    SetConfigValue("MouseHighlightColor", 0);

  if (!HasConfigValue("MouseHighlightOutline"))
    SetConfigValue("MouseHighlightOutline", 0);

  if (!GetConfigValue("MouseHighlightVisible")) return;

  POINT pos;
  GetCursorPos(&pos);
  ::ScreenToClient((HWND)App->GetHandle(), &pos);
  CPoint cp(pos.x, pos.y);

  if ((GetKeyState(VK_RBUTTON) & 0x100) != 0)
    cp = lastpos;
  else
    cp = ScreenToClient(cp);

  lastpos = cp;

  if (cp == lastchangedpos) {
    numSameFrames++;
  } else {
    numSameFrames = 0;
    lastchangedpos = cp;
  }

  CRect cl = GetClientRect();

  int Color = GetConfigValue("MouseHighlightColor");

  if (GetConfigValue("MouseHighlightOutline")) {
    API->DrawRect(CRect(cp.x - 1, cl.y1, cp.x + 2, cl.y2), CColor{0xff000000});
    API->DrawRect(CRect(cl.x1, cp.y - 1, cl.x2, cp.y + 2), CColor{0xff000000});
  }

  API->DrawRect(CRect(cp.x, cl.y1, cp.x + 1, cl.y2), CGAPalette[Color]);
  API->DrawRect(CRect(cl.x1, cp.y, cl.x2, cp.y + 1), CGAPalette[Color]);
}

GW2MouseHighlight::GW2MouseHighlight(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position) {}

GW2MouseHighlight::~GW2MouseHighlight() = default;

CWBItem* GW2MouseHighlight::Factory(CWBItem* Root, CXMLNode& node, CRect& Pos) {
  return GW2MouseHighlight::Create(Root, Pos).get();
}

bool GW2MouseHighlight::IsMouseTransparent(CPoint& ClientSpacePoint,
                                           WBMESSAGE MessageType) {
  return true;
}
