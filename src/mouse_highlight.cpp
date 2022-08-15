#include "src/mouse_highlight.h"

#include "src/overlay_config.h"
#include "src/white_board/application.h"

using math::CPoint;
using math::CRect;

void GW2MouseHighlight::OnDraw(CWBDrawAPI* API) {
  if (!GetConfigValue("MouseHighlightVisible")) return;

  POINT pos;
  GetCursorPos(&pos);
  ::ScreenToClient(App->GetHandle(), &pos);
  CPoint cp(pos.x, pos.y);

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

CWBItem* GW2MouseHighlight::Factory(CWBItem* Root, const CXMLNode& node,
                                    CRect& Pos) {
  return GW2MouseHighlight::Create(Root, Pos);
}

bool GW2MouseHighlight::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                           WBMESSAGE MessageType) {
  return true;
}
