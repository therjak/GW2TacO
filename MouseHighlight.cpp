#include "MouseHighlight.h"

#include "OverlayConfig.h"

CColor CGAPalette[] = {0xffaa0000, 0xffff5555, 0xff000000, 0xff555555,
                       0xff0000aa, 0xff5555ff, 0xff00aa00, 0xff55ff55,
                       0xff00aaaa, 0xff55ffff, 0xffaa00aa, 0xffff55ff,
                       0xffaa5500, 0xffffff55, 0xffaaaaaa, 0xffffffff};

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
    API->DrawRect(CRect(cp.x - 1, cl.y1, cp.x + 2, cl.y2), 0xff000000);
    API->DrawRect(CRect(cl.x1, cp.y - 1, cl.x2, cp.y + 2), 0xff000000);
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
