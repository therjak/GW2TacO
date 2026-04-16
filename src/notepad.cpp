module;

#include "src/base/stream_reader.h"
#include "src/base/stream_writer.h"
#include "src/gw2_tactical.h"

module taco.notepad;

import xml;
import whiteboard;
import taco.mumble_link;
import taco.overlay_config;
import math;

using math::Point;
using math::Rect;

bool GW2Notepad::IsMouseTransparent(const Point& ClientSpacePoint,
                                    gui::WBMESSAGE MessageType) {
  return true;
}

GW2Notepad::GW2Notepad() : CWBGuiType() {
  App->GenerateGUITemplate(this, "gw2pois", "notepad");

  SetID("notepad");

  CStreamReaderMemory nptext;
  if (!nptext.Open("notepad.txt")) return;

  auto* tb =
      dynamic_cast<gui::CWBTextBox*>(FindChildByID("notepad", "textbox"));
  if (!tb) return;

  tb->SetForcedMouseTransparency(true);
  tb->SetText(std::string_view(reinterpret_cast<const char*>(nptext.GetData()),
                               static_cast<int32_t>(nptext.GetLength())));
  tb->SetCursorPos(0, false);
}

GW2Notepad::~GW2Notepad() {
  auto* tb =
      dynamic_cast<gui::CWBTextBox*>(FindChildByID("notepad", "textbox"));
  if (!tb) return;

  CStreamWriterFile nptext;
  if (!nptext.Open("notepad.txt")) return;

  nptext.Write(tb->GetText());
}

gui::CWBItem* GW2Notepad::Factory(gui::CWBItem* Root, CXMLNode& node,
                                  Rect& Pos) {
  return GW2Notepad::Create(Root, Pos);
}

void GW2Notepad::StartEdit() {
  auto* tb =
      dynamic_cast<gui::CWBTextBox*>(FindChildByID("notepad", "textbox"));
  if (!tb) return;

  canSetFocus = true;
  tb->SetFocus();
  tb->SetCursorPos(tb->GetText().size(), false);
}

void GW2Notepad::OnDraw(gui::CWBDrawAPI* API) {}

bool GW2Notepad::MessageProc(const gui::CWBMessage& Message) {
  switch (Message.Get()) {
    case gui::WBM_FOCUSGAINED: {
      gui::CWBItem* tb = FindChildByID("notepad", "textbox");
      if (tb->GetGuid() == Message.GetTarget()) {
        tb->SetForcedMouseTransparency(false);
      }
    } break;
    case gui::WBM_FOCUSLOST: {
      gui::CWBItem* tb = FindChildByID("notepad", "textbox");
      if (tb->GetGuid() == Message.GetTarget()) {
        tb->SetForcedMouseTransparency(true);
      }
    } break;
  }

  return CWBItem::MessageProc(Message);
}
