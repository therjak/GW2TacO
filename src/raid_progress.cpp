#include "src/raid_progress.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <thread>
#include <unordered_set>

#include "src/base/logger.h"
#include "src/gw2_api.h"
#include "src/language.h"
#include "src/overlay_config.h"

using math::CPoint;
using math::CRect;

void RaidProgress::OnDraw(CWBDrawAPI* API) {
  bool compact = GetConfigValue("CompactRaidWindow");

  CWBFont* f = GetFont(GetState());
  GW2::APIKeyManager::Status status =
      GW2::apiKeyManager.DisplayStatusText(API, f);
  if (status != GW2::APIKeyManager::Status::OK) {
    return;
  }
  GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

  if (key && key->valid &&
      (GetTime() - lastFetchTime > 150000 || !lastFetchTime) && !beingFetched &&
      !fetchThread.joinable()) {
    beingFetched = true;
    fetchThread = std::thread([this, key]() {
      auto finishedEvents = key->Raids();

      for (auto& r : raids) {
        for (auto& w : r.wings) {
          for (auto& e : w.events) {
            if (finishedEvents.contains(std::string(e.name))) {
              e.finished = true;
            }
          }
        }
      }

      beingFetched = false;
    });
  }

  if (!beingFetched && fetchThread.joinable()) {
    lastFetchTime = GetTime();
    fetchThread.join();
  }

  int32_t posx = 0;
  if (compact) {
    for (const auto& r : raids) posx = std::max(posx, f->GetWidth(r.shortName));
  }
  posx += 3;
  int32_t oposx = posx;

  int32_t posy = 0;
  for (auto& r : raids) {
    if (HasConfigValue(r.configName) && !GetConfigValue(r.configName)) continue;

    if (!compact) {
      f->Write(API, DICT(r.configName, r.name), CPoint(0, posy + 1),
               CColor{0xffffffff});
      posy += f->GetLineHeight();
    } else {
      f->Write(API, r.shortName, CPoint(0, posy + 1), CColor{0xffffffff});
    }
    for (size_t y = 0; y < r.wings.size(); y++) {
      auto& w = r.wings[y];

      if (!compact) {
        posx = f->GetLineHeight() * 1;
      } else {
        posx = oposx;
      }

      if (!compact) {
        f->Write(API, DICT("raid_wing") + std::to_string(y + 1),
                 CPoint(posx, posy + 1), CColor{0xffffffff});
      }

      if (!compact) posx = f->GetLineHeight() * 3;

      int cnt = 1;

      for (auto& e : w.events) {
        CRect r = CRect(posx, posy, posx + f->GetLineHeight() * 2,
                        posy + f->GetLineHeight() - 1);
        CRect cr = API->GetCropRect();
        API->SetCropRect(ClientToScreen(r));
        posx += f->GetLineHeight() * 2 + 1;
        API->DrawRect(r, e.finished ? CColor{0x8033cc11} : CColor{0x80cc3322});
        auto s = e.type == RaidEvent::Type::Boss
                     ? (DICT("raid_boss") + std::to_string(cnt))
                     : DICT("raid_event");

        if (e.type == RaidEvent::Type::Boss) cnt++;

        CPoint tp = f->GetTextPosition(
            s, r + CRect(-3, 0, 0, 0), WBTEXTALIGNMENTX::WBTA_CENTERX,
            WBTEXTALIGNMENTY::WBTA_CENTERY, WBTEXTTRANSFORM::WBTT_NONE);
        tp.y = posy + 1;
        f->Write(API, s, tp, CColor{0xffffffff});
        API->DrawRectBorder(r, CColor{0x80000000});
        API->SetCropRect(cr);
      }

      posy += f->GetLineHeight();
    }
  }

  DrawBorder(API);
}

RaidProgress::RaidProgress(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position),
      raids{
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
      } {}

RaidProgress::~RaidProgress() {
  if (fetchThread.joinable()) fetchThread.join();
}

CWBItem* RaidProgress::Factory(CWBItem* Root, CXMLNode& node, CRect& Pos) {
  return RaidProgress::Create(Root, Pos);
}

bool RaidProgress::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                      WBMESSAGE MessageType) {
  return true;
}

std::vector<Raid>& RaidProgress::GetRaids() { return raids; }
