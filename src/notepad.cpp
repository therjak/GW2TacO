module;
#include "src/base/rectangle.h"
#include "src/base/vector.h"
#include "src/gw2_tactical.h"
#include "src/white_board/application.h"
#include "src/base/stream_writer.h"
#include "src/base/stream_reader.h"
#include "src/util/xml_node.h"

module taco.notepad;

import whiteboard.text_box;
import taco.mumble_link;
import taco.overlay_config;

using math::CPoint;
using math::CRect;

bool GW2Notepad::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                    WBMESSAGE MessageType) {
  return true;
}

GW2Notepad::GW2Notepad(CWBItem* Parent, CRect Position)
    : CWBGuiType(Parent, Position) {
  App->GenerateGUITemplate(this, "gw2pois", "notepad");

  SetID("notepad");

  CStreamReaderMemory nptext;
  if (!nptext.Open("notepad.txt")) return;

  auto* tb = dynamic_cast<CWBTextBox*>(FindChildByID("notepad", "textbox"));
  if (!tb) return;

  tb->SetForcedMouseTransparency(true);
  tb->SetText(std::string_view(reinterpret_cast<const char*>(nptext.GetData()),
                               static_cast<int32_t>(nptext.GetLength())));
  tb->SetCursorPos(0, false);
}

GW2Notepad::~GW2Notepad() {
  auto* tb = dynamic_cast<CWBTextBox*>(FindChildByID("notepad", "textbox"));
  if (!tb) return;

  CStreamWriterFile nptext;
  if (!nptext.Open("notepad.txt")) return;

  nptext.Write(tb->GetText());
}

CWBItem* GW2Notepad::Factory(CWBItem* Root, CXMLNode& node, CRect& Pos) {
  return GW2Notepad::Create(Root, Pos);
}

void GW2Notepad::StartEdit() {
  auto* tb = dynamic_cast<CWBTextBox*>(FindChildByID("notepad", "textbox"));
  if (!tb) return;

  canSetFocus = true;
  tb->SetFocus();
  tb->SetCursorPos(tb->GetText().size(), false);
}

void GW2Notepad::OnDraw(CWBDrawAPI* API) {}

bool GW2Notepad::MessageProc(const CWBMessage& Message) {
  switch (Message.GetMessage()) {
    case WBM_FOCUSGAINED: {
      CWBItem* tb = FindChildByID("notepad", "textbox");
      if (tb->GetGuid() == Message.GetTarget()) {
        tb->SetForcedMouseTransparency(false);
      }
    } break;
    case WBM_FOCUSLOST: {
      CWBItem* tb = FindChildByID("notepad", "textbox");
      if (tb->GetGuid() == Message.GetTarget()) {
        tb->SetForcedMouseTransparency(true);
      }
    } break;
  }

  return CWBItem::MessageProc(Message);
}
