module;
#include <algorithm>
#include <cctype>
#include <format>
#include <future>
#include <string>
#include <unordered_map>

#include "src/gw2_tactical.h"

module taco.dungeon_progress;

import math;
import taco.gw2;
import taco.language;
import taco.overlay_config;
import time;
import whiteboard;
import xml;

using math::Point;
using math::Rect;

void DungeonProgress::OnDraw(gui::CWBDrawAPI* api) {
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
        const auto& dungeon_data = key->QuerySet("/v2/account/dungeons");
        dungeon_queue_.push(dungeon_data);
        const auto& dungeon_frequenter_status = key->QueryAchievementBits(2963);
        dungeon_achievements_queue_.push(dungeon_frequenter_status);
      });
    }
  }

  const auto& new_dungeon_data = dungeon_queue_.pop();
  if (new_dungeon_data.has_value()) {
    const auto& dungeon_data = new_dungeon_data.value();
    for (auto& d : dungeons_) {
      for (auto& p : d.paths_) {
        p.finished_ = dungeon_data.contains(std::string(p.name_));
      }
    }
  }

  const auto& new_dungeon_achievements = dungeon_achievements_queue_.pop();
  if (new_dungeon_achievements.has_value()) {
    const auto& dungeon_frequenter_status = new_dungeon_achievements.value();
    for (auto& d : dungeons_) {
      for (auto& p : d.paths_) {
        if (p.id_ < 0) {
          continue;
        }
        p.frequenter_ = dungeon_frequenter_status.contains(p.id_);
      }
    }
  }

  int32_t posy = 1;
  int32_t textwidth = 0;

  for (const auto& d : dungeons_) {
    textwidth = std::max(textwidth, f->GetWidth(d.short_name_, false));
  }

  for (auto& d : dungeons_) {
    f->Write(api, (std::string(d.short_name_) + ":"), Point(0, posy + 1),
             CColor{0xffffffff});
    int32_t posx = textwidth + f->GetLineHeight() / 2;
    for (int y = 0; y < d.paths_.size(); y++) {
      auto& p = d.paths_[y];

      Rect r = Rect(posx, posy, posx + f->GetLineHeight() * 2,
                    posy + f->GetLineHeight() - 1);
      Rect cr = api->GetCropRect();
      api->SetCropRect(ClientToScreen(r));
      posx += f->GetLineHeight() * 2 + 1;
      if (y == 0) {
        posx += f->GetLineHeight() / 2;
      }
      {
        api->DrawRect(r, p.finished_ ? CColor{0x8033cc11} : CColor{0x80cc3322});
      }
      std::string s = y == 0 ? "S" : std::format("P{:d}", y);

      if (d.short_name_ == "TA") {
        switch (y) {
          case 1:
            s = "Up";
            break;
          case 2:
            s = "Fwd";
            break;
          case 3:
            s = "Ae";
            break;
        }
      }

      Point tp = f->GetTextPosition(
          s, r + Rect(-3, 0, 0, 0), gui::WBTEXTALIGNMENTX::WBTA_CENTERX,
          gui::WBTEXTALIGNMENTY::WBTA_TOP, gui::WBTEXTTRANSFORM::WBTT_NONE);
      tp.y = posy + 1;
      f->Write(api, s, tp, CColor{0xffffffff});
      {
        api->DrawRectBorder(
            r, p.frequenter_ ? CColor{0xffffcc00} : CColor{0x80000000});
      }
      api->SetCropRect(cr);
    }
    posy += f->GetLineHeight();
  }

  DrawBorder(api);
}

namespace {
constexpr auto st = "Story";
constexpr auto ex = "Explorable";
constexpr int32_t ignore = -1;  // does not count for dungeon frequenter
}  // namespace

DungeonProgress::DungeonProgress()
    : CWBGuiType(),
      dungeons_{
          Dungeon{"ascalonian_catacombs",
                  "AC",
                  {{"ac_story", st, 4},
                   {"hodgins", ex, 5},
                   {"detha", ex, 6},
                   {"tzark", ex, 7}}},
          Dungeon{"caudecus_manor",
                  "CM",
                  {{"cm_story", st, 12},
                   {"asura", ex, 13},
                   {"seraph", ex, 14},
                   {"butler", ex, 15}}},
          Dungeon{"twilight_arbor",
                  "TA",
                  {{"ta_story", st, 20},
                   {"leurent", ex, 21},
                   {"vevina", ex, 22},
                   {"aetherpath", ex, 23}}},
          Dungeon{"sorrows_embrace",
                  "SE",
                  {{"se_story", st, 16},
                   {"fergg", ex, 17},
                   {"rasalov", ex, 18},
                   {"koptev", ex, 19}}},
          Dungeon{"citadel_of_flame",
                  "CoF",
                  {{"cof_story", st, 28},
                   {"ferrah", ex, 29},
                   {"magg", ex, 30},
                   {"rhiannon", ex, 31}}},
          Dungeon{"honor_of_the_waves",
                  "HotW",
                  {{"hotw_story", st, 24},
                   {"butcher", ex, 25},
                   {"plunderer", ex, 26},
                   {"zealot", ex, 27}}},
          Dungeon{"crucible_of_eternity",
                  "CoE",
                  {{"coe_story", st, 0},
                   {"submarine", ex, 1},
                   {"teleporter", ex, 2},
                   {"front_door", ex, 3}}},
          Dungeon{"ruined_city_of_arah",
                  "Arah",
                  {{"arah_story", st, ignore},
                   {"jotun", ex, 8},
                   {"mursaat", ex, 9},
                   {"forgotten", ex, 10},
                   {"seer", ex, 11}}},
      } {}

DungeonProgress::~DungeonProgress() {}

gui::CWBItem* DungeonProgress::Factory(gui::CWBItem* root, const CXMLNode& node,
                                       const Rect& pos) {
  return DungeonProgress::Create(root, pos);
}

bool DungeonProgress::IsMouseTransparent(const Point& client_space_point,
                                         gui::WBMESSAGE message_type) {
  return true;
}
