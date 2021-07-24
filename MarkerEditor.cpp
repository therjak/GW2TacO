﻿#include "MarkerEditor.h"

#include "Bedrock/Whiteboard/Application.h"
#include "Bedrock/Whiteboard/Button.h"
#include "Bedrock/Whiteboard/Label.h"
#include "MumbleLink.h"
#include "OverlayConfig.h"
#include "TrailLogger.h"
#include "gw2tactical.h"

bool GW2MarkerEditor::IsMouseTransparent(CPoint& ClientSpacePoint,
                                         WBMESSAGE MessageType) {
  return true;
}

GW2MarkerEditor::GW2MarkerEditor(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position) {
  App->GenerateGUITemplate(this, "gw2pois", "markereditor");
}

GW2MarkerEditor::~GW2MarkerEditor() = default;

CWBItem* GW2MarkerEditor::Factory(CWBItem* Root, CXMLNode& node, CRect& Pos) {
  return GW2MarkerEditor::Create(Root, Pos).get();
}

void GW2MarkerEditor::OnDraw(CWBDrawAPI* API) {
  if (!HasConfigValue("AutoHideMarkerEditor"))
    SetConfigValue("AutoHideMarkerEditor", 1);

  bool autoHide = GetConfigValue("AutoHideMarkerEditor");

  if (!HasConfigValue("TacticalLayerVisible"))
    SetConfigValue("TacticalLayerVisible", 1);

  if (!GetConfigValue("TacticalLayerVisible")) return;

  if (!mumbleLink.IsValid()) return;

  if (mumbleLink.mapID == -1) return;
  for (auto& poi : POIs) {
    auto& cpoi = poi.second;

    if (cpoi.mapID != mumbleLink.mapID) continue;
    if (cpoi.External) continue;

    CVector3 v = cpoi.position - CVector3(mumbleLink.charPosition);
    if (v.Length() < cpoi.typeData.triggerRange) {
      if (autoHide) {
        if (Hidden)
          for (uint32_t z = 0; z < NumChildren(); z++) GetChild(z)->Hide(false);
        Hidden = false;
      }

      if (CurrentPOI != cpoi.guid) {
        CWBLabel* type =
            dynamic_cast<CWBLabel*>(FindChildByID("markertype", "label"));
        if (type) {
          std::string typeName;
          if (cpoi.category) typeName = cpoi.category->GetFullTypeName();

          type->SetText("Type: " + typeName);
          if (typeName.empty()) type->SetText("Type: undefined");
        }
      }

      CurrentPOI = cpoi.guid;
      return;
    }
  }

  if (autoHide) {
    if (!Hidden)
      for (uint32_t x = 0; x < NumChildren(); x++) GetChild(x)->Hide(true);

    Hidden = true;
  } else {
    if (Hidden)
      for (uint32_t x = 0; x < NumChildren(); x++) GetChild(x)->Hide(false);

    Hidden = false;
  }
}

bool GW2MarkerEditor::MessageProc(const CWBMessage& Message) {
  switch (Message.GetMessage()) {
    case WBM_COMMAND: {
      if (Hidden) break;

      CWBButton* b = dynamic_cast<CWBButton*>(
          App->FindItemByGuid(Message.GetTarget(), _T( "button" )));
      if (!b) break;
      if (b->GetID() == _T( "changemarkertype" )) {
        auto ctx = b->OpenContextMenu(App->GetMousePos());
        OpenTypeContextMenu(ctx, CategoryList, false, 0, true);
        ChangeDefault = false;
      }

      if (b->GetID() == _T( "changedefaultmarkertype" )) {
        auto ctx = b->OpenContextMenu(App->GetMousePos());
        OpenTypeContextMenu(ctx, CategoryList, false, 0, true);
        ChangeDefault = true;
      }

      if (b->GetID() == _T( "starttrail" )) {
        b->Push(!b->IsPushed());
        b->SetText(b->IsPushed() ? "Stop Recording" : "Start New Trail");
        GW2TrailDisplay* trails = dynamic_cast<GW2TrailDisplay*>(
            App->GetRoot()->FindChildByID(_T( "trail" ), _T( "gw2Trails" )));
        if (trails) trails->StartStopTrailRecording(b->IsPushed());
      }

      if (b->GetID() == _T( "pausetrail" )) {
        GW2TrailDisplay* trails = dynamic_cast<GW2TrailDisplay*>(
            App->GetRoot()->FindChildByID(_T( "trail" ), _T( "gw2Trails" )));
        if (trails) trails->PauseTrail(!b->IsPushed());
      }

      if (b->GetID() == _T( "startnewsection" )) {
        GW2TrailDisplay* trails = dynamic_cast<GW2TrailDisplay*>(
            App->GetRoot()->FindChildByID(_T( "trail" ), _T( "gw2Trails" )));
        if (trails) trails->PauseTrail(false, true);
      }

      if (b->GetID() == _T( "deletelastsegment" )) {
        GW2TrailDisplay* trails = dynamic_cast<GW2TrailDisplay*>(
            App->GetRoot()->FindChildByID(_T( "trail" ), _T( "gw2Trails" )));
        if (trails) trails->DeleteLastTrailSegment();
      }

      if (b->GetID() == _T( "savetrail" )) {
        GW2TrailDisplay* trails = dynamic_cast<GW2TrailDisplay*>(
            App->GetRoot()->FindChildByID(_T( "trail" ), _T( "gw2Trails" )));
        if (trails) trails->ExportTrail();
      }

      if (b->GetID() == _T( "loadtrail" )) {
        GW2TrailDisplay* trails = dynamic_cast<GW2TrailDisplay*>(
            App->GetRoot()->FindChildByID(_T( "trail" ), _T( "gw2Trails" )));
        if (trails) trails->ImportTrail();
      }

    } break;

    case WBM_CONTEXTMESSAGE:
      if (Message.Data >= 0 && Message.Data < CategoryList.size()) {
        if (!ChangeDefault) {
          POIs[CurrentPOI].SetCategory(App, CategoryList[Message.Data]);
          ExportPOIS();
          CWBLabel* type =
              dynamic_cast<CWBLabel*>(FindChildByID("markertype", "label"));
          if (type)
            type->SetText("Marker Type: " +
                          CategoryList[Message.Data]->GetFullTypeName());
        } else {
          extern std::string DefaultMarkerCategory;
          DefaultMarkerCategory = CategoryList[Message.Data]->GetFullTypeName();
          CWBLabel* type = dynamic_cast<CWBLabel*>(
              FindChildByID("defaultmarkertype", "label"));
          if (type)
            type->SetText("Default Marker Type: " +
                          CategoryList[Message.Data]->GetFullTypeName());
        }
      }

      break;

    default:
      break;
  }

  return CWBItem::MessageProc(Message);
}
