module taco.overlay_window;

import math;
import taco.overlay_config;
import whiteboard;

using math::CPoint;
using math::CRect;

bool OverlayWindow::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                       gui::WBMESSAGE MessageType) {
  if (GetConfigValue("EditMode")) return false;
  return true;
}

OverlayWindow::OverlayWindow() : CWBGuiType() {}

OverlayWindow::~OverlayWindow() { SetWindowPosition(GetID(), GetPosition()); }

gui::CWBItem* OverlayWindow::Factory(gui::CWBItem* Root, const CXMLNode& node,
                                     CRect& Pos) {
  return OverlayWindow::Create(Root, Pos);
}

void OverlayWindow::OnDraw(gui::CWBDrawAPI* API) {
  if (!GetConfigValue("EditMode")) return;
  gui::CWBWindow::OnDraw(API);
}

bool OverlayWindow::MessageProc(const gui::CWBMessage& Message) {
  switch (Message.Get()) {
    case gui::WBM_LEFTBUTTONDOWN:
      if (gui::CWBWindow::MessageProc(Message)) return true;
      if (App->GetMouseItem() == this) {
        SetCapture();
        SavePosition();

        if (Style & gui::WB_WINDOW_CLOSEABLE) {
          if (GetElementPos(gui::WBWINDOWELEMENT::WB_WINELEMENT_CLOSE)
                  .Contains(ScreenToClient(Message.GetPosition()))) {
            DragMode = gui::WB_DRAGMODE_CLOSEBUTTON;
            return true;
          }
        }

        if (Style & gui::WB_WINDOW_RESIZABLE) {
          DragMode = GetBorderSelectionArea(Message.GetPosition());
          if (DragMode & gui::WB_DRAGMASK) return true;
        }

        if (Style & gui::WB_WINDOW_MOVEABLE) {
          DragMode = gui::WB_DRAGMODE_MOVE;
          return true;
        }

        DragMode = 0;
      }
      break;
    case gui::WBM_REPOSITION:
      SetWindowPosition(GetID(), GetPosition());
      break;
    case gui::WBM_CLOSE:
      SetWindowOpenState(GetID(), false);
      break;
  }

  return gui::CWBWindow::MessageProc(Message);
}
