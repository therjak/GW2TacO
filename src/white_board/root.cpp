#include "src/white_board/root.h"

#include "src/white_board/application.h"
#include "src/white_board/message.h"

using math::CRect;

bool CWBRoot::MessageProc(const CWBMessage& Message) {
  switch (Message.GetMessage()) {
    default:
      break;

    case WBM_REPOSITION: {
      if (Message.GetTarget() == GetGuid() || Message.GetTarget() == 0) {
        if (!Message.Resized) return true;  // ignore movement

        CWBItem::MessageProc(Message);
        for (uint32_t x = 0; x < NumChildren(); x++) {
          App->SendMessage(CWBMessage(
              App, WBM_ROOTRESIZE, GetChild(x)->GetGuid(),
              Message.Rectangle.Width(), Message.Rectangle.Height()));
        }
        return true;
      }
    }
  }

  return CWBItem::MessageProc(Message);
}

CWBRoot::CWBRoot(CWBItem* Parent, const CRect& Pos) : CWBItem() {
  Initialize(Parent, Pos);
}

CWBRoot::~CWBRoot() = default;

void CWBRoot::SetApplication(CWBApplication* Application) { App = Application; }

bool CWBRoot::Initialize(CWBItem* Parent, const CRect& Position) {
  if (!CWBItem::Initialize(Parent, Position)) return false;
  return true;
}

void CWBRoot::OnDraw(CWBDrawAPI* API) {}
