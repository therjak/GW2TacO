#include "Notepad.h"

#include "MumbleLink.h"
#include "OverlayConfig.h"
#include "gw2tactical.h"

bool GW2Notepad::IsMouseTransparent(CPoint& ClientSpacePoint,
                                    WBMESSAGE MessageType) {
  return true;
}

GW2Notepad::GW2Notepad(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position) {
  App->GenerateGUITemplate(this, "gw2pois", "notepad");

  SetID("notepad");

  CStreamReaderMemory nptext;
  if (!nptext.Open("notepad.txt")) return;

  CWBTextBox* tb =
      dynamic_cast<CWBTextBox*>(FindChildByID("notepad", "textbox"));
  if (!tb) return;

  tb->SetForcedMouseTransparency(true);
  tb->SetText(std::string_view(reinterpret_cast<char*>(nptext.GetData()),
                               static_cast<int32_t>(nptext.GetLength())));
  tb->SetCursorPos(0, false);
}

GW2Notepad::~GW2Notepad() {
  CWBTextBox* tb =
      dynamic_cast<CWBTextBox*>(FindChildByID("notepad", "textbox"));
  if (!tb) return;

  CStreamWriterFile nptext;
  if (!nptext.Open("notepad.txt")) return;

  nptext.Write(tb->GetText());
}

CWBItem* GW2Notepad::Factory(CWBItem* Root, CXMLNode& node, CRect& Pos) {
  return GW2Notepad::Create(Root, Pos).get();
}

void GW2Notepad::StartEdit() {
  CWBTextBox* tb =
      dynamic_cast<CWBTextBox*>(FindChildByID("notepad", "textbox"));
  if (!tb) return;

  canSetFocus = true;
  tb->SetFocus();
  tb->SetCursorPos(tb->GetText().size(), false);
}

void GW2Notepad::OnDraw(CWBDrawAPI* API) {}

bool GW2Notepad::MessageProc(CWBMessage& Message) {
  switch (Message.GetMessage()) {
    case WBM_FOCUSGAINED: {
      CWBItem* tb = FindChildByID("notepad", "textbox");
      if (tb->GetGuid() == Message.GetTarget())
        tb->SetForcedMouseTransparency(false);
    } break;
    case WBM_FOCUSLOST: {
      CWBItem* tb = FindChildByID("notepad", "textbox");
      if (tb->GetGuid() == Message.GetTarget())
        tb->SetForcedMouseTransparency(true);
    } break;
  }

  return CWBItem::MessageProc(Message);
}
