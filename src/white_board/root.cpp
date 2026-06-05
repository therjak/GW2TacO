module;
#include <cstdint>
#include <memory>

module whiteboard;

import :application;
import :message;
import :root;

using math::Rect;

namespace gui {

bool Root::MessageProc(const CWBMessage& Message) {
  switch (Message.Get()) {
    default:
      break;

    case WBM_REPOSITION: {
      if (Message.GetTarget() == GetGuid() || Message.GetTarget() == 0) {
        if (!Message.Resized()) return true;  // ignore movement

        Widget::MessageProc(Message);
        for (uint32_t x = 0; x < NumChildren(); x++) {
          GetApplication()->Send(CWBMessage(
              GetApplication(), WBM_ROOTRESIZE, GetChild(x)->GetGuid(),
              Message.Rectangle().Width(), Message.Rectangle().Height()));
        }
        return true;
      }
    }
  }

  return Widget::MessageProc(Message);
}

Root::Root() : GuiType() {}

Root::~Root() = default;

void Root::SetApplication(Application* Application) { app_ = Application; }

void Root::OnDraw(DrawAPI* API) {}

}  // namespace gui
