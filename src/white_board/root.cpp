module;
#include <cstdint>
#include <memory>

module whiteboard;

import :application;
import :root;
import :message;

using math::CRect;

bool CWBRoot::MessageProc(const CWBMessage& Message) {
  switch (Message.Get()) {
    default:
      break;

    case WBM_REPOSITION: {
      if (Message.GetTarget() == GetGuid() || Message.GetTarget() == 0) {
        if (!Message.Resized()) return true;  // ignore movement

        CWBItem::MessageProc(Message);
        for (uint32_t x = 0; x < NumChildren(); x++) {
          App->Send(CWBMessage(App, WBM_ROOTRESIZE, GetChild(x)->GetGuid(),
                               Message.Rectangle().Width(),
                               Message.Rectangle().Height()));
        }
        return true;
      }
    }
  }

  return CWBItem::MessageProc(Message);
}

CWBRoot::CWBRoot() : CWBGuiType() {}

CWBRoot::~CWBRoot() = default;

void CWBRoot::SetApplication(CWBApplication* Application) { App = Application; }

void CWBRoot::OnDraw(CWBDrawAPI* API) {}
