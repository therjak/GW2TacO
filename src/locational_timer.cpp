module;
#include <format>
#include <vector>

#include "src/gw2_tactical.h"

module taco.locational_timer;

import math;
import taco.language;
import taco.mumble_link;
import taco.overlay_config;
import time;
import xml;

using math::Point;
using math::Rect;

std::vector<LocationalTimer> LocationalTimers;

void ImportLocationalTimers() {
  CXMLDocument d;
  if (!d.LoadFromFile("locationaltimers.xml")) return;

  if (!d.GetDocumentNode().GetChildCount("timers")) return;
  CXMLNode root = d.GetDocumentNode().GetChild("timers");

  for (int32_t x = 0; x < root.GetChildCount("areatriggeredtimer"); x++) {
    LocationalTimer t;
    t.ImportData(root.GetChild("areatriggeredtimer", x));
    LocationalTimers.push_back(t);
  }
}

LocationalTimer::LocationalTimer() = default;

LocationalTimer::~LocationalTimer() = default;

void LocationalTimer::Update() {
  if (mumbleLink.map_id != map_id_) {
    is_running_ = false;
    return;
  }

  if (!is_running_) {
    if (enter_sphere_.Contains(mumbleLink.charPosition)) {
      is_running_ = true;
      start_time_ = GetTime();
    }
  }

  if (is_running_) {
    if ((GetTime() - start_time_) / 1000.0f > timer_length_) is_running_ = false;
    if (!exit_sphere_.Contains(mumbleLink.charPosition)) is_running_ = false;
    if ((reset_point_ - mumbleLink.charPosition).Length() < 0.1) {
      is_running_ = false;
    }
  }
}

void LocationalTimer::ImportData(const CXMLNode& node) {
  if (node.HasAttribute("mapid")) node.GetAttributeAsInteger("mapid", &map_id_);
  if (node.HasAttribute("length")) {
    node.GetAttributeAsInteger("length", &timer_length_);
  }
  if (node.HasAttribute("startdelay")) {
    node.GetAttributeAsInteger("startdelay", &start_delay_);
  }

  if (node.HasAttribute("enterspherex")) {
    node.GetAttributeAsFloat("enterspherex", &enter_sphere_.Position.x);
  }
  if (node.HasAttribute("enterspherey")) {
    node.GetAttributeAsFloat("enterspherey", &enter_sphere_.Position.y);
  }
  if (node.HasAttribute("enterspherez")) {
    node.GetAttributeAsFloat("enterspherez", &enter_sphere_.Position.z);
  }
  if (node.HasAttribute("entersphererad")) {
    node.GetAttributeAsFloat("entersphererad", &enter_sphere_.Radius);
  }

  if (node.HasAttribute("exitspherex")) {
    node.GetAttributeAsFloat("exitspherex", &exit_sphere_.Position.x);
  }
  if (node.HasAttribute("exitspherey")) {
    node.GetAttributeAsFloat("exitspherey", &exit_sphere_.Position.y);
  }
  if (node.HasAttribute("exitspherez")) {
    node.GetAttributeAsFloat("exitspherez", &exit_sphere_.Position.z);
  }
  if (node.HasAttribute("exitsphererad")) {
    node.GetAttributeAsFloat("exitsphererad", &exit_sphere_.Radius);
  }

  if (node.HasAttribute("resetpointx")) {
    node.GetAttributeAsFloat("resetpointx", &reset_point_.x);
  }
  if (node.HasAttribute("resetpointy")) {
    node.GetAttributeAsFloat("resetpointy", &reset_point_.y);
  }
  if (node.HasAttribute("resetpointz")) {
    node.GetAttributeAsFloat("resetpointz", &reset_point_.z);
  }

  for (int32_t x = 0; x < node.GetChildCount("timeevent"); x++) {
    CXMLNode te = node.GetChild("timeevent", x);
    TimerEvent tmr;
    if (te.HasAttribute("text")) tmr.text = te.GetAttributeAsString("text");
    if (te.HasAttribute("timestamp")) {
      te.GetAttributeAsInteger("timestamp", &tmr.time);
    }
    if (te.HasAttribute("countdown")) {
      te.GetAttributeAsInteger("countdown", &tmr.countdown_length);
    }
    if (te.HasAttribute("onscreentime")) {
      te.GetAttributeAsInteger("onscreentime", &tmr.on_screen_length);
    }
    events_.push_back(tmr);
  }
}

void TimerDisplay::OnDraw(gui::CWBDrawAPI* api) {
  if (!GetConfigValue("LocationalTimersVisible")) return;

  int32_t tme = GetTime();
  gui::CWBFont* f = GetFont(GetState());

  int32_t ypos = static_cast<int32_t>(
      math::Lerp(static_cast<float>(GetClientRect().y1),
                 static_cast<float>(GetClientRect().y2), 0.25f));

  for (auto& t : LocationalTimers) {
    t.Update();
    if (!t.is_running_) continue;

    float timepos = (tme - t.start_time_) / 1000.0f - t.start_delay_;

    for (auto& e : t.events_) {
      if (!(timepos > e.time - e.countdown_length &&
            timepos < e.time + e.on_screen_length)) {
        continue;
      }

      auto s = e.text;
      if (timepos < e.time && timepos > e.time - e.countdown_length) {
        s += std::format(" in {:d}", static_cast<int32_t>(e.time - timepos));
      }

      Point pos = f->GetTextPosition(
          s,
          Rect(static_cast<float>(GetClientRect().x1), static_cast<float>(ypos),
               static_cast<float>(GetClientRect().x2),
               static_cast<float>(ypos)),
          gui::WBTEXTALIGNMENTX::WBTA_CENTERX,
          gui::WBTEXTALIGNMENTY::WBTA_CENTERY, gui::WBTEXTTRANSFORM::WBTT_NONE,
          true);
      ypos += f->GetLineHeight();
      f->Write(api, s, pos);
    }
  }
}

bool TimerDisplay::IsMouseTransparent(const Point& client_space_point,
                                      gui::WBMESSAGE message_type) {
  return true;
}

TimerDisplay::TimerDisplay() : CWBGuiType() {}

TimerDisplay::~TimerDisplay() = default;

gui::CWBItem* TimerDisplay::Factory(gui::CWBItem* root, const CXMLNode& node,
                                    Rect& pos) {
  return TimerDisplay::Create(root, pos);
}
