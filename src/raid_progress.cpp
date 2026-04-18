module;
#include <algorithm>
#include <cctype>
#include <format>
#include <future>
#include <unordered_set>

#include "src/gw2_tactical.h"

module taco.raid_progress;

import math;
import taco.gw2;
import taco.language;
import taco.overlay_config;
import time;
import whiteboard;
import xml;

using math::Point;
using math::Rect;

void RaidProgress::OnDraw(gui::CWBDrawAPI* api) {
  bool compact = GetConfigValue("CompactRaidWindow");

  gui::CWBFont* f = GetFont(GetState());
  GW2::APIKeyManager::Status status =
      GW2::apiKeyManager.DisplayStatusText(api, f);
  if (status != GW2::APIKeyManager::Status::OK) {
    return;
  }
  GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

  if (key && key->Valid() &&
      (GetTime() - last_fetch_time_ > 150000 || !last_fetch_time_)) {
    if (!fetch_task_.valid() || fetch_task_.wait_for(std::chrono::seconds(0)) ==
                                    std::future_status::ready) {
      last_fetch_time_ = GetTime();
      fetch_task_ = std::async(std::launch::async, [this, key]() {
        const auto& raid_data = key->QuerySet("/v2/account/raids_");
        raid_queue.push(raid_data);
      });
    }
  }

  const auto& new_raid_data = raid_queue.pop();
  if (new_raid_data.has_value()) {
    const auto& raid_data = new_raid_data.value();
    for (auto& r : raids_) {
      for (auto& w : r.wings) {
        for (auto& e : w.events) {
          e.finished = raid_data.contains(std::string(e.name));
        }
      }
    }
  }

  int32_t pos_x = 0;
  if (compact) {
    for (const auto& r : raids_)
      pos_x = std::max(pos_x, f->GetWidth(r.short_name));
  }
  pos_x += 3;
  int32_t original_pos_x = pos_x;

  int32_t pos_y = 0;
  for (auto& r : raids_) {
    if (HasConfigValue(r.config_name) && !GetConfigValue(r.config_name))
      continue;

    if (!compact) {
      f->Write(api, DICT(r.config_name, r.name), Point(0, pos_y + 1),
               CColor{0xffffffff});
      pos_y += f->GetLineHeight();
    } else {
      f->Write(api, r.short_name, Point(0, pos_y + 1), CColor{0xffffffff});
    }
    for (size_t y = 0; y < r.wings.size(); y++) {
      auto& w = r.wings[y];

      if (!compact) {
        pos_x = f->GetLineHeight() * 1;
      } else {
        pos_x = original_pos_x;
      }

      if (!compact) {
        f->Write(api, DICT("raid_wing") + std::to_string(y + 1),
                 Point(pos_x, pos_y + 1), CColor{0xffffffff});
      }

      if (!compact) pos_x = f->GetLineHeight() * 3;

      int cnt = 1;

      for (auto& e : w.events) {
        Rect r = Rect(pos_x, pos_y, pos_x + f->GetLineHeight() * 2,
                      pos_y + f->GetLineHeight() - 1);
        Rect cr = api->GetCropRect();
        api->SetCropRect(ClientToScreen(r));
        pos_x += f->GetLineHeight() * 2 + 1;
        api->DrawRect(r, e.finished ? CColor{0x8033cc11} : CColor{0x80cc3322});
        auto s = e.type == RaidEvent::Type::Boss
                     ? (DICT("raid_boss") + std::to_string(cnt))
                     : DICT("raid_event");

        if (e.type == RaidEvent::Type::Boss) cnt++;

        Point tp = f->GetTextPosition(s, r + Rect(-3, 0, 0, 0),
                                      gui::WBTEXTALIGNMENTX::WBTA_CENTERX,
                                      gui::WBTEXTALIGNMENTY::WBTA_CENTERY,
                                      gui::WBTEXTTRANSFORM::WBTT_NONE);
        tp.y = pos_y + 1;
        f->Write(api, s, tp, CColor{0xffffffff});
        api->DrawRectBorder(r, CColor{0x80000000});
        api->SetCropRect(cr);
      }

      pos_y += f->GetLineHeight();
    }
  }

  DrawBorder(api);
}

RaidProgress::RaidProgress()
    : CWBGuiType(),
      raids_{
          Raid{"Forsaken Thicket",
               "FT",
               "showraid_forsaken_thicket",
               {Wing{"spirit_vale",
                     {{"vale_guardian", RaidEvent::Type::Boss},
                      {"spirit_woods", RaidEvent::Type::Checkpoint},
                      {"gorseval", RaidEvent::Type::Boss},
                      {"sabetha", RaidEvent::Type::Boss}}},
                Wing{"salvation_pass",
                     {{"slothasor", RaidEvent::Type::Boss},
                      {"bandit_trio", RaidEvent::Type::Boss},
                      {"matthias", RaidEvent::Type::Boss}}},
                Wing{"stronghold_of_the_faithful",
                     {{"escort", RaidEvent::Type::Boss},
                      {"keep_construct", RaidEvent::Type::Boss},
                      {"twisted_castle", RaidEvent::Type::Checkpoint},
                      {"xera", RaidEvent::Type::Boss}}}}},
          Raid{"Bastion of the Penitent",
               "BotP",
               "showraid_bastion_of_the_penitent",
               {Wing{"bastion_of_the_penitent",
                     {{"cairn", RaidEvent::Type::Boss},
                      {"mursaat_overseer", RaidEvent::Type::Boss},
                      {"samarog", RaidEvent::Type::Boss},
                      {"deimos", RaidEvent::Type::Boss}}}}},
          Raid{"Hall of Chains",
               "HoC",
               "showraid_hall_of_chains",
               {Wing{"hall_of_chains",
                     {{"soulless_horror", RaidEvent::Type::Boss},
                      {"river_of_souls", RaidEvent::Type::Boss},
                      {"statues_of_grenth", RaidEvent::Type::Boss},
                      {"voice_in_the_void", RaidEvent::Type::Boss}}}}},
          Raid{"Mythwright Gambit",
               "MG",
               "showraid_mythwright_gambit",
               {Wing{"mythwright_gambit",
                     {{"conjured_amalgamate", RaidEvent::Type::Boss},
                      {"twin_largos", RaidEvent::Type::Boss},
                      {"qadim", RaidEvent::Type::Boss}}}}},

          Raid{"The Key of Ahdashim",
               "TKoA",
               "showraid_the_key_of_ahdashim",
               {Wing{"the_key_of_ahdashim",
                     {{"gate", RaidEvent::Type::Checkpoint},
                      {"adina", RaidEvent::Type::Boss},
                      {"sabir", RaidEvent::Type::Boss},
                      {"qadim_the_peerless", RaidEvent::Type::Boss}}}}},
          Raid{"Mount Balrior",
               "MB",
               "showraid_mount_balrior",
               {Wing{"mount_balrior",
                     {{"camp", RaidEvent::Type::Checkpoint},
                      {"greer", RaidEvent::Type::Boss},
                      {"decima", RaidEvent::Type::Boss},
                      {"ura", RaidEvent::Type::Boss}}}}},
      } {}

RaidProgress::~RaidProgress() {}

gui::CWBItem* RaidProgress::Factory(gui::CWBItem* root, CXMLNode& node,
                                    Rect& pos) {
  return RaidProgress::Create(root, pos);
}

bool RaidProgress::IsMouseTransparent(const Point& client_space_point,
                                      gui::WBMESSAGE message_type) {
  return true;
}

std::vector<Raid>& RaidProgress::GetRaids() { return raids_; }
