module;
#include <cstdint>
#include <memory>

module whiteboard;

import :application;
import :message;
import :root;

using math::Rect;

namespace gui {

bool Root::MessageProc(const Message& message) {
  switch (message.Get()) {
    default:
      break;

    case WBM_REPOSITION: {
      if (message.GetTarget() == GetGuid() || message.GetTarget() == 0) {
        if (!message.Resized()) return true;  // ignore movement

        Widget::MessageProc(message);
        for (uint32_t x = 0; x < NumChildren(); x++) {
          GetApplication()->Send(Message(
              GetApplication(), WBM_ROOTRESIZE, GetChild(x)->GetGuid(),
              message.Rectangle().Width(), message.Rectangle().Height()));
        }
        return true;
      }
    }
  }

  return Widget::MessageProc(message);
}

Root::Root() : GuiType() {}

Root::~Root() = default;

void Root::SetApplication(Application* Application) { app_ = Application; }

void Root::OnDraw(DrawAPI* API) {}

}  // namespace gui
