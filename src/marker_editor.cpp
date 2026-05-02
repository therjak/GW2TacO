module;
#include <string>
#include <vector>

#include "src/gw2_tactical.h"

module taco.marker_editor;

import math;
import taco.mumble_link;
import taco.overlay_config;
import taco.trail_logger;
import whiteboard;
import xml;

using math::Point;
using math::Rect;
using math::Vector3;

bool GW2MarkerEditor::IsMouseTransparent(const Point& client_space_point,
                                         gui::WBMESSAGE message_type) {
  return true;
}

GW2MarkerEditor::GW2MarkerEditor() : CWBGuiType() {
  App->GenerateGUITemplate(this, "gw2pois", "markereditor");
}

GW2MarkerEditor::~GW2MarkerEditor() = default;

gui::CWBItem* GW2MarkerEditor::Factory(gui::CWBItem* root, CXMLNode& node,
                                       Rect& pos) {
  return GW2MarkerEditor::Create(root, pos);
}

void GW2MarkerEditor::OnDraw(gui::CWBDrawAPI* api) {
  bool auto_hide = GetConfigValue("AutoHideMarkerEditor");

  if (!GetConfigValue("TacticalLayerVisible")) return;

  if (!mumbleLink.IsValid()) return;

  if (mumbleLink.map_id == -1) return;

  auto& m_pois = GetMapPOIs();
  for (auto& poi : m_pois) {
    auto& cpoi = poi.second;

    if (cpoi.map_id != mumbleLink.map_id) continue;
    if (cpoi.External) continue;

    Vector3 v = cpoi.position - Vector3(mumbleLink.char_position);
    if (v.Length() < cpoi.type_data_.trigger_range_) {
      if (auto_hide) {
        if (hidden_) {
          for (uint32_t z = 0; z < NumChildren(); z++) GetChild(z)->Hide(false);
        }
        hidden_ = false;
      }

      if (current_poi_ != cpoi.guid) {
        auto* type =
            dynamic_cast<gui::CWBLabel*>(FindChildByID("markertype", "label"));
        if (type) {
          std::string type_name;
          if (cpoi.category) type_name = cpoi.category->GetFullTypeName();

          type->SetText("Type: " + type_name);
          if (type_name.empty()) type->SetText("Type: undefined");
        }
      }

      current_poi_ = cpoi.guid;
      return;
    }
  }

  if (auto_hide) {
    if (!hidden_) {
      for (uint32_t x = 0; x < NumChildren(); x++) GetChild(x)->Hide(true);
    }

    hidden_ = true;
  } else {
    if (hidden_) {
      for (uint32_t x = 0; x < NumChildren(); x++) GetChild(x)->Hide(false);
    }

    hidden_ = false;
  }
}

bool GW2MarkerEditor::MessageProc(const gui::CWBMessage& message) {
  switch (message.Get()) {
    case gui::WBM_COMMAND: {
      if (hidden_) break;

      auto* b = dynamic_cast<gui::CWBButton*>(
          App->FindItemByGuid(message.GetTarget(), "button"));
      if (!b) break;
      if (b->GetID() == "changemarkertype") {
        auto ctx = b->OpenContextMenu(App->GetMousePos());
        OpenTypeContextMenu(ctx, category_list_, false, 0, true);
        change_default_ = false;
      }

      if (b->GetID() == "changedefaultmarkertype") {
        auto ctx = b->OpenContextMenu(App->GetMousePos());
        OpenTypeContextMenu(ctx, category_list_, false, 0, true);
        change_default_ = true;
      }

      if (b->GetID() == "starttrail") {
        b->Push(!b->IsPushed());
        b->SetText(b->IsPushed() ? "Stop Recording" : "Start New Trail");
        auto* trails = dynamic_cast<GW2TrailDisplay*>(
            App->GetRoot()->FindChildByID("trail", "gw2Trails"));
        if (trails) trails->StartStopTrailRecording(b->IsPushed());
      }

      if (b->GetID() == "pausetrail") {
        auto* trails = dynamic_cast<GW2TrailDisplay*>(
            App->GetRoot()->FindChildByID("trail", "gw2Trails"));
        if (trails) trails->PauseTrail(!b->IsPushed());
      }

      if (b->GetID() == "startnewsection") {
        auto* trails = dynamic_cast<GW2TrailDisplay*>(
            App->GetRoot()->FindChildByID("trail", "gw2Trails"));
        if (trails) trails->PauseTrail(false, true);
      }

      if (b->GetID() == "deletelastsegment") {
        auto* trails = dynamic_cast<GW2TrailDisplay*>(
            App->GetRoot()->FindChildByID("trail", "gw2Trails"));
        if (trails) trails->DeleteLastTrailSegment();
      }

      if (b->GetID() == "savetrail") {
        auto* trails = dynamic_cast<GW2TrailDisplay*>(
            App->GetRoot()->FindChildByID("trail", "gw2Trails"));
        if (trails) trails->ExportTrail();
      }

      if (b->GetID() == "loadtrail") {
        auto* trails = dynamic_cast<GW2TrailDisplay*>(
            App->GetRoot()->FindChildByID("trail", "gw2Trails"));
        if (trails) trails->ImportTrail();
      }
    } break;

    case gui::WBM_CONTEXTMESSAGE:
      if (message.Data() >= 0 && message.Data() < category_list_.size()) {
        if (!change_default_) {
          auto& m_pois = GetMapPOIs();
          m_pois[current_poi_].SetCategory(category_list_[message.Data()]);
          ExportPOIS();
          auto* type = dynamic_cast<gui::CWBLabel*>(
              FindChildByID("markertype", "label"));
          if (type) {
            type->SetText("Marker Type: " +
                          category_list_[message.Data()]->GetFullTypeName());
          }
        } else {
          default_marker_category =
              category_list_[message.Data()]->GetFullTypeName();
          auto* type = dynamic_cast<gui::CWBLabel*>(
              FindChildByID("defaultmarkertype", "label"));
          if (type) {
            type->SetText("Default Marker Type: " +
                          category_list_[message.Data()]->GetFullTypeName());
          }
        }
      }

      break;

    default:
      break;
  }

  return gui::CWBItem::MessageProc(message);
}
