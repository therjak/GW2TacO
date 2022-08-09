#include "src/dungeon_progress.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <string>
#include <thread>
#include <unordered_map>

#include "src/base/logger.h"
#include "src/gw2_api.h"
#include "src/language.h"
#include "src/overlay_config.h"
#include "src/util/jsonxx.h"

using namespace jsonxx;
using math::CPoint;
using math::CRect;

void DungeonProgress::OnDraw(CWBDrawAPI* API) {
  CWBFont* f = GetFont(GetState());
  int32_t size = f->GetLineHeight();

  GW2::APIKeyManager::Status status =
      GW2::apiKeyManager.DisplayStatusText(API, f);
  GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

  if (key && key->valid &&
      (GetTime() - lastFetchTime > 150000 || !lastFetchTime) && !beingFetched &&
      !fetchThread.joinable()) {
    beingFetched = true;
    fetchThread = std::thread([this, key]() {
      const auto& lastDungeonStatus =
          "{\"dungeons\":" + key->QueryAPI("/v2/account/dungeons") + "}";
      const auto& dungeonFrequenterStatus =
          "{\"dungeons\":" +
          key->QueryAPI("/v2/account/achievements?ids=2963") + "}";
      Object json;
      Object json2;
      json.parse(lastDungeonStatus);
      json2.parse(dungeonFrequenterStatus);

      if (json.has<Array>("dungeons")) {
        const auto& dungeonData = json.get<Array>("dungeons").values();

        for (const auto& x : dungeonData) {
          if (!x->is<String>()) continue;

          auto eventName = x->get<String>();
          for (auto& d : dungeons) {
            for (auto& p : d.paths) {
              if (p.name == eventName) {
                std::lock_guard<std::mutex> lockGuard(p.mtx);
                p.finished = true;
              }
            }
          }
        }
      }

      if (json2.has<Array>("dungeons")) {
        const auto& dungeonData = json2.get<Array>("dungeons").values();

        std::vector<bool> fq(32, false);
        if (!dungeonData.empty() && dungeonData[0]->is<Object>()) {
          Object obj = dungeonData[0]->get<Object>();
          if (obj.has<Array>("bits")) {
            auto bits = obj.get<Array>("bits").values();
            if (bits.size() > 0) {
              for (auto& bit : bits) {
                if (bit->is<Number>()) {
                  auto frequentedID = static_cast<int32_t>(bit->get<Number>());
                  if (frequentedID < 0 || frequentedID >= fq.size()) {
                    Log_Err("unknown dungeon fequenterID: {:d}", frequentedID);
                    continue;
                  }
                  fq[frequentedID] = true;
                }
              }
            }
          }
        }
        for (auto& d : dungeons) {
          for (auto& p : d.paths) {
            if (p.id < 0) {
              continue;
            }
            std::lock_guard<std::mutex> lockGuard(p.mtx);
            p.frequenter = fq[p.id];
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

  int32_t posy = 1;
  int32_t textwidth = 0;

  for (const auto& d : dungeons) {
    textwidth = std::max(textwidth, f->GetWidth(d.shortName, false));
  }

  for (auto& d : dungeons) {
    f->Write(API, (std::string(d.shortName) + ":"), CPoint(0, posy + 1),
             CColor{0xffffffff});
    int32_t posx = textwidth + f->GetLineHeight() / 2;
    for (int y = 0; y < d.paths.size(); y++) {
      auto& p = d.paths[y];

      CRect r = CRect(posx, posy, posx + f->GetLineHeight() * 2,
                      posy + f->GetLineHeight() - 1);
      CRect cr = API->GetCropRect();
      API->SetCropRect(ClientToScreen(r));
      posx += f->GetLineHeight() * 2 + 1;
      if (y == 0) {
        posx += f->GetLineHeight() / 2;
      }
      {
        std::lock_guard<std::mutex> lockGuard(p.mtx);
        API->DrawRect(r, p.finished ? CColor{0x8033cc11} : CColor{0x80cc3322});
      }
      std::string s = y == 0 ? "S" : std::format("P{:d}", y);

      if (d.shortName == "TA") {
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

      CPoint tp = f->GetTextPosition(
          s, r + CRect(-3, 0, 0, 0), WBTEXTALIGNMENTX::WBTA_CENTERX,
          WBTEXTALIGNMENTY::WBTA_CENTERY, WBTEXTTRANSFORM::WBTT_NONE);
      tp.y = posy + 1;
      f->Write(API, s, tp, CColor{0xffffffff});
      {
        std::lock_guard<std::mutex> lockGuard(p.mtx);
        API->DrawRectBorder(
            r, p.frequenter ? CColor{0xffffcc00} : CColor{0x80000000});
      }
      API->SetCropRect(cr);
    }
    posy += f->GetLineHeight();
  }

  DrawBorder(API);
}

namespace {
constexpr auto st = "Story";
constexpr auto ex = "Explorable";
constexpr int32_t ignore = -1;  // does not count for dungeon frequenter
}  // namespace

DungeonProgress::DungeonProgress(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position),
      dungeons{
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

DungeonProgress::~DungeonProgress() {
  if (fetchThread.joinable()) fetchThread.join();
}

CWBItem* DungeonProgress::Factory(CWBItem* Root, CXMLNode& node, CRect& Pos) {
  return DungeonProgress::Create(Root, Pos);
}

bool DungeonProgress::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                         WBMESSAGE MessageType) {
  return true;
}
