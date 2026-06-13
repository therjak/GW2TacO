module;

#include "src/base/color.h"

module taco.overlay_window;

import math;
import taco.overlay_config;
import whiteboard;

using math::Point;
using math::Rect;

bool OverlayWindow::IsMouseTransparent(const Point& ClientSpacePoint,
                                       gui::MessageType message_type) {
  if (GetConfigValue("EditMode")) return false;
  return true;
}

OverlayWindow::OverlayWindow() : GuiType() {}

OverlayWindow::~OverlayWindow() { SetWindowPosition(GetID(), GetPosition()); }

gui::Widget* OverlayWindow::Factory(gui::Widget* Root, const XMLNode& node,
                                    const Rect& Pos) {
  return OverlayWindow::Create(Root, Pos);
}

void OverlayWindow::OnDraw(gui::DrawAPI* API) {
  if (!GetConfigValue("EditMode")) return;
  gui::Window::OnDraw(API);
}

bool OverlayWindow::MessageProc(const gui::Message& message) {
  switch (message.Get()) {
    case gui::WBM_LEFTBUTTONDOWN:
      if (gui::Window::MessageProc(message)) return true;
      if (GetMouseItem() == this) {
        SetCapture();
        SavePosition();

        if (style_ & gui::WB_WINDOW_CLOSEABLE) {
          if (GetElementPos(gui::WindowElement::WB_WINELEMENT_CLOSE)
                  .Contains(ScreenToClient(message.GetPosition()))) {
            drag_mode_ = gui::WB_DRAGMODE_CLOSEBUTTON;
            return true;
          }
        }

        if (style_ & gui::WB_WINDOW_RESIZABLE) {
          drag_mode_ = GetBorderSelectionArea(message.GetPosition());
          if (drag_mode_ & gui::WB_DRAGMASK) return true;
        }

        if (style_ & gui::WB_WINDOW_MOVEABLE) {
          drag_mode_ = gui::WB_DRAGMODE_MOVE;
          return true;
        }

        drag_mode_ = 0;
      }
      break;
    case gui::WBM_REPOSITION:
      SetWindowPosition(GetID(), GetPosition());
      break;
    case gui::WBM_CLOSE:
      SetWindowOpenState(GetID(), false);
      break;
  }

  return gui::Window::MessageProc(message);
}
