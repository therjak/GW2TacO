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

bool Notepad::IsMouseTransparent(const Point& client_space_point,
                                 gui::WBMESSAGE message_type) {
  return true;
}

Notepad::Notepad() : CWBGuiType() {
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

Notepad::~Notepad() {
  auto* tb =
      dynamic_cast<gui::CWBTextBox*>(FindChildByID("notepad", "textbox"));
  if (!tb) return;

  CStreamWriterFile nptext;
  if (!nptext.Open("notepad.txt")) return;

  nptext.Write(tb->GetText());
}

gui::CWBItem* Notepad::Factory(gui::CWBItem* root, const CXMLNode& node,
                               const Rect& pos) {
  return Notepad::Create(root, pos);
}

void Notepad::StartEdit() {
  auto* tb =
      dynamic_cast<gui::CWBTextBox*>(FindChildByID("notepad", "textbox"));
  if (!tb) return;

  can_set_focus_ = true;
  tb->SetFocus();
  tb->SetCursorPos(tb->GetText().size(), false);
}

void Notepad::OnDraw(gui::CWBDrawAPI* api) {}

bool Notepad::MessageProc(const gui::CWBMessage& message) {
  switch (message.Get()) {
    case gui::WBM_FOCUSGAINED: {
      gui::CWBItem* tb = FindChildByID("notepad", "textbox");
      if (tb->GetGuid() == message.GetTarget()) {
        tb->SetForcedMouseTransparency(false);
      }
    } break;
    case gui::WBM_FOCUSLOST: {
      gui::CWBItem* tb = FindChildByID("notepad", "textbox");
      if (tb->GetGuid() == message.GetTarget()) {
        tb->SetForcedMouseTransparency(true);
      }
    } break;
  }

  return CWBItem::MessageProc(message);
}
