#pragma once
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "src/gw2_tactical.h"
#include "src/ts3_connection.h"
#include "src/white_board/gui_item.h"
#include "src/white_board/text_box.h"
#include "src/white_board/window.h"

enum class TacOKeyAction : int32_t {
  NoAction = 0,
  AddPOI,
  RemovePOI,
  ActivatePOI,
  EditNotepad,
  StartTrailRec,
  PauseTrailRec,
  DeleteLastTrailSegment,
  ResumeTrailAndCreateNewSection,
  Toggle_tactical_layer,
  Toggle_range_circles,
  Toggle_tactical_compass,
  Toggle_locational_timers,
  Toggle_hp_grids,
  Toggle_mouse_highlight,
  Toggle_map_timer,
  Toggle_ts3_window,
  Toggle_marker_editor,
  Toggle_notepad,
  Toggle_raid_progress,
  Toggle_dungeon_progress,
  Toggle_tp_tracker,
  Toggle_window_edit_mode,

  // if you add one here, add it to the ActionNames array in the .cpp as well!
};

enum class APIKeys {
  None = 0,
  TS3APIKey,
  GW2APIKey,
};

extern std::vector<std::string_view> ActionNames;

class GW2TacO : public CWBItem {
  std::string lastInfoLine;
  bool RebindMode = false;
  bool ScriptRebindMode = false;
  TacOKeyAction ActionToRebind = TacOKeyAction::NoAction;
  int32_t ScriptActionToRebind = 0;

  bool ApiKeyInputMode = false;
  APIKeys ApiKeyToSet = APIKeys::None;
  int32_t ApiKeyIndex = 0;

  void OpenAboutWindow();
  void BuildChannelTree(TS3Connection::TS3Schandler& h,
                        CWBContextItem* parentitm, int32_t ParentID);

  std::unordered_map<int32_t, TacOKeyAction> KeyBindings;
  std::unordered_map<int32_t, std::string> ScriptKeyBindings;

  void RebindAction(TacOKeyAction Action);
  void RebindScriptKey(int32_t evendIDX);
  std::vector<GW2TacticalCategory*> CategoryList;

  void ApiKeyInputAction(APIKeys keyType, int32_t idx);
  CWBTextBox* APIKeyInput = nullptr;

  bool menuHoverLastFrame = false;
  int32_t lastMenuHoverTransitionTime = 0;

  void TurnOnTPLight();
  void TurnOffTPLight();

  void CheckItemPickup();

  std::string lastItemPickup;
  bool pickupsBeingFetched = false;
  std::thread pickupFetcherThread;
  int32_t lastPickupFetchTime = 0;
  bool showPickupHighlight = false;
  float lastScaleValue = 1.0f;

  void StoreIconSizes();
  void AdjustMenuForWindowTooSmallScale(float scale);

  std::string mouseToolTip;
  std::string GetKeybindString(TacOKeyAction action);

 public:
  void OnDraw(CWBDrawAPI* API) override;
  void OnPostDraw(CWBDrawAPI* API) override;
  bool IsMouseTransparent(const math::CPoint& ClientSpacePoint,
                          WBMESSAGE MessageType) override;

  GW2TacO(CWBItem* Parent, math::CRect Position);
  static inline GW2TacO* Create(CWBItem* Parent, math::CRect Position) {
    auto p = std::make_unique<GW2TacO>(Parent, Position);
    auto r = p.get();
    assert(Parent);
    Parent->AddChild(std::move(p));
    return r;
  }
  ~GW2TacO() override;

  static CWBItem* Factory(CWBItem* Root, const CXMLNode& node,
                          math::CRect& Pos);
  WB_DECLARE_GUIITEM("GW2TacO", CWBItem);
  void OpenWindow(std::string_view s);

  // return true if this item handled the message
  bool MessageProc(const CWBMessage& Message) override;

  void SetInfoLine(std::string_view string);
  void SetMouseToolTip(std::string_view toolTip);

  void InitScriptEngines();
  void TickScriptEngine();
  void TriggerScriptEngineAction(GUID& guid);
  void TriggerScriptEngineKeyEvent(std::string_view eventID);
};

extern std::string_view UIFileNames[];

void SetMouseToolTip(std::string_view toolTip);
