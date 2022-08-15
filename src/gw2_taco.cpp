#include "src/gw2_taco.h"

#include <shellapi.h>

#include <algorithm>
#include <format>
#include <mutex>
#include <thread>

#include "src/base/logger.h"
#include "src/build_count.h"
#include "src/build_info.h"
#include "src/dungeon_progress.h"
#include "src/gw2_api.h"
#include "src/gw2_tactical.h"
#include "src/language.h"
#include "src/map_timer.h"
#include "src/marker_editor.h"
#include "src/mouse_highlight.h"
#include "src/mumble_link.h"
#include "src/notepad.h"
#include "src/overlay_config.h"
#include "src/overlay_window.h"
#include "src/raid_progress.h"
#include "src/tp_tracker.h"
#include "src/trail_logger.h"
#include "src/ts3_connection.h"
#include "src/ts3_control.h"
#include "src/util/jsonxx.h"
#include "src/white_board/application.h"
#include "src/white_board/button.h"
#include "src/white_board/label.h"
#include "src/white_board/text_box.h"
#include "src/white_board/window.h"

using namespace jsonxx;

using math::CPoint;
using math::CRect;
using math::CVector3;

std::string_view UIFileNames[] = {
    "UI_small.css",
    "UI_normal.css",
    "UI_large.css",
    "UI_larger.css",
};

std::vector<std::string_view> ActionNames = {
    "no_action",                //"No Action",
    "add_marker",               //"Add New Marker",
    "remove_marker",            //"Remove Marker",
    "action_key",               //"Action Key",
    "edit_notepad",             //"Edit notepad",
    "*toggle_trail_recording",  //"*Start/Stop Trail Recording",
    "*pause_trail_recording",   //"*Pause/Resume Trail Recording",
    "*remove_last_trail",       //"*Remove Last Trail Segment",
    "*resume_trail",            //"*Resume Trail By Creating New Section"
    "toggle_tactical_layer",
    "toggle_range_circles",
    "toggle_tactical_compass",
    "toggle_locational_timers",
    "toggle_hp_grids",
    "toggle_mouse_highlight",
    "toggle_map_timer",
    "toggle_ts3_window",
    "toggle_marker_editor",
    "toggle_notepad",
    "toggle_raid_progress",
    "toggle_dungeon_progress",
    "toggle_tp_tracker",
    "toggle_window_edit_mode",
};

std::string_view APIKeyNames[] = {
    "no_action",               //"No Action",
    "ts3_clientquery_plugin",  //"TS3 ClientQuery Plugin",
    "guild_wars_2",            //"Guild Wars 2",
};

enum MainMenuItems {
  Menu_Exit = 0x25472,
  Menu_About,
  Menu_ToggleHighLight,
  Menu_ToggleTactical,
  Menu_ToggleMapTimer,
  Menu_ToggleMarkerEditor,
  Menu_ToggleNotepad,
  Menu_ToggleTS3Control,
  Menu_ToggleRaidProgress,
  Menu_ToggleDungeonProgress,
  Menu_ToggleTPTracker,
  Menu_ToggleTPTracker_OnlyOutbid,
  Menu_ToggleTPTracker_ShowBuys,
  Menu_ToggleTPTracker_ShowSells,
  Menu_ToggleTPTracker_OnlyNextFulfilled,
  Menu_ToggleEditMode,
  Menu_ToggleTacticalsOnEdge,
  Menu_ToggleDrawDistance,
  Menu_DrawWvWNames,
  Menu_ToggleInfoLine,
  Menu_ToggleLocationalTimers,
  Menu_ToggleGW2ExitMode,
  Menu_Interface_Small,
  Menu_Interface_Normal,
  Menu_Interface_Large,
  Menu_Interface_Larger,
  Menu_ToggleVersionCheck,
  Menu_DownloadNewBuild,
  Menu_ToggleRangeCircles,
  Menu_RangeCircleTransparency40,
  Menu_RangeCircleTransparency60,
  Menu_RangeCircleTransparency100,
  Menu_ToggleRangeCircle90,
  Menu_ToggleRangeCircle120,
  Menu_ToggleRangeCircle180,
  Menu_ToggleRangeCircle240,
  Menu_ToggleRangeCircle300,
  Menu_ToggleRangeCircle400,
  Menu_ToggleRangeCircle600,
  Menu_ToggleRangeCircle900,
  Menu_ToggleRangeCircle1200,
  Menu_ToggleRangeCircle1500,
  Menu_ToggleRangeCircle1600,
  Menu_ToggleTacticalCompass,
  Menu_ToggleVsync,
  Menu_ToggleHPGrid,
  Menu_ToggleCompactMapTimer,
  Menu_ToggleMapTimerCategories,
  Menu_ToggleMouseHighlightOutline,
  Menu_MouseHighlightColor0,
  Menu_MouseHighlightColor1,
  Menu_MouseHighlightColor2,
  Menu_MouseHighlightColor3,
  Menu_MouseHighlightColor4,
  Menu_MouseHighlightColor5,
  Menu_MouseHighlightColor6,
  Menu_MouseHighlightColor7,
  Menu_MouseHighlightColor8,
  Menu_MouseHighlightColor9,
  Menu_MouseHighlightColora,
  Menu_MouseHighlightColorb,
  Menu_MouseHighlightColorc,
  Menu_MouseHighlightColord,
  Menu_MouseHighlightColore,
  Menu_MouseHighlightColorf,
  Menu_TS3APIKey,
  Menu_GW2APIKey,
  Menu_ToggleTrailLogging,
  Menu_ToggleAutoHideMarkerEditor,
  Menu_HideOnLoadingScreens,
  Menu_ToggleFadeoutBubble,
  Menu_TacOSettings,
  Menu_Language,
  Menu_ToggleCompactRaids,
  Menu_MarkerVisibility_MiniMap_Off,
  Menu_MarkerVisibility_MiniMap_Default,
  Menu_MarkerVisibility_MiniMap_Force,
  Menu_MarkerVisibility_BigMap_Off,
  Menu_MarkerVisibility_BigMap_Default,
  Menu_MarkerVisibility_BigMap_Force,
  Menu_MarkerVisibility_InGameMap_Off,
  Menu_MarkerVisibility_InGameMap_Default,
  Menu_MarkerVisibility_InGameMap_Force,
  Menu_TrailVisibility_MiniMap_Off,
  Menu_TrailVisibility_MiniMap_Default,
  Menu_TrailVisibility_MiniMap_Force,
  Menu_TrailVisibility_BigMap_Off,
  Menu_TrailVisibility_BigMap_Default,
  Menu_TrailVisibility_BigMap_Force,
  Menu_TrailVisibility_InGameMap_Off,
  Menu_TrailVisibility_InGameMap_Default,
  Menu_TrailVisibility_InGameMap_Force,
  Menu_ToggleMetricSystem,
  Menu_ToggleForceDPIAware,
  Menu_ToggleShowNotificationIcon,
  Menu_ReloadMarkers,
  Menu_DeleteMyMarkers,
  Menu_AddGW2ApiKey,
  Menu_TogglePOIInfoText,

  Menu_OpacityIngame_Solid,
  Menu_OpacityIngame_Transparent,
  Menu_OpacityIngame_Faded,
  Menu_OpacityMap_Solid,
  Menu_OpacityMap_Transparent,
  Menu_OpacityMap_Faded,
  Menu_KeyBindsEnabled,

  Menu_RaidToggles = 0x1000,
  Menu_RaidToggles_End = 0x2000,
  Menu_GW2APIKey_Base = 0x3000,
  Menu_GW2APIKey_End = 0x4000,
  Menu_DeleteGW2APIKey_Base = 0x5000,
  Menu_DeleteGW2APIKey_End = 0x6000,

  Menu_ToggleMapTimerMap = 0x30000,

  Menu_RebindKey_Base = 0x31337,

  Menu_Language_Base = 0x40000,

  Menu_MarkerFilter_Base = 0x65535,
};

bool GW2TacO::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                 WBMESSAGE MessageType) {
  return true;
}

GW2TacO::GW2TacO(CWBItem* Parent, CRect Position) : CWBItem(Parent, Position) {
  GetKeyBindings(KeyBindings);
  GetScriptKeyBindings(ScriptKeyBindings);
}

GW2TacO::~GW2TacO() {
  if (pickupFetcherThread.joinable()) pickupFetcherThread.join();

  // scriptEngines.FreeArray();
}

CWBItem* GW2TacO::Factory(CWBItem* Root, const CXMLNode& node, CRect& Pos) {
  auto ret = GW2TacO::Create(Root, Pos);
  ret->SetFocus();

  return ret;
}

bool iconSizesStored = false;
CRect tacoIconRect;
CRect menuHoverRect;
CRect tpButtonRect;
CRect tpHighlightRect;
int scaleCountDownHack = 0;

void ChangeUIScale(int size) {
  if (size < 0 || size > 3) {
    Log_Err(
        "Someone wants to set the UI size to {:d}! Are you multi-clienting "
        "perhaps?",
        size);
    return;
  }

  extern std::unique_ptr<CWBApplication> App;
  if (!App) return;

  if (App->LoadCSSFromFile(UIFileNames[size], true)) {
    SetConfigValue("InterfaceSize", size);
  }
  App->ReApplyStyle();

  scaleCountDownHack = 2;
  iconSizesStored = false;
}

std::string GW2TacO::GetKeybindString(TacOKeyAction action) {
  for (auto& kb : KeyBindings) {
    if (kb.second == action) {
      return std::format(" [{:c}]", kb.first);
      break;
    }
  }
  return "";
}

bool GW2TacO::MessageProc(const CWBMessage& Message) {
  switch (Message.GetMessage()) {
    case WBM_COMMAND: {
      auto* cb = dynamic_cast<CWBButton*>(
          App->FindItemByGuid(Message.GetTarget(), "clickthroughbutton"));
      if (cb && cb->GetID() == "TPButton") {
        TurnOffTPLight();
        break;
      }

      auto* b = dynamic_cast<CWBButton*>(
          App->FindItemByGuid(Message.GetTarget(), "button"));
      if (!b) break;

      if (b->GetID() == "MenuButton") {
        auto ctx = b->OpenContextMenu(App->GetMousePos());
        ctx->SetID("TacOMenu");

        if (GetConfigValue("TacticalLayerVisible")) {
          auto flt =
              ctx->AddItem(DICT("filtermarkers"), Menu_ToggleTacticalsOnEdge);
          OpenTypeContextMenu(flt, CategoryList, true, Menu_MarkerFilter_Base);
          auto options = ctx->AddItem(DICT("tacticalsettings"), 0);

          options->AddItem(
              DICT("togglepoidistance") +
                  (GetConfigValue("TacticalDrawDistance") ? " [x]" : " [ ]"),
              Menu_ToggleDrawDistance);
          options->AddItem(
              DICT("toggleherdicons") +
                  (GetConfigValue("TacticalIconsOnEdge") ? " [x]" : " [ ]"),
              Menu_ToggleTacticalsOnEdge);
          options->AddItem(
              DICT("toggledrawwvwnames") +
                  (GetConfigValue("DrawWvWNames") ? " [x]" : " [ ]"),
              Menu_DrawWvWNames);
          options->AddItem(
              DICT("togglefadeoutbubble") +
                  (GetConfigValue("FadeoutBubble") ? " [x]" : " [ ]"),
              Menu_ToggleFadeoutBubble);
          options->AddItem(
              DICT("togglemetricsystem") +
                  (GetConfigValue("UseMetricDisplay") ? " [x]" : " [ ]"),
              Menu_ToggleMetricSystem);
          options->AddItem(
              DICT("toggletacticalinfotext") +
                  (GetConfigValue("TacticalInfoTextVisible") ? " [x]" : " [ ]"),
              Menu_TogglePOIInfoText);

          auto opacityMenu = options->AddItem(DICT("markeropacity"), 0);
          auto opacityInGame = opacityMenu->AddItem(DICT("ingameopacity"), 0);
          opacityInGame->AddItem(
              DICT("opacitysolid") +
                  (GetConfigValue("OpacityIngame") == 0 ? " [x]" : " [ ]"),
              Menu_OpacityIngame_Solid);
          opacityInGame->AddItem(
              DICT("opacitytransparent") +
                  (GetConfigValue("OpacityIngame") == 2 ? " [x]" : " [ ]"),
              Menu_OpacityIngame_Transparent);
          opacityInGame->AddItem(
              DICT("opacityfaded") +
                  (GetConfigValue("OpacityIngame") == 1 ? " [x]" : " [ ]"),
              Menu_OpacityIngame_Faded);
          auto opacityMiniMap = opacityMenu->AddItem(DICT("mapopacity"), 0);
          opacityMiniMap->AddItem(
              DICT("opacitysolid") +
                  (GetConfigValue("OpacityMap") == 0 ? " [x]" : " [ ]"),
              Menu_OpacityMap_Solid);
          opacityMiniMap->AddItem(
              DICT("opacitytransparent") +
                  (GetConfigValue("OpacityMap") == 2 ? " [x]" : " [ ]"),
              Menu_OpacityMap_Transparent);
          opacityMiniMap->AddItem(
              DICT("opacityfaded") +
                  (GetConfigValue("OpacityMap") == 1 ? " [x]" : " [ ]"),
              Menu_OpacityMap_Faded);

          auto visibilityMenu = options->AddItem(DICT("visibilitymenu"), 0);
          auto markerSubMenu =
              visibilityMenu->AddItem(DICT("markervisibilitymenu"), 0);
          auto markerInGameSubMenu =
              markerSubMenu->AddItem(DICT("ingamevisibility"), 0);
          markerInGameSubMenu->AddItem(
              DICT("defaultvisibility") +
                  (GetConfigValue("ShowInGameMarkers") == 1 ? " [x]" : " [ ]"),
              Menu_MarkerVisibility_InGameMap_Default);
          markerInGameSubMenu->AddItem(
              DICT("forceonvisibility") +
                  (GetConfigValue("ShowInGameMarkers") == 2 ? " [x]" : " [ ]"),
              Menu_MarkerVisibility_InGameMap_Force);
          markerInGameSubMenu->AddItem(
              DICT("forceoffvisibility") +
                  (GetConfigValue("ShowInGameMarkers") == 0 ? " [x]" : " [ ]"),
              Menu_MarkerVisibility_InGameMap_Off);
          auto markerMiniMapSubMenu =
              markerSubMenu->AddItem(DICT("minimapvisibility"), 0);
          markerMiniMapSubMenu->AddItem(
              DICT("defaultvisibility") +
                  (GetConfigValue("ShowMinimapMarkers") == 1 ? " [x]" : " [ ]"),
              Menu_MarkerVisibility_MiniMap_Default);
          markerMiniMapSubMenu->AddItem(
              DICT("forceonvisibility") +
                  (GetConfigValue("ShowMinimapMarkers") == 2 ? " [x]" : " [ ]"),
              Menu_MarkerVisibility_MiniMap_Force);
          markerMiniMapSubMenu->AddItem(
              DICT("forceoffvisibility") +
                  (GetConfigValue("ShowMinimapMarkers") == 0 ? " [x]" : " [ ]"),
              Menu_MarkerVisibility_MiniMap_Off);
          auto markerMapSubMenu =
              markerSubMenu->AddItem(DICT("mapvisibility"), 0);
          markerMapSubMenu->AddItem(
              DICT("defaultvisibility") +
                  (GetConfigValue("ShowBigmapMarkers") == 1 ? " [x]" : " [ ]"),
              Menu_MarkerVisibility_BigMap_Default);
          markerMapSubMenu->AddItem(
              DICT("forceonvisibility") +
                  (GetConfigValue("ShowBigmapMarkers") == 2 ? " [x]" : " [ ]"),
              Menu_MarkerVisibility_BigMap_Force);
          markerMapSubMenu->AddItem(
              DICT("forceoffvisibility") +
                  (GetConfigValue("ShowBigmapMarkers") == 0 ? " [x]" : " [ ]"),
              Menu_MarkerVisibility_BigMap_Off);
          auto trailSubMenu =
              visibilityMenu->AddItem(DICT("trailvisibilitymenu"), 0);
          auto trailInGameSubMenu =
              trailSubMenu->AddItem(DICT("ingamevisibility"), 0);
          trailInGameSubMenu->AddItem(
              DICT("defaultvisibility") +
                  (GetConfigValue("ShowInGameTrails") == 1 ? " [x]" : " [ ]"),
              Menu_TrailVisibility_InGameMap_Default);
          trailInGameSubMenu->AddItem(
              DICT("forceonvisibility") +
                  (GetConfigValue("ShowInGameTrails") == 2 ? " [x]" : " [ ]"),
              Menu_TrailVisibility_InGameMap_Force);
          trailInGameSubMenu->AddItem(
              DICT("forceoffvisibility") +
                  (GetConfigValue("ShowInGameTrails") == 0 ? " [x]" : " [ ]"),
              Menu_TrailVisibility_InGameMap_Off);
          auto trailMiniMapSubMenu =
              trailSubMenu->AddItem(DICT("minimapvisibility"), 0);
          trailMiniMapSubMenu->AddItem(
              DICT("defaultvisibility") +
                  (GetConfigValue("ShowMinimapTrails") == 1 ? " [x]" : " [ ]"),
              Menu_TrailVisibility_MiniMap_Default);
          trailMiniMapSubMenu->AddItem(
              DICT("forceonvisibility") +
                  (GetConfigValue("ShowMinimapTrails") == 2 ? " [x]" : " [ ]"),
              Menu_TrailVisibility_MiniMap_Force);
          trailMiniMapSubMenu->AddItem(
              DICT("forceoffvisibility") +
                  (GetConfigValue("ShowMinimapTrails") == 0 ? " [x]" : " [ ]"),
              Menu_TrailVisibility_MiniMap_Off);
          auto trailMapSubMenu =
              trailSubMenu->AddItem(DICT("mapvisibility"), 0);
          trailMapSubMenu->AddItem(
              DICT("defaultvisibility") +
                  (GetConfigValue("ShowBigmapTrails") == 1 ? " [x]" : " [ ]"),
              Menu_TrailVisibility_BigMap_Default);
          trailMapSubMenu->AddItem(
              DICT("forceonvisibility") +
                  (GetConfigValue("ShowBigmapTrails") == 2 ? " [x]" : " [ ]"),
              Menu_TrailVisibility_BigMap_Force);
          trailMapSubMenu->AddItem(
              DICT("forceoffvisibility") +
                  (GetConfigValue("ShowBigmapTrails") == 0 ? " [x]" : " [ ]"),
              Menu_TrailVisibility_BigMap_Off);

          auto utils = ctx->AddItem(DICT("tacticalutilities"), 0);
          utils->AddItem(DICT("reloadmarkers"), Menu_ReloadMarkers);
          utils->AddItem(DICT("removemymarkers"), 0)
              ->AddItem(DICT("reallyremovemarkers"), Menu_DeleteMyMarkers);
        }
        ctx->AddItem(
            DICT("toggletactical") +
                (GetConfigValue("TacticalLayerVisible") ? " [x]" : " [ ]") +
                GetKeybindString(TacOKeyAction::Toggle_tactical_layer),
            Menu_ToggleTactical);

        ctx->AddSeparator();

        if (mumbleLink.isPvp) {
          ctx->AddItem(DICT("rangecirclesnotavailable"), 0);
        } else {
          ctx->AddItem(
              DICT("togglerangecircles") +
                  (GetConfigValue("RangeCirclesVisible") ? " [x]" : " [ ]") +
                  GetKeybindString(TacOKeyAction::Toggle_range_circles),
              Menu_ToggleRangeCircles);
          if (GetConfigValue("RangeCirclesVisible")) {
            auto trns = ctx->AddItem(DICT("rangevisibility"), 0);
            trns->AddItem("40%", Menu_RangeCircleTransparency40);
            trns->AddItem("60%", Menu_RangeCircleTransparency60);
            trns->AddItem("100%", Menu_RangeCircleTransparency100);
            auto ranges = ctx->AddItem(DICT("toggleranges"), 0);
            ranges->AddItem(
                GetConfigValue("RangeCircle90") ? "90 [x]" : "90 [ ]",
                Menu_ToggleRangeCircle90, false, false);
            ranges->AddItem(
                GetConfigValue("RangeCircle120") ? "120 [x]" : "120 [ ]",
                Menu_ToggleRangeCircle120, false, false);
            ranges->AddItem(
                GetConfigValue("RangeCircle180") ? "180 [x]" : "180 [ ]",
                Menu_ToggleRangeCircle180, false, false);
            ranges->AddItem(
                GetConfigValue("RangeCircle240") ? "240 [x]" : "240 [ ]",
                Menu_ToggleRangeCircle240, false, false);
            ranges->AddItem(
                GetConfigValue("RangeCircle300") ? "300 [x]" : "300 [ ]",
                Menu_ToggleRangeCircle300, false, false);
            ranges->AddItem(
                GetConfigValue("RangeCircle400") ? "400 [x]" : "400 [ ]",
                Menu_ToggleRangeCircle400, false, false);
            ranges->AddItem(
                GetConfigValue("RangeCircle600") ? "600 [x]" : "600 [ ]",
                Menu_ToggleRangeCircle600, false, false);
            ranges->AddItem(
                GetConfigValue("RangeCircle900") ? "900 [x]" : "900 [ ]",
                Menu_ToggleRangeCircle900, false, false);
            ranges->AddItem(
                GetConfigValue("RangeCircle1200") ? "1200 [x]" : "1200 [ ]",
                Menu_ToggleRangeCircle1200, false, false);
            ranges->AddItem(
                GetConfigValue("RangeCircle1500") ? "1500 [x]" : "1500 [ ]",
                Menu_ToggleRangeCircle1500, false, false);
            ranges->AddItem(
                GetConfigValue("RangeCircle1600") ? "1600 [x]" : "1600 [ ]",
                Menu_ToggleRangeCircle1600, false, false);
          }
        }

        ctx->AddSeparator();

        ctx->AddItem(
            DICT("togglecompass") +
                (GetConfigValue("TacticalCompassVisible") ? " [x]" : " [ ]") +
                GetKeybindString(TacOKeyAction::Toggle_tactical_compass),
            Menu_ToggleTacticalCompass);
        ctx->AddItem(
            DICT("toggleloctimers") +
                (GetConfigValue("LocationalTimersVisible") ? " [x]" : " [ ]") +
                GetKeybindString(TacOKeyAction::Toggle_locational_timers),
            Menu_ToggleLocationalTimers);
        ctx->AddItem(DICT("togglehpgrid") +
                         (GetConfigValue("HPGridVisible") ? " [x]" : " [ ]") +
                         GetKeybindString(TacOKeyAction::Toggle_hp_grids),
                     Menu_ToggleHPGrid);
        // ctx->AddItem( GetConfigValue( "Vsync" ) ? "Toggle TacO Vsync [x]" :
        // "Toggle TacO Vsync [ ]", Menu_ToggleVsync );
        ctx->AddSeparator();
        ctx->AddItem(
            DICT("togglemousehighlight") +
                (GetConfigValue("MouseHighlightVisible") ? " [x]" : " [ ]") +
                GetKeybindString(TacOKeyAction::Toggle_mouse_highlight),
            Menu_ToggleHighLight);
        if (GetConfigValue("MouseHighlightVisible")) {
          ctx->AddItem(
              DICT("togglemouseoutline") +
                  (GetConfigValue("MouseHighlightOutline") ? " [x]" : " [ ]"),
              Menu_ToggleMouseHighlightOutline);
          auto cols = ctx->AddItem(DICT("mousecolor"), 0);

          int mouseColor = 0;
          if (HasConfigValue("MouseHighlightColor")) {
            mouseColor = GetConfigValue("MouseHighlightColor");
          }

          for (int x = 0; x < CGAPaletteNames.size(); x++) {
            if (mouseColor == x) {
              cols->AddItem(("[x] " + DICT(CGAPaletteNames[x])),
                            Menu_MouseHighlightColor0 + x, true);
            } else {
              cols->AddItem(("[ ] " + DICT(CGAPaletteNames[x])),
                            Menu_MouseHighlightColor0 + x, false);
            }
          }
        }
        ctx->AddSeparator();
        // auto it = ctx->AddItem("Windows", 0);
        ctx->AddItem((IsWindowOpen("MapTimer") ? DICT("closemaptimer")
                                               : DICT("openmaptimer")) +
                         GetKeybindString(TacOKeyAction::Toggle_map_timer),
                     Menu_ToggleMapTimer);
        if (IsWindowOpen("MapTimer")) {
          ctx->AddItem(
              DICT("compactmaptimer") +
                  (GetConfigValue("MapTimerCompact") ? " [x]" : " [ ]"),
              Menu_ToggleCompactMapTimer);
          ctx->AddItem(
              DICT("maptimercategories") +
                  (GetConfigValue("MapTimerCategories") ? " [x]" : " [ ]"),
              Menu_ToggleMapTimerCategories);

          auto* timer = dynamic_cast<GW2MapTimer*>(
              App->GetRoot()->FindChildByID("MapTimer", "maptimer"));

          if (timer) {
            auto itm = ctx->AddItem(DICT("configmaptimer"), 0);

            for (size_t x = 0; x < timer->maps.size(); x++) {
              bool open = true;
              auto str = "maptimer_mapopen_" + timer->maps[x].id;

              if (HasConfigValue(str)) open = GetConfigValue(str);

              itm->AddItem(open ? (timer->maps[x].name + " [x]")
                                : (timer->maps[x].name + " [ ]"),
                           Menu_ToggleMapTimerMap + x, open, false);
            }
          }
        }
        ctx->AddSeparator();
        ctx->AddItem((IsWindowOpen("TS3Control") ? DICT("closetswindow")
                                                 : DICT("opentswindow")) +
                         GetKeybindString(TacOKeyAction::Toggle_ts3_window),
                     Menu_ToggleTS3Control);
        auto markerEditor = ctx->AddItem(
            (IsWindowOpen("MarkerEditor") ? DICT("closemarkereditor")
                                          : DICT("openmarkereditor")) +
                GetKeybindString(TacOKeyAction::Toggle_marker_editor),
            Menu_ToggleMarkerEditor);
        if (IsWindowOpen("MarkerEditor")) {
          markerEditor->AddItem(
              DICT("autohidemarkereditor") +
                  (GetConfigValue("AutoHideMarkerEditor") ? " [x]" : " [ ]"),
              Menu_ToggleAutoHideMarkerEditor);
          markerEditor->AddSeparator();
          int cnt = 1;
          for (int32_t x = 1; x < ActionNames.size(); x++) {
            auto str = DICT(ActionNames[x]) + " " + DICT("action_no_key_bound");
            for (auto& kb : KeyBindings) {
              if (static_cast<int32_t>(kb.second) == x) {
                str = DICT(ActionNames[x]) + std::format(" [{:c}]", kb.first);
                break;
              }
            }

            if (ActionNames[x][0] == '*') {
              markerEditor->AddItem(str, Menu_RebindKey_Base + x);
            }
            cnt++;
          }
        }

        ctx->AddItem((IsWindowOpen("Notepad") ? DICT("closenotepad")
                                              : DICT("opennotepad")) +
                         GetKeybindString(TacOKeyAction::Toggle_notepad),
                     Menu_ToggleNotepad);
        ctx->AddSeparator();

        auto raid = ctx->AddItem(
            (IsWindowOpen("RaidProgress") ? DICT("closeraidprogress")
                                          : DICT("openraidprogress")) +
                GetKeybindString(TacOKeyAction::Toggle_raid_progress),
            Menu_ToggleRaidProgress);

        if (IsWindowOpen("RaidProgress")) {
          raid->AddItem(
              DICT("raidwindow_compact") +
                  (GetConfigValue("CompactRaidWindow") ? " [x]" : " [ ]"),
              Menu_ToggleCompactRaids);
          auto* rp = FindChildByID<RaidProgress>("RaidProgressView");
          if (rp) {
            auto& raids = rp->GetRaids();
            if (!raids.empty()) raid->AddSeparator();
            for (int32_t x = 0; x < raids.size(); x++) {
              auto& r = raids[x];
              raid->AddItem(((HasConfigValue(r.configName) &&
                              !GetConfigValue(r.configName))
                                 ? "[ ] "
                                 : "[x] ") +
                                DICT(r.configName, r.name),
                            Menu_RaidToggles + x, false, false);
            }
          }
        }

        ctx->AddItem(
            (IsWindowOpen("DungeonProgress") ? DICT("closedungeonprogress")
                                             : DICT("opendungeonprogress")) +
                GetKeybindString(TacOKeyAction::Toggle_dungeon_progress),
            Menu_ToggleDungeonProgress);
        auto tpTracker =
            ctx->AddItem((IsWindowOpen("TPTracker") ? DICT("closetptracker")
                                                    : DICT("opentptracker")) +
                             GetKeybindString(TacOKeyAction::Toggle_tp_tracker),
                         Menu_ToggleTPTracker);
        if (IsWindowOpen("TPTracker")) {
          tpTracker->AddItem(
              DICT("tptracker_onlyoutbid") +
                  (GetConfigValue("TPTrackerOnlyShowOutbid") ? " [x]" : " [ ]"),
              Menu_ToggleTPTracker_OnlyOutbid);
          tpTracker->AddItem(
              DICT("tptracker_showbuys") +
                  (GetConfigValue("TPTrackerShowBuys") ? " [x]" : " [ ]"),
              Menu_ToggleTPTracker_ShowBuys);
          tpTracker->AddItem(
              DICT("tptracker_showsells") +
                  (GetConfigValue("TPTrackerShowSells") ? " [x]" : " [ ]"),
              Menu_ToggleTPTracker_ShowSells);
          tpTracker->AddItem(
              DICT("tptracker_nextsellonly") +
                  (GetConfigValue("TPTrackerNextSellOnly") ? " [x]" : " [ ]"),
              Menu_ToggleTPTracker_OnlyNextFulfilled);
        }
        ctx->AddSeparator();

        auto settings = ctx->AddItem(DICT("tacosettings"), Menu_TacOSettings);
        settings->AddItem(
            DICT("togglewindoweditmode") +
                (GetConfigValue("EditMode") ? " [x]" : " [ ]") +
                GetKeybindString(TacOKeyAction::Toggle_window_edit_mode),
            Menu_ToggleEditMode);
        settings->AddSeparator();

        settings->AddItem(
            DICT("hideonload") +
                (GetConfigValue("HideOnLoadingScreens") ? " [x]" : " [ ]"),
            Menu_HideOnLoadingScreens);
        settings->AddItem(
            DICT("closewithgw2") +
                (GetConfigValue("CloseWithGW2") ? " [x]" : " [ ]"),
            Menu_ToggleGW2ExitMode);
        settings->AddItem(
            DICT("toggleinfoline") +
                (GetConfigValue("InfoLineVisible") ? " [x]" : " [ ]"),
            Menu_ToggleInfoLine);
        settings->AddItem(
            DICT("toggleforcedpiaware") +
                (GetConfigValue("ForceDPIAware") ? " [x]" : " [ ]"),
            Menu_ToggleForceDPIAware);
        settings->AddItem(
            DICT("enabletpnotificationicon") +
                (GetConfigValue("EnableTPNotificationIcon") ? " [x]" : " [ ]"),
            Menu_ToggleShowNotificationIcon);

        settings->AddSeparator();
        settings->AddItem(
            DICT("togglekeybinds") +
                (GetConfigValue("KeybindsEnabled") ? " [x]" : " [ ]"),
            Menu_KeyBindsEnabled);
        auto bind = settings->AddItem(DICT("rebindkeys"), 0);
        int cnt = 1;
        for (int32_t x = 1; x < ActionNames.size(); x++) {
          auto str = DICT(ActionNames[x]) + " " + DICT("action_no_key_bound");
          for (auto& kb : KeyBindings) {
            if (static_cast<int32_t>(kb.second) == x) {
              str = DICT(ActionNames[x]) + std::format(" [{:c}]", kb.first);
              break;
            }
          }

          if (ActionNames[x][0] != '*') {
            bind->AddItem(str, Menu_RebindKey_Base + x);
          }
          cnt++;
        }
        settings->AddSeparator();

        auto apiKeys = settings->AddItem(DICT("apikeys"), 0);
        auto gw2keys = apiKeys->AddItem(DICT("gw2apikey"), 0);

        auto currKey = GW2::apiKeyManager.GetIdentifiedAPIKey();

        for (int32_t x = 0; x < GW2::apiKeyManager.size(); x++) {
          auto key = GW2::apiKeyManager.GetKey(x);
          auto keyMenu = gw2keys->AddItem(
              (!key->accountName.empty()) ? key->accountName : key->apiKey,
              Menu_GW2APIKey_Base + x, key == currKey);
          keyMenu->AddItem(DICT("deletekey"), Menu_DeleteGW2APIKey_Base + x);
        }

        if (!GW2::apiKeyManager.empty()) {
          gw2keys->AddSeparator();
        }

        gw2keys->AddItem(DICT("addgw2apikey"), Menu_AddGW2ApiKey);

        apiKeys->AddItem(DICT("ts3controlplugin"), Menu_TS3APIKey);

        settings->AddSeparator();

        auto languages = localization->GetLanguages();
        auto langs = settings->AddItem(DICT("language"), Menu_Language);
        for (int x = 0; x < languages.size(); x++) {
          langs->AddItem(
              (x == localization->GetActiveLanguageIndex() ? "[x] " : "[ ] ") +
                  languages[x],
              Menu_Language_Base + x,
              x == localization->GetActiveLanguageIndex());
        }

        ctx->AddSeparator();
        ctx->AddItem(DICT("abouttaco"), Menu_About);
        ctx->AddSeparator();
        ctx->AddItem(DICT("exittaco"), Menu_Exit);
        return true;
      }
      if (b->GetID() == "GoToWebsite") {
        ShellExecute(App->GetHandle(), "open",
                     "https://github.com/therjak/GW2TacO/", nullptr, nullptr,
                     SW_SHOW);
        return true;
      }
    } break;
    case WBM_REBUILDCONTEXTITEM:

      if (Message.Data >= Menu_RaidToggles &&
          Message.Data < Menu_RaidToggles_End) {
        int32_t raidToggle = Message.Data - Menu_RaidToggles;

        auto* rp = FindChildByID<RaidProgress>("RaidProgressView");
        if (rp) {
          auto& raids = rp->GetRaids();
          if (raidToggle < raids.size()) {
            auto* ctxMenu = dynamic_cast<CWBContextMenu*>(
                App->FindItemByGuid(Message.Position[1]));
            auto itm = ctxMenu->GetItem(Message.Data);
            auto& r = raids[raidToggle];
            itm->SetText(
                ((HasConfigValue(r.configName) && !GetConfigValue(r.configName))
                     ? "[ ] "
                     : "[x] ") +
                DICT(r.configName, r.name));
          }
        }
      }

      if (Message.Data >= Menu_MarkerFilter_Base &&
          Message.Data < Menu_MarkerFilter_Base + CategoryList.size()) {
        auto* ctxMenu = dynamic_cast<CWBContextMenu*>(
            App->FindItemByGuid(Message.Position[1]));
        if (ctxMenu) {
          auto itm = ctxMenu->GetItem(Message.Data);

          auto& dta = CategoryList[Message.Data - Menu_MarkerFilter_Base];

          if (!dta->IsOnlySeparator) {
            auto txt = "[" + std::string(dta->IsDisplayed ? "x" : " ") + "] ";
            if (!dta->displayName.empty()) {
              txt += dta->displayName;
            } else {
              txt += dta->name;
            }

            itm->SetText(txt);
            itm->SetHighlight(dta->IsDisplayed);
          }
        }
        break;
      }
      if (Message.Data >= Menu_ToggleMapTimerMap) {
        auto* ctxMenu = dynamic_cast<CWBContextMenu*>(
            App->FindItemByGuid(Message.Position[1]));
        auto itm = ctxMenu->GetItem(Message.Data);
        int32_t mapIdx = Message.Data - Menu_ToggleMapTimerMap;

        auto* timer = dynamic_cast<GW2MapTimer*>(
            App->GetRoot()->FindChildByID("MapTimer", "maptimer"));
        if (!timer) break;

        bool open = true;
        auto str = "maptimer_mapopen_" + timer->maps[mapIdx].id;

        if (HasConfigValue(str)) open = GetConfigValue(str);

        itm->SetText(open ? (timer->maps[mapIdx].name + " [x]")
                          : (timer->maps[mapIdx].name + " [ ]"));
        itm->SetHighlight(open);
        break;
      }

      {
        auto* ctxMenu = dynamic_cast<CWBContextMenu*>(
            App->FindItemByGuid(Message.Position[1]));
        auto itm = ctxMenu->GetItem(Message.Data);

        switch (Message.Data) {
          case Menu_ToggleRangeCircle90:
            if (itm) {
              itm->SetText(GetConfigValue("RangeCircle90") ? "90 [x]"
                                                           : "90 [ ]");
            }
            break;
          case Menu_ToggleRangeCircle120:
            if (itm) {
              itm->SetText(GetConfigValue("RangeCircle120") ? "120 [x]"
                                                            : "120 [ ]");
            }
            break;
          case Menu_ToggleRangeCircle180:
            if (itm) {
              itm->SetText(GetConfigValue("RangeCircle180") ? "180 [x]"
                                                            : "180 [ ]");
            }
            break;
          case Menu_ToggleRangeCircle240:
            if (itm) {
              itm->SetText(GetConfigValue("RangeCircle240") ? "240 [x]"
                                                            : "240 [ ]");
            }
            break;
          case Menu_ToggleRangeCircle300:
            if (itm) {
              itm->SetText(GetConfigValue("RangeCircle300") ? "300 [x]"
                                                            : "300 [ ]");
            }
            break;
          case Menu_ToggleRangeCircle400:
            if (itm) {
              itm->SetText(GetConfigValue("RangeCircle400") ? "400 [x]"
                                                            : "400 [ ]");
            }
            break;
          case Menu_ToggleRangeCircle600:
            if (itm) {
              itm->SetText(GetConfigValue("RangeCircle600") ? "600 [x]"
                                                            : "600 [ ]");
            }
            break;
          case Menu_ToggleRangeCircle900:
            if (itm) {
              itm->SetText(GetConfigValue("RangeCircle900") ? "900 [x]"
                                                            : "900 [ ]");
            }
            break;
          case Menu_ToggleRangeCircle1200:
            if (itm) {
              itm->SetText(GetConfigValue("RangeCircle1200") ? "1200 [x]"
                                                             : "1200 [ ]");
            }
            break;
          case Menu_ToggleRangeCircle1500:
            if (itm) {
              itm->SetText(GetConfigValue("RangeCircle1500") ? "1500 [x]"
                                                             : "1500 [ ]");
            }
            break;
          case Menu_ToggleRangeCircle1600:
            if (itm) {
              itm->SetText(GetConfigValue("RangeCircle1600") ? "1600 [x]"
                                                             : "1600 [ ]");
            }
            break;
        }
      }

      break;

    case WBM_CONTEXTMESSAGE:
      if (Message.Data >= Menu_GW2APIKey_Base &&
          Message.Data < Menu_GW2APIKey_End) {
        int32_t idx = Message.Data - Menu_GW2APIKey_Base;
        ApiKeyInputAction(APIKeys::GW2APIKey, idx);
        return true;
      }

      if (Message.Data >= Menu_DeleteGW2APIKey_Base &&
          Message.Data < Menu_DeleteGW2APIKey_End) {
        int32_t idx = Message.Data - Menu_DeleteGW2APIKey_Base;
        GW2::apiKeyManager.RemoveKey(idx);
        GW2::apiKeyManager.RebuildConfigValues();
        return true;
      }

      if (Message.Data >= Menu_RaidToggles &&
          Message.Data < Menu_RaidToggles_End) {
        int32_t raidToggle = Message.Data - Menu_RaidToggles;

        auto* rp = FindChildByID<RaidProgress>("RaidProgressView");
        if (rp) {
          auto& raids = rp->GetRaids();
          if (raidToggle < raids.size()) {
            if (!HasConfigValue(raids[raidToggle].configName)) {
              SetConfigValue(raids[raidToggle].configName, 0);
            } else {
              ToggleConfigValue(raids[raidToggle].configName);
            }
          }
        }
        break;
      }

      if (Message.Data >= Menu_RebindKey_Base &&
          Message.Data < Menu_RebindKey_Base + ActionNames.size()) {
        RebindAction(
            static_cast<TacOKeyAction>(Message.Data - Menu_RebindKey_Base));
        break;
      }

      {
        auto languages = localization->GetLanguages();

        if (Message.Data >= Menu_Language_Base &&
            Message.Data < Menu_Language_Base + languages.size()) {
          localization->SetActiveLanguage(
              languages[Message.Data - Menu_Language_Base]);
          break;
        }
      }

      if (Message.Data >= Menu_MarkerFilter_Base &&
          Message.Data < Menu_MarkerFilter_Base + CategoryList.size()) {
        bool displayed =
            !CategoryList[Message.Data - Menu_MarkerFilter_Base]->IsDisplayed;
        CategoryList[Message.Data - Menu_MarkerFilter_Base]->IsDisplayed =
            displayed;
        SetConfigValue(("CategoryVisible_" +
                        CategoryList[Message.Data - Menu_MarkerFilter_Base]
                            ->GetFullTypeName()),
                       displayed);
        CategoryRoot.CalculateVisibilityCache();
        break;
      }

      if (Message.Data >= Menu_ToggleMapTimerMap) {
        auto* timer = dynamic_cast<GW2MapTimer*>(
            App->GetRoot()->FindChildByID("MapTimer", "maptimer"));

        if (timer) {
          if (Message.Data < Menu_ToggleMapTimerMap + timer->maps.size()) {
            int32_t mapIdx = Message.Data - Menu_ToggleMapTimerMap;
            auto str = "maptimer_mapopen_" + timer->maps[mapIdx].id;
            timer->maps[mapIdx].display = !timer->maps[mapIdx].display;
            SetConfigValue(str, timer->maps[mapIdx].display);
            break;
          }
        }
      }

      switch (Message.Data) {
        case Menu_Exit:
          GetApplication()->SetDone(true);
          return true;
        case Menu_About:
          OpenAboutWindow();
          return true;
        case Menu_ToggleInfoLine:
          ToggleConfigValue("InfoLineVisible");
          return true;
        case Menu_ToggleHighLight:
          ToggleConfigValue("MouseHighlightVisible");
          return true;
        case Menu_ToggleTactical:
          ToggleConfigValue("TacticalLayerVisible");
          return true;
        case Menu_ToggleTacticalsOnEdge:
          ToggleConfigValue("TacticalIconsOnEdge");
          return true;
        case Menu_ToggleDrawDistance:
          ToggleConfigValue("TacticalDrawDistance");
          return true;
        case Menu_DrawWvWNames:
          ToggleConfigValue("DrawWvWNames");
          return true;
        case Menu_ToggleLocationalTimers:
          ToggleConfigValue("LocationalTimersVisible");
          return true;
        case Menu_ToggleGW2ExitMode:
          ToggleConfigValue("CloseWithGW2");
          return true;
        case Menu_ToggleVersionCheck:
          ToggleConfigValue("CheckForUpdates");
          return true;
        case Menu_ToggleMapTimer:
          OpenWindow("MapTimer");
          return true;
        case Menu_ToggleTS3Control:
          OpenWindow("TS3Control");
          return true;
        case Menu_ToggleMarkerEditor:
          OpenWindow("MarkerEditor");
          return true;
        case Menu_ToggleNotepad:
          OpenWindow("Notepad");
          return true;
        case Menu_ToggleRaidProgress:
          OpenWindow("RaidProgress");
          return true;
        case Menu_ToggleDungeonProgress:
          OpenWindow("DungeonProgress");
          return true;
        case Menu_ToggleTPTracker:
          OpenWindow("TPTracker");
          return true;
        case Menu_ToggleEditMode:
          ToggleConfigValue("EditMode");
          return true;
        case Menu_HideOnLoadingScreens:
          ToggleConfigValue("HideOnLoadingScreens");
          return true;
          // case Menu_Interface_Small:
        // case Menu_Interface_Normal:
        // case Menu_Interface_Large:
        // case Menu_Interface_Larger:
        //  if ( App->LoadCSSFromFile( UIFileNames[ Message.Data -
        //  Menu_Interface_Small ], true ) )
        //    SetConfigValue( "InterfaceSize", Message.Data -
        //    Menu_Interface_Small );
        //  App->ReApplyStyle();
        //  return true;
        //  break;
        case Menu_DownloadNewBuild:
          ShellExecute(App->GetHandle(), "open", "http://www.gw2taco.com",
                       nullptr, nullptr, SW_SHOW);
          return true;
          break;
        case Menu_ToggleRangeCircles:
          ToggleConfigValue("RangeCirclesVisible");
          return true;
          break;
        case Menu_RangeCircleTransparency40:
          SetConfigValue("RangeCircleTransparency", 40);
          return true;
          break;
        case Menu_RangeCircleTransparency60:
          SetConfigValue("RangeCircleTransparency", 60);
          return true;
          break;
        case Menu_RangeCircleTransparency100:
          SetConfigValue("RangeCircleTransparency", 100);
          return true;
          break;
        case Menu_ToggleRangeCircle90:
          ToggleConfigValue("RangeCircle90");
          return true;
          break;
        case Menu_ToggleRangeCircle120:
          ToggleConfigValue("RangeCircle120");
          return true;
          break;
        case Menu_ToggleRangeCircle180:
          ToggleConfigValue("RangeCircle180");
          return true;
          break;
        case Menu_ToggleRangeCircle240:
          ToggleConfigValue("RangeCircle240");
          return true;
          break;
        case Menu_ToggleRangeCircle300:
          ToggleConfigValue("RangeCircle300");
          return true;
          break;
        case Menu_ToggleRangeCircle400:
          ToggleConfigValue("RangeCircle400");
          return true;
          break;
        case Menu_ToggleRangeCircle600:
          ToggleConfigValue("RangeCircle600");
          return true;
          break;
        case Menu_ToggleRangeCircle900:
          ToggleConfigValue("RangeCircle900");
          return true;
          break;
        case Menu_ToggleRangeCircle1200:
          ToggleConfigValue("RangeCircle1200");
          return true;
          break;
        case Menu_ToggleRangeCircle1500:
          ToggleConfigValue("RangeCircle1500");
          return true;
          break;
        case Menu_ToggleRangeCircle1600:
          ToggleConfigValue("RangeCircle1600");
          return true;
          break;
        case Menu_ToggleTacticalCompass:
          ToggleConfigValue("TacticalCompassVisible");
          return true;
        case Menu_ToggleVsync:
          ToggleConfigValue("Vsync");
          App->SetVSync(GetConfigValue("Vsync"));
          return true;
        case Menu_ToggleHPGrid:
          ToggleConfigValue("HPGridVisible");
          return true;
        case Menu_ToggleCompactMapTimer:
          ToggleConfigValue("MapTimerCompact");
          return true;
        case Menu_ToggleMapTimerCategories:
          ToggleConfigValue("MapTimerCategories");
          return true;
        case Menu_ToggleMouseHighlightOutline:
          ToggleConfigValue("MouseHighlightOutline");
          return true;
        case Menu_ToggleTPTracker_OnlyOutbid:
          ToggleConfigValue("TPTrackerOnlyShowOutbid");
          return true;
        case Menu_ToggleTPTracker_ShowBuys:
          ToggleConfigValue("TPTrackerShowBuys");
          return true;
        case Menu_ToggleTPTracker_ShowSells:
          ToggleConfigValue("TPTrackerShowSells");
          return true;
        case Menu_ToggleTPTracker_OnlyNextFulfilled:
          ToggleConfigValue("TPTrackerNextSellOnly");
          return true;
        case Menu_ToggleCompactRaids:
          ToggleConfigValue("CompactRaidWindow");
          return true;
        case Menu_TogglePOIInfoText:
          ToggleConfigValue("TacticalInfoTextVisible");
          return true;
        case Menu_MouseHighlightColor0:
        case Menu_MouseHighlightColor1:
        case Menu_MouseHighlightColor2:
        case Menu_MouseHighlightColor3:
        case Menu_MouseHighlightColor4:
        case Menu_MouseHighlightColor5:
        case Menu_MouseHighlightColor6:
        case Menu_MouseHighlightColor7:
        case Menu_MouseHighlightColor8:
        case Menu_MouseHighlightColor9:
        case Menu_MouseHighlightColora:
        case Menu_MouseHighlightColorb:
        case Menu_MouseHighlightColorc:
        case Menu_MouseHighlightColord:
        case Menu_MouseHighlightColore:
        case Menu_MouseHighlightColorf:
          SetConfigValue("MouseHighlightColor",
                         Message.Data - Menu_MouseHighlightColor0);
          return true;
        case Menu_TS3APIKey:
          ApiKeyInputAction(APIKeys::TS3APIKey, 0);
          return true;
        case Menu_ToggleTrailLogging:
          ToggleConfigValue("LogTrails");
          return true;

        case Menu_ToggleAutoHideMarkerEditor:
          ToggleConfigValue("AutoHideMarkerEditor");
          return true;
        case Menu_ToggleFadeoutBubble:
          ToggleConfigValue("FadeoutBubble");
          return true;
        case Menu_ToggleMetricSystem:
          ToggleConfigValue("UseMetricDisplay");
          return true;
        case Menu_ToggleForceDPIAware:
          ToggleConfigValue("ForceDPIAware");
          return true;
        case Menu_ToggleShowNotificationIcon:
          ToggleConfigValue("EnableTPNotificationIcon");
          return true;
        case Menu_MarkerVisibility_MiniMap_Off:
          SetConfigValue("ShowMinimapMarkers", 0);
          return true;
        case Menu_MarkerVisibility_MiniMap_Default:
          SetConfigValue("ShowMinimapMarkers", 1);
          return true;
        case Menu_MarkerVisibility_MiniMap_Force:
          SetConfigValue("ShowMinimapMarkers", 2);
          return true;
        case Menu_MarkerVisibility_BigMap_Off:
          SetConfigValue("ShowBigmapMarkers", 0);
          return true;
        case Menu_MarkerVisibility_BigMap_Default:
          SetConfigValue("ShowBigmapMarkers", 1);
          return true;
        case Menu_MarkerVisibility_BigMap_Force:
          SetConfigValue("ShowBigmapMarkers", 2);
          return true;
        case Menu_MarkerVisibility_InGameMap_Off:
          SetConfigValue("ShowInGameMarkers", 0);
          return true;
        case Menu_MarkerVisibility_InGameMap_Default:
          SetConfigValue("ShowInGameMarkers", 1);
          return true;
        case Menu_MarkerVisibility_InGameMap_Force:
          SetConfigValue("ShowInGameMarkers", 2);
          return true;
        case Menu_TrailVisibility_MiniMap_Off:
          SetConfigValue("ShowMinimapTrails", 0);
          return true;
        case Menu_TrailVisibility_MiniMap_Default:
          SetConfigValue("ShowMinimapTrails", 1);
          return true;
        case Menu_TrailVisibility_MiniMap_Force:
          SetConfigValue("ShowMinimapTrails", 2);
          return true;
        case Menu_TrailVisibility_BigMap_Off:
          SetConfigValue("ShowBigmapTrails", 0);
          return true;
        case Menu_TrailVisibility_BigMap_Default:
          SetConfigValue("ShowBigmapTrails", 1);
          return true;
        case Menu_TrailVisibility_BigMap_Force:
          SetConfigValue("ShowBigmapTrails", 2);
          return true;
        case Menu_TrailVisibility_InGameMap_Off:
          SetConfigValue("ShowInGameTrails", 0);
          return true;
        case Menu_TrailVisibility_InGameMap_Default:
          SetConfigValue("ShowInGameTrails", 1);
          return true;
        case Menu_TrailVisibility_InGameMap_Force:
          SetConfigValue("ShowInGameTrails", 2);
          return true;
        case Menu_ReloadMarkers:
          ImportPOIS(GetApplication());
          return true;

        case Menu_OpacityIngame_Solid:
          SetConfigValue("OpacityIngame", 0);
          return true;
        case Menu_OpacityIngame_Transparent:
          SetConfigValue("OpacityIngame", 1);
          return true;
        case Menu_OpacityIngame_Faded:
          SetConfigValue("OpacityIngame", 2);
          return true;
        case Menu_OpacityMap_Solid:
          SetConfigValue("OpacityMap", 0);
          return true;
        case Menu_OpacityMap_Transparent:
          SetConfigValue("OpacityMap", 1);
          return true;
        case Menu_OpacityMap_Faded:
          SetConfigValue("OpacityMap", 2);
          return true;
        case Menu_DeleteMyMarkers: {
          auto* tactical = dynamic_cast<GW2TacticalDisplay*>(
              GetApplication()->GetRoot()->FindChildByID("tactical",
                                                         "gw2tactical"));
          if (tactical) tactical->RemoveUserMarkersFromMap();
          return true;
        }
        case Menu_KeyBindsEnabled:
          ToggleConfigValue("KeybindsEnabled");
          return true;

        case Menu_AddGW2ApiKey: {
          GW2::apiKeyManager.AddKey(std::make_unique<GW2::APIKey>());
          ApiKeyInputAction(APIKeys::GW2APIKey, GW2::apiKeyManager.size() - 1);
          return true;
        } break;
        default:
          break;
      }
      break;
    case WBM_CHAR:
      if (RebindMode) {
        if (!ScriptRebindMode) {
          auto it = KeyBindings.begin();
          while (it != KeyBindings.end()) {
            if (it->second == ActionToRebind) {
              DeleteKeyBinding(it->first);
              it = KeyBindings.erase(it);
            } else {
              ++it;
            }
          }

          if (Message.Key != VK_ESCAPE) {
            KeyBindings[Message.Key] = ActionToRebind;
            SetKeyBinding(ActionToRebind, Message.Key);
          }
        }

        RebindMode = false;
        ScriptRebindMode = false;
        return true;
      }

      if (GetConfigValue("KeybindsEnabled") &&
          KeyBindings.find(Message.Key) != KeyBindings.end()) {
        switch (KeyBindings[Message.Key]) {
          case TacOKeyAction::AddPOI:
            AddPOI(App);
            return true;
          case TacOKeyAction::RemovePOI:
            DeletePOI();
            return true;
          case TacOKeyAction::ActivatePOI: {
            UpdatePOI();
            return true;
          }
          case TacOKeyAction::EditNotepad: {
            auto* d =
                dynamic_cast<GW2Notepad*>(FindChildByID("notepad", "notepad"));
            if (d) {
              d->StartEdit();
              return true;
            }
            return true;
          }
          case TacOKeyAction::StartTrailRec: {
            auto* startTrail = dynamic_cast<CWBButton*>(
                App->GetRoot()->FindChildByID("starttrail", "button"));
            auto* trails = dynamic_cast<GW2TrailDisplay*>(
                App->GetRoot()->FindChildByID("trail", "gw2Trails"));
            if (startTrail && trails) {
              // startTrail->Push( !startTrail->IsPushed() );
              App->SendMessage(
                  CWBMessage(App, WBM_COMMAND, startTrail->GetGuid()));
            }
          }
            return true;
          case TacOKeyAction::PauseTrailRec: {
            auto* pauseTrail = dynamic_cast<CWBButton*>(
                App->GetRoot()->FindChildByID("pausetrail", "button"));
            auto* trails = dynamic_cast<GW2TrailDisplay*>(
                App->GetRoot()->FindChildByID("trail", "gw2Trails"));
            if (pauseTrail && trails) {
              // pauseTrail->Push( !pauseTrail->IsPushed() );
              App->SendMessage(
                  CWBMessage(App, WBM_COMMAND, pauseTrail->GetGuid()));
            }
          }
            return true;
          case TacOKeyAction::DeleteLastTrailSegment: {
            auto* trails = dynamic_cast<GW2TrailDisplay*>(
                App->GetRoot()->FindChildByID("trail", "gw2Trails"));
            if (trails) trails->DeleteLastTrailSegment();
          }
            return true;
          case TacOKeyAction::ResumeTrailAndCreateNewSection: {
            auto* pauseTrail = dynamic_cast<CWBButton*>(
                App->GetRoot()->FindChildByID("startnewsection", "button"));
            auto* trails = dynamic_cast<GW2TrailDisplay*>(
                App->GetRoot()->FindChildByID("trail", "gw2Trails"));
            if (pauseTrail && !pauseTrail->IsHidden() && trails) {
              App->SendMessage(
                  CWBMessage(App, WBM_COMMAND, pauseTrail->GetGuid()));
            }
          }
            return true;
          case TacOKeyAction::Toggle_tactical_layer:
            ToggleConfigValue("TacticalLayerVisible");
            return true;
          case TacOKeyAction::Toggle_range_circles:
            ToggleConfigValue("RangeCirclesVisible");
            return true;
          case TacOKeyAction::Toggle_tactical_compass:
            ToggleConfigValue("TacticalCompassVisible");
            return true;
          case TacOKeyAction::Toggle_locational_timers:
            ToggleConfigValue("LocationalTimersVisible");
            return true;
          case TacOKeyAction::Toggle_hp_grids:
            ToggleConfigValue("HPGridVisible");
            return true;
          case TacOKeyAction::Toggle_mouse_highlight:
            ToggleConfigValue("MouseHighlightVisible");
            return true;
          case TacOKeyAction::Toggle_map_timer:
            OpenWindow("MapTimer");
            return true;
          case TacOKeyAction::Toggle_ts3_window:
            OpenWindow("TS3Control");
            return true;
          case TacOKeyAction::Toggle_marker_editor:
            OpenWindow("MarkerEditor");
            return true;
          case TacOKeyAction::Toggle_notepad:
            OpenWindow("Notepad");
            return true;
          case TacOKeyAction::Toggle_raid_progress:
            OpenWindow("RaidProgress");
            return true;
          case TacOKeyAction::Toggle_dungeon_progress:
            OpenWindow("DungeonProgress");
            return true;
          case TacOKeyAction::Toggle_tp_tracker:
            OpenWindow("TPTracker");
            return true;
          case TacOKeyAction::Toggle_window_edit_mode:
            ToggleConfigValue("EditMode");
            return true;
        }
      }

      if (ScriptKeyBindings.find(Message.Key) != ScriptKeyBindings.end()) {
        TriggerScriptEngineKeyEvent(ScriptKeyBindings[Message.Key]);
      }

      break;
    case WBM_FOCUSLOST:
      if (Message.GetTarget() == GetGuid()) {
        RebindMode = false;
        ScriptRebindMode = false;
      }
      if (APIKeyInput && Message.GetTarget() == APIKeyInput->GetGuid()) {
        ApiKeyInputMode = false;
        switch (ApiKeyToSet) {
          case APIKeys::None:
            break;
          case APIKeys::TS3APIKey:
            SetConfigString("TS3APIKey", APIKeyInput->GetText());
            break;
          case APIKeys::GW2APIKey: {
            auto key = GW2::apiKeyManager.GetKey(ApiKeyIndex);
            key->SetKey(APIKeyInput->GetText());
            key->FetchData();
            GW2::apiKeyManager.RebuildConfigValues();
          } break;
          default:
            break;
        }
        APIKeyInput->MarkForDeletion();
        APIKeyInput = nullptr;
        return true;
      }
      break;
    default:
      break;
  }

  return CWBItem::MessageProc(Message);
}

void GW2TacO::SetInfoLine(std::string_view string) { lastInfoLine = string; }

void GW2TacO::SetMouseToolTip(std::string_view toolTip) {
  mouseToolTip = toolTip;
}

void GW2TacO::InitScriptEngines() {}

void GW2TacO::TickScriptEngine() {}

void GW2TacO::TriggerScriptEngineAction(GUID& guid) {}

void GW2TacO::TriggerScriptEngineKeyEvent(std::string_view eventID) {}

void GW2TacO::OpenAboutWindow() {
  auto child = FindChildByID("About", "window");
  if (child) return;

  CPoint cl = GetClientRect().Center();

  auto w = CWBWindow::Create(
      this, CRect(cl - CPoint(180, 160), cl + CPoint(180, 50 + 26)),
      "About GW2 TacO");
  w->SetID("About");

  w->ReapplyStyles();

  auto l1 = CWBLabel::Create(w, w->GetClientRect() + CPoint(0, 2),
                             "GW2 TacO - The Guild Wars 2 Tactical Overlay");
  l1->ApplyStyleDeclarations(
      "font-family:ProFont;text-align:center;vertical-align:top;");
  l1 = CWBLabel::Create(w, w->GetClientRect() + CPoint(0, 16),
                        "Build " + TacOBuild + " built on " + buildDateTime);
  l1->ApplyStyleDeclarations(
      "font-family:ProFont;text-align:center;vertical-align:top;");
  l1 = CWBLabel::Create(w, w->GetClientRect() + CPoint(0, 32),
                        "(c) BoyC / Conspiracy");
  l1->ApplyStyleDeclarations(
      "font-family:ProFont;text-align:center;vertical-align:top;");
  l1 = CWBLabel::Create(w, w->GetClientRect() + CPoint(0, 48),
                        "Taco Icon from http://icons8.com");
  l1->ApplyStyleDeclarations(
      "font-family:ProFont;text-align:center;vertical-align:top;");
  l1 = CWBLabel::Create(
      w, w->GetClientRect() + CPoint(0, 64),
      "If you like TacO, send some Mystic Coins to BoyC.2653 :)");
  l1->ApplyStyleDeclarations(
      "font-family:ProFont;text-align:center;vertical-align:top;");

  auto TacoIcon = CWBButton::Create(
      w, CRect(-50, -40 + 16, 50, 72 + 16) + w->GetClientRect().Center());
  TacoIcon->ApplyStyleDeclarations(
      "background-color:none;background: skin(TacoIcon) center middle;");

  int32_t width = w->GetClientRect().Width();
  int32_t height = w->GetClientRect().Height();

  auto WebsiteButton = CWBButton::Create(
      w, CRect(3, height - 25, width / 2 - 1, height - 3), "WebSite");
  WebsiteButton->SetID("GoToWebsite");
  WebsiteButton->ApplyStyleDeclarations("font-family:ProFont;");
}

float GetWindowTooSmallScale() {
  extern std::unique_ptr<CWBApplication> App;

  if (!App || !App->GetRoot()) return 1.0f;

  CRect rect = App->GetRoot()->GetClientRect();

  if (rect.Width() < 1024 || rect.Height() < 768) {
    float xScale = rect.Width() / 1024.0f;
    float yScale = rect.Height() / 768.0f;
    return std::min(xScale, yScale);
  }

  return 1.0f;
}

void GW2TacO::OnDraw(CWBDrawAPI* API) {
  mouseToolTip = "";

  float windowTooSmallScale = GetWindowTooSmallScale();
  if (windowTooSmallScale != lastScaleValue || scaleCountDownHack == 0) {
    StoreIconSizes();
    AdjustMenuForWindowTooSmallScale(windowTooSmallScale);
    lastScaleValue = windowTooSmallScale;
  }
  scaleCountDownHack--;

  if (IsWindowOpen("TS3Control")) {
    teamSpeakConnection.Tick();
  }
  CheckItemPickup();

  auto it = FindChildByID("MenuHoverBox");
  if (it) {
    auto taco = dynamic_cast<CWBButton*>(FindChildByID("MenuButton", "button"));
    if (taco) {
#define speed 500.0f

      int32_t currTime = GetTime();
      float delta = std::max(
          0.f, std::min(1.f, (currTime - lastMenuHoverTransitionTime) / speed));

      bool hover =
          ClientToScreen(it->GetClientRect()).Contains(App->GetMousePos());

      if (App->GetRoot()->FindChildByID("TacOMenu", "contextmenu")) {
        hover = true;
        taco->Push(true);
      } else {
        taco->Push(false);
      }

      if (hover != menuHoverLastFrame) {
        lastMenuHoverTransitionTime =
            static_cast<int32_t>(currTime - (1 - delta) * speed);
        delta = 1 - delta;
      }

      float col = 1 - delta * 0.5f;
      if (hover) col = 0.5f + delta * 0.5f;

      uint32_t o =
          static_cast<uint32_t>(std::max(0.f, std::min(255.f, col * 255.f)));

      taco->SetDisplayProperty(WB_STATE_NORMAL, WB_ITEM_OPACITY,
                               o * 0x01010101);
      taco->SetDisplayProperty(WB_STATE_ACTIVE, WB_ITEM_OPACITY,
                               o * 0x01010101);
      taco->SetDisplayProperty(WB_STATE_HOVER, WB_ITEM_OPACITY, o * 0x01010101);

      menuHoverLastFrame = hover;
    }
  }

  auto tpFlairButton = FindChildByID("RedCircle");
  if (tpFlairButton && showPickupHighlight &&
      GetConfigValue("EnableTPNotificationIcon")) {
    CRect r = tpFlairButton->ClientToScreen(tpFlairButton->GetClientRect());
    auto& dd = tpFlairButton->GetDisplayDescriptor();
    auto skin = dd.GetSkin(WB_STATE_NORMAL, WB_ITEM_BACKGROUNDIMAGE);
    CWBSkinElement* e = App->GetSkin()->GetElement(skin);
    if (e) {
      API->DrawAtlasElementRotated(e->GetHandle(), r, CColor{0x80ffffff},
                                   GetTime() / 1000.0f);
      API->DrawAtlasElementRotated(e->GetHandle(), r, CColor{0x80ffffff},
                                   -GetTime() / 1000.0f);
    }
  }

  int ypos = 0;

  if (GetConfigValue("InfoLineVisible")) {
    auto font = App->GetFont("ProFontOutlined");
    if (!font) return;

    auto infoline = lastInfoLine;

    if (lastInfoLine.empty()) {
      infoline = std::format(
          "map: {:d} "
          "world: {:d} "
          "shard: {:d} "
          "position: {:f} {:f} {:f} "
          "campos: {:.2f} {:.2f} {:.2f} "
          "game fps: {:.2f} "
          "overlay fps: {:.2f} "
          "map:{:d} "
          "compPos:{:d} "
          "compRot:{:d} "
          "cW:{:d} cH:{:d} cR:{:f} "
          "pX::{:f} pY:{:f} "
          "mcX:{:f} mcY:{:f} mS:{:f}",
          mumbleLink.mapID, mumbleLink.worldID, mumbleLink.mapInstance,
          mumbleLink.charPosition.x, mumbleLink.charPosition.y,
          mumbleLink.charPosition.z, mumbleLink.camDir.x, mumbleLink.camDir.y,
          mumbleLink.camDir.z, mumbleLink.GetFrameRate(), App->GetFrameRate(),
          static_cast<int>(mumbleLink.isMapOpen),
          static_cast<int>(mumbleLink.isMinimapTopRight),
          static_cast<int>(mumbleLink.isMinimapRotating),
          (mumbleLink.miniMap.compassWidth), (mumbleLink.miniMap.compassHeight),
          mumbleLink.miniMap.compassRotation, mumbleLink.miniMap.playerX,
          mumbleLink.miniMap.playerY, mumbleLink.miniMap.mapCenterX,
          mumbleLink.miniMap.mapCenterY, mumbleLink.miniMap.mapScale);

      if (GetConfigValue("CircleCalc_enabled")) {
        CVector3 minvals;
        CVector3 maxvals;
        bool initialized = false;
        auto& mPOIs = GetMapPOIs();
        for (auto& poi : mPOIs) {
          auto& p = poi.second;

          if (p.mapID == mumbleLink.mapID && !p.category) {
            if (!initialized) {
              minvals = maxvals = p.position;
              initialized = true;
            } else {
              minvals.x = std::min(p.position.x, minvals.x);
              minvals.y = std::min(p.position.y, minvals.y);
              minvals.z = std::min(p.position.z, minvals.z);
              maxvals.x = std::max(p.position.x, maxvals.x);
              maxvals.y = std::max(p.position.y, maxvals.y);
              maxvals.z = std::max(p.position.z, maxvals.z);
            }
          }
        }

        float maxdistance2d = 0;
        float maxdistance3d = 0;
        CVector3 center = (maxvals + minvals) * 0.5f;

        for (auto& poi : mPOIs) {
          auto& p = poi.second;

          if (p.mapID == mumbleLink.mapID && !p.category) {
            CVector3 d = p.position - center;

            maxdistance3d = std::max(d.Length(), maxdistance3d);
            d.y = 0;
            maxdistance2d = std::max(d.Length(), maxdistance2d);
          }
        }

        float playerdist =
            /*WorldToGameCoords*/ ((center - mumbleLink.charPosition).Length());
        maxdistance2d = /*WorldToGameCoords*/ (maxdistance2d);
        maxdistance3d = /*WorldToGameCoords*/ (maxdistance3d);

        infoline = std::format(
            "map: {:d} "
            "markercenter: {:.2f} {:.2f} {:.2f} "
            "maxdist2d: {:.2f} "
            "maxdist3d: {:.2f} "
            "playerdist: {:.2f}",
            mumbleLink.mapID, center.x, center.y, center.z, maxdistance2d,
            maxdistance3d, playerdist);
      }
    }

    CPoint startpos = font->GetTextPosition(
        infoline, GetClientRect(), WBTEXTALIGNMENTX::WBTA_CENTERX,
        WBTEXTALIGNMENTY::WBTA_TOP, WBTEXTTRANSFORM::WBTT_UPPERCASE);

    font->Write(API, infoline, startpos, CColor{0xffffffff},
                WBTEXTTRANSFORM::WBTT_UPPERCASE, true);
    ypos += font->GetLineHeight();
  }

  extern int gw2WindowCount;
  if (gw2WindowCount > 1) {
    auto font = App->GetFont("UniFontOutlined");
    if (!font) return;

    auto infoline = DICT("multiclientwarning");
    CPoint spos2 = font->GetTextPosition(
        infoline, GetClientRect(), WBTEXTALIGNMENTX::WBTA_CENTERX,
        WBTEXTALIGNMENTY::WBTA_TOP, WBTEXTTRANSFORM::WBTT_UPPERCASE);

    /*
        for (int x = 0; x < 3; x++)
          for (int y = 0; y < 3; y++)
            font->Write(API, infoline, CPoint(spos2.x + x - 1, ypos + y - 1),
                        CColor{0xff000000}, WBTEXTTRANSFORM::WBTT_UPPERCASE,
       true);
    */
    font->Write(API, infoline, CPoint(spos2.x, ypos), CColor{0xffff4040},
                WBTEXTTRANSFORM::WBTT_UPPERCASE, true);
    ypos += font->GetLineHeight();
  }

  if (RebindMode) {
    API->DrawRect(GetClientRect(), CColor{0x60000000});
    CWBFont* f = GetFont(GetState());

    std::string line1;

    if (!ScriptRebindMode) {
      int32_t key = -1;
      for (const auto& kb : KeyBindings) {
        if (kb.second == ActionToRebind) {
          key = kb.first;
          break;
        }
      }

      if (key == -1) {
        line1 = DICT("action") + " '" +
                DICT(ActionNames[static_cast<int32_t>(ActionToRebind)]) + "' " +
                DICT("currently_not_bound");
      } else {
        line1 = DICT("action") + " '" +
                DICT(ActionNames[static_cast<int32_t>(ActionToRebind)]) + "' " +
                DICT("currently_bound") + std::format(" '{:c}'", key);
      }
    } else {
      if (ScriptActionToRebind <
          0 /*|| ScriptActionToRebind >= scriptKeyBinds.NumItems()*/) {
        RebindMode = false;
        ScriptRebindMode = false;
      } else {
      }
    }
    auto line2 = DICT("press_to_bind");
    auto line3 = DICT("escape_to_unbind");
    CPoint line1p = f->GetTextPosition(
        line1, GetClientRect(), WBTEXTALIGNMENTX::WBTA_CENTERX,
        WBTEXTALIGNMENTY::WBTA_CENTERY, WBTEXTTRANSFORM::WBTT_NONE, true);
    CPoint line2p = f->GetTextPosition(
        line2, GetClientRect(), WBTEXTALIGNMENTX::WBTA_CENTERX,
        WBTEXTALIGNMENTY::WBTA_CENTERY, WBTEXTTRANSFORM::WBTT_NONE, true);
    CPoint line3p = f->GetTextPosition(
        line3, GetClientRect(), WBTEXTALIGNMENTX::WBTA_CENTERX,
        WBTEXTALIGNMENTY::WBTA_CENTERY, WBTEXTTRANSFORM::WBTT_NONE, true);
    f->Write(API, line1, line1p - CPoint(0, f->GetLineHeight() / 2));
    f->Write(API, line2,
             line2p - CPoint(0, f->GetLineHeight() / 2) +
                 CPoint(0, f->GetLineHeight()));
    f->Write(API, line3,
             line3p - CPoint(0, f->GetLineHeight() / 2) +
                 CPoint(0, 2 * f->GetLineHeight()));
  }

  if (ApiKeyInputMode) {
    API->DrawRect(GetClientRect(), CColor{0x60000000});
    CWBFont* f = GetFont(GetState());

    auto line1 = DICT("enter_api") + " " +
                 DICT(APIKeyNames[static_cast<int32_t>(ApiKeyToSet)]) + " " +
                 DICT("below_and_press");
    CPoint line1p = f->GetTextPosition(
        line1, GetClientRect(), WBTEXTALIGNMENTX::WBTA_CENTERX,
        WBTEXTALIGNMENTY::WBTA_CENTERY, WBTEXTTRANSFORM::WBTT_NONE, true);

    if (ApiKeyToSet == APIKeys::TS3APIKey) {
      auto line2 = DICT("ts3_help_1");
      auto line3 = DICT("ts3_help_2");
      CPoint line2p = f->GetTextPosition(
          line2, GetClientRect(), WBTEXTALIGNMENTX::WBTA_CENTERX,
          WBTEXTALIGNMENTY::WBTA_CENTERY, WBTEXTTRANSFORM::WBTT_NONE, true);
      CPoint line3p = f->GetTextPosition(
          line3, GetClientRect(), WBTEXTALIGNMENTX::WBTA_CENTERX,
          WBTEXTALIGNMENTY::WBTA_CENTERY, WBTEXTTRANSFORM::WBTT_NONE, true);

      f->Write(API, line2,
               line2p - CPoint(0, f->GetLineHeight() / 2) +
                   CPoint(0, f->GetLineHeight() * 3));
      f->Write(API, line3,
               line3p - CPoint(0, f->GetLineHeight() / 2) +
                   CPoint(0, f->GetLineHeight() * 4));
    }

    if (ApiKeyToSet == APIKeys::GW2APIKey) {
      auto line2 = DICT("gw2_api_help_1");
      std::string_view line3("https://account.arena.net/applications");
      CPoint line2p = f->GetTextPosition(
          line2, GetClientRect(), WBTEXTALIGNMENTX::WBTA_CENTERX,
          WBTEXTALIGNMENTY::WBTA_CENTERY, WBTEXTTRANSFORM::WBTT_NONE, true);
      CPoint line3p = f->GetTextPosition(
          line3, GetClientRect(), WBTEXTALIGNMENTX::WBTA_CENTERX,
          WBTEXTALIGNMENTY::WBTA_CENTERY, WBTEXTTRANSFORM::WBTT_NONE, true);

      f->Write(API, line2,
               line2p - CPoint(0, f->GetLineHeight() / 2) +
                   CPoint(0, f->GetLineHeight() * 3));
      f->Write(API, line3,
               line3p - CPoint(0, f->GetLineHeight() / 2) +
                   CPoint(0, f->GetLineHeight() * 4));
    }

    f->Write(API, line1, line1p - CPoint(0, f->GetLineHeight() / 2));
  }
}

void SetMouseToolTip(std::string_view toolTip) {
  extern std::unique_ptr<CWBApplication> App;

  if (!App) return;

  auto* tacoRoot = dynamic_cast<GW2TacO*>(
      App->GetRoot()->FindChildByID("tacoroot", "GW2TacO"));
  if (!tacoRoot) return;

  tacoRoot->SetMouseToolTip(toolTip);
}

void GW2TacO::OnPostDraw(CWBDrawAPI* API) {
  CWBFont* font = GetApplication()->GetRoot()->GetFont(WB_STATE_NORMAL);

  if (!font) return;

  if (mouseToolTip.empty()) return;

  int32_t width = font->GetWidth(mouseToolTip);

  CPoint pos = GetApplication()->GetMousePos();
  pos.x += 6;
  pos.y -= font->GetLineHeight() / 2;

  API->DrawRect(CRect(pos, pos + CPoint(width, font->GetLineHeight())),
                CColor(0, 0, 0, 0x80));
  font->Write(API, mouseToolTip, pos);
}

void GW2TacO::OpenWindow(std::string_view s) {
  CRect pos;
  if (!HasWindowData(s)) {
    pos = CRect(-150, -150, 150, 150) + GetClientRect().Center();
  } else {
    pos = GetWindowPosition(s);
  }

  auto itm = FindChildByID(s);
  if (itm) {
    bool openState = false;

    if (itm->IsHidden()) {
      openState = true;
    }

    itm->Hide(!openState);

    // delete itm;
    SetWindowOpenState(s, openState);
    if (!openState) {
      itm->MarkForDeletion();
    }
    return;
  }

  if (s == "MapTimer") {
    auto w = OverlayWindow::Create(this, pos);
    w->SetID(s);
    SetWindowOpenState(s, true);
    auto mt = GW2MapTimer::Create(w, w->GetClientRect());
    w->ReapplyStyles();
  }

  if (s == "TS3Control") {
    auto w = OverlayWindow::Create(this, pos);
    w->SetID(s);
    SetWindowOpenState(s, true);
    auto mt = TS3Control::Create(w, w->GetClientRect());
    w->ReapplyStyles();
  }

  if (s == "MarkerEditor") {
    auto w = OverlayWindow::Create(this, pos);
    w->SetID(s);
    SetWindowOpenState(s, true);
    auto mt = GW2MarkerEditor::Create(w, w->GetClientRect());
    w->ReapplyStyles();
  }

  if (s == "Notepad") {
    auto w = OverlayWindow::Create(this, pos);
    w->SetID(s);
    SetWindowOpenState(s, true);
    auto mt = GW2Notepad::Create(w, w->GetClientRect());
    w->ReapplyStyles();
  }

  if (s == "RaidProgress") {
    auto w = OverlayWindow::Create(this, pos);
    w->SetID(s);
    SetWindowOpenState(s, true);
    auto mt = RaidProgress::Create(w, w->GetClientRect());
    mt->SetID("RaidProgressView");
    w->ReapplyStyles();
  }

  if (s == "DungeonProgress") {
    auto w = OverlayWindow::Create(this, pos);
    w->SetID(s);
    SetWindowOpenState(s, true);
    auto mt = DungeonProgress::Create(w, w->GetClientRect());
    w->ReapplyStyles();
  }

  if (s == "TPTracker") {
    auto w = OverlayWindow::Create(this, pos);
    w->SetID(s);
    SetWindowOpenState(s, true);
    auto mt = TPTracker::Create(w, w->GetClientRect());
    w->ReapplyStyles();
  }
}

void GW2TacO::BuildChannelTree(TS3Connection::TS3Schandler& h,
                               CWBContextItem* parentitm, int32_t ParentID) {
  for (const auto& x : h.Channels) {
    const TS3Connection::TS3Channel& chn = x.second;
    if (chn.parentid == ParentID) {
      auto newitm = parentitm->AddItem(chn.name, 0);
      if (chn.id != chn.parentid) BuildChannelTree(h, newitm, chn.id);
    }
  }
}

void GW2TacO::RebindAction(TacOKeyAction Action) {
  RebindMode = true;
  ScriptRebindMode = false;
  ActionToRebind = Action;
  SetFocus();
}

void GW2TacO::RebindScriptKey(int32_t eventIndex) {
  RebindMode = true;
  ScriptRebindMode = true;
  ScriptActionToRebind = eventIndex;
  SetFocus();
}

void GW2TacO::ApiKeyInputAction(APIKeys keyType, int32_t idx) {
  ApiKeyInputMode = true;
  ApiKeyToSet = keyType;
  APIKeyInput =
      CWBTextBox::Create(this, GetClientRect(), WB_TEXTBOX_SINGLELINE);
  APIKeyInput->SetID("APIkeyInput");
  APIKeyInput->ReapplyStyles();
  APIKeyInput->EnableHScrollbar(false, false);
  APIKeyInput->EnableVScrollbar(false, false);
  CWBMessage m = BuildPositionMessage(GetClientRect());
  m.Resized = true;
  App->SendMessage(m);
  ApiKeyIndex = idx;

  switch (keyType) {
    case APIKeys::None:
      break;
    case APIKeys::TS3APIKey:
      if (HasConfigString("TS3APIKey")) {
        APIKeyInput->SetText(GetConfigString("TS3APIKey"));
      }
      break;
    case APIKeys::GW2APIKey: {
      auto key = GW2::apiKeyManager.GetKey(idx);
      APIKeyInput->SetText(key->apiKey);
    } break;
    default:
      break;
  }

  APIKeyInput->SetFocus();
}

void GW2TacO::TurnOnTPLight() { showPickupHighlight = true; }

void GW2TacO::TurnOffTPLight() { showPickupHighlight = false; }

void GW2TacO::CheckItemPickup() {
  if (GW2::apiKeyManager.GetStatus() != GW2::APIKeyManager::Status::OK) return;

  GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

  if (key && key->valid &&
      (GetTime() - lastPickupFetchTime > 150000 || !lastPickupFetchTime) &&
      !pickupsBeingFetched && !pickupFetcherThread.joinable()) {
    pickupsBeingFetched = true;
    pickupFetcherThread = std::thread([this, key]() {
      auto query = key->QueryAPI("v2/commerce/delivery");

      Object json;
      json.parse(query);

      int32_t coins = 0;
      int32_t itemCount = 0;

      if (json.has<Number>("coins")) {
        coins = static_cast<int32_t>(json.get<Number>("coins"));

        if (json.has<Array>("items")) {
          itemCount = json.get<Array>("items").size();

          if ((!coins && !itemCount) || lastItemPickup.empty()) {
            TurnOffTPLight();
            if (lastItemPickup.empty()) lastItemPickup = query;
          } else {
            if (query != lastItemPickup) {
              TurnOnTPLight();
              lastItemPickup = query;
            }
          }
        } else {
          TurnOffTPLight();
        }
      } else {
        TurnOffTPLight();
      }

      pickupsBeingFetched = false;
    });
  }

  if (!pickupsBeingFetched && pickupFetcherThread.joinable()) {
    lastPickupFetchTime = GetTime();
    pickupFetcherThread.join();
  }
}

void GW2TacO::StoreIconSizes() {
  if (iconSizesStored || !App) return;

  CWBItem* v1 = App->GetRoot()->FindChildByID("MenuButton");
  CWBItem* v2 = App->GetRoot()->FindChildByID("MenuHoverBox");
  CWBItem* v3 = App->GetRoot()->FindChildByID("TPButton");
  CWBItem* v4 = App->GetRoot()->FindChildByID("RedCircle");

  if (v1) {
    tacoIconRect = v1->GetPosition();
  }

  if (v2) {
    menuHoverRect = v2->GetPosition();
  }

  if (v3) {
    tpButtonRect = v3->GetPosition();
  }

  if (v4) {
    tpHighlightRect = v4->GetPosition();
  }

  iconSizesStored = true;
}

void GW2TacO::AdjustMenuForWindowTooSmallScale(float scale) {
  if (!iconSizesStored || !App) return;

  CWBItem* v1 = App->GetRoot()->FindChildByID("MenuButton");
  CWBItem* v2 = App->GetRoot()->FindChildByID("MenuHoverBox");
  CWBItem* v3 = App->GetRoot()->FindChildByID("TPButton");
  CWBItem* v4 = App->GetRoot()->FindChildByID("RedCircle");

  if (v1) {
    auto str = std::format(
        "#MenuButton{{ left:{:d}px; top:{:d}px; width:{:d}px; height:{:d}px; "
        "background:skin(taco_stretch_dark) top left; }} "
        "#MenuButton:hover{{background:skin(taco_stretch_light) top left;}} "
        "#MenuButton:active{{background:skin(taco_stretch_light) top left;}}",
        static_cast<int>(tacoIconRect.x1 * scale),
        static_cast<int>(tacoIconRect.y1 * scale),
        static_cast<int>(tacoIconRect.Width() * scale),
        static_cast<int>(tacoIconRect.Height() * scale));
    App->LoadCSS(str, false);
  }

  if (v2) {
    auto str = std::format(
        "#MenuHoverBox{{ left:{:d}px; top:{:d}px; width:{:d}px; height:{:d}px; "
        "}}",
        static_cast<int>(menuHoverRect.x1 * scale),
        static_cast<int>(menuHoverRect.y1 * scale),
        static_cast<int>(menuHoverRect.Width() * scale),
        static_cast<int>(menuHoverRect.Height() * scale));
    App->LoadCSS(str, false);
  }

  if (v3) {
    auto str = std::format(
        "#TPButton{{ left:{:d}px; top:{:d}px; width:{:d}px; height:{:d}px; }}",
        static_cast<int>(tpButtonRect.x1 * scale),
        static_cast<int>(tpButtonRect.y1 * scale),
        static_cast<int>(tpButtonRect.Width() * scale),
        static_cast<int>(tpButtonRect.Height() * scale));
    App->LoadCSS(str, false);
  }

  if (v4) {
    auto str = std::format(
        "#RedCircle{{ left:{:d}px; top:{:d}px; width:{:d}px; height:{:d}px; "
        "background:skin(redcircle_stretch) top left; }}",
        static_cast<int>(tpHighlightRect.x1 * scale),
        static_cast<int>(tpHighlightRect.y1 * scale),
        static_cast<int>(tpHighlightRect.Width() * scale),
        static_cast<int>(tpHighlightRect.Height() * scale));
    App->LoadCSS(str, false);
  }

  App->ReApplyStyle();
}
