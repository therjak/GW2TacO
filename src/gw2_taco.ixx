module;
#include <cassert>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/gw2_tactical.h"

export module taco.gw2taco;

import taco.ts3connection;
import whiteboard;

export enum class TacOKeyAction : int32_t {
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

export enum class APIKeys {
  None = 0,
  TS3APIKey,
  GW2APIKey,
};

export class GW2TacO : public gui::CWBGuiType<"GW2TacO", gui::CWBItem> {
 public:
  GW2TacO();
  static inline GW2TacO* Create(gui::CWBItem* parent, math::Rect position) {
    auto p = std::make_unique<GW2TacO>();
    p->Initialize(parent, position);
    auto r = p.get();
    assert(parent);
    parent->AddChild(std::move(p));
    return r;
  }
  ~GW2TacO() override;

  static gui::CWBItem* Factory(gui::CWBItem* root, const CXMLNode& node,
                               math::Rect& pos);

  void OpenWindow(std::string_view s);
  void OnDraw(gui::CWBDrawAPI* api) override;
  void OnPostDraw(gui::CWBDrawAPI* api) override;
  bool IsMouseTransparent(const math::Point& client_space_point,
                          gui::WBMESSAGE message_type) override;

  // return true if this item handled the message
  bool MessageProc(const gui::CWBMessage& message) override;

  void SetInfoLine(std::string_view string);
  void SetMouseToolTip(std::string_view tool_tip);

  void InitScriptEngines();
  void TickScriptEngine();
  void TriggerScriptEngineAction(GUID& guid);
  void TriggerScriptEngineKeyEvent(std::string_view event_id);

 private:
  void OpenAboutWindow();
  void BuildChannelTree(TS3Connection::TS3Schandler& h,
                        gui::CWBContextItem* parent_itm, int32_t parent_id);
  void RebindAction(TacOKeyAction action);
  void RebindScriptKey(int32_t event_idx);
  void ApiKeyInputAction(APIKeys key_type, int32_t idx);
  void TurnOnTPLight();
  void TurnOffTPLight();

  void CheckItemPickup();
  void StoreIconSizes();
  void AdjustMenuForWindowTooSmallScale(float scale);
  std::string GetKeybindString(TacOKeyAction action);

  std::string last_info_line_;
  bool rebind_mode_ = false;
  bool script_rebind_mode_ = false;
  TacOKeyAction action_to_rebind_ = TacOKeyAction::NoAction;
  int32_t script_action_to_rebind_ = 0;

  bool api_key_input_mode_ = false;
  APIKeys api_key_to_set_ = APIKeys::None;
  int32_t api_key_index_ = 0;

  std::unordered_map<int32_t, TacOKeyAction> key_bindings_;
  std::unordered_map<int32_t, std::string> script_key_bindings_;

  std::vector<GW2TacticalCategory*> category_list_;

  gui::CWBTextBox* api_key_input_ = nullptr;

  bool menu_hover_last_frame_ = false;
  int32_t last_menu_hover_transition_time_ = 0;

  std::string last_item_pickup_;
  int32_t last_pickup_fetch_time_ = 0;
  bool show_pickup_highlight_ = false;
  float last_scale_value_ = 1.0f;

  std::string mouse_tool_tip_;
  std::future<void> pickup_fetch_task_;
};

export constexpr std::string_view UIFileNames[] = {
    "UI_small.css",
    "UI_normal.css",
    "UI_large.css",
    "UI_larger.css",
};

export void SetMouseToolTip(std::string_view toolTip);
