#include "DungeonProgress.h"

#include <cctype>
#include <string>
#include <unordered_map>

#include "Bedrock/BaseLib/string_format.h"
#include "Bedrock/UtilLib/jsonxx.h"
#include "GW2API.h"
#include "Language.h"
#include "OverlayConfig.h"

std::unordered_map<std::string, int32_t> dungeonToAchievementMap;

using namespace jsonxx;

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
      if (!hasFullDungeonInfo) {
        Object json;

        auto globalRaidInfo =
            "{\"dungeons\":" + key->QueryAPI("v2/dungeons") + "}";
        json.parse(globalRaidInfo);

        if (json.has<Array>("dungeons")) {
          auto dungeonData = json.get<Array>("dungeons").values();

          for (auto& x : dungeonData) {
            if (!x->is<String>()) continue;

            Dungeon d;
            d.name = x->get<String>();

            auto raidInfo = key->QueryAPI("v2/dungeons/" + d.name);
            Object dungeonJson;
            dungeonJson.parse(raidInfo);

            if (dungeonJson.has<Array>("paths")) {
              auto wings = dungeonJson.get<Array>("paths").values();
              for (auto& wing : wings) {
                auto dungeonPath = wing->get<Object>();
                DungeonPath p;
                if (dungeonPath.has<String>("id"))
                  p.name = dungeonPath.get<String>("id");

                if (dungeonPath.has<String>("type"))
                  p.type = dungeonPath.get<String>("type");

                d.paths.push_back(p);
              }
            }

            if (!d.name.empty()) {
              d.shortName = std::toupper(d.name[0]);

              for (unsigned int y = 0; y + 1 < d.name.size(); y++) {
                if (d.name[y] == '_') {
                  if (d.name[y + 1] == 'o' && y + 2 < d.name.size() &&
                      d.name[y + 2] == 'f') {
                    d.shortName += "o";
                  } else if (d.name[y + 1] == 't' && y + 3 < d.name.size() &&
                             d.name[y + 2] == 'h' && d.name[y + 3] == 'e') {
                    d.shortName += "t";
                  } else if (d.name[y + 1] == 'a' && y + 4 < d.name.size() &&
                             d.name[y + 2] == 'r' && d.name[y + 3] == 'a' &&
                             d.name[y + 4] == 'h') {
                    d.shortName = "Arah";
                  } else {
                    d.shortName += std::toupper(d.name[y + 1]);
                  }
                }
              }
            }

            dungeons.push_back(d);
          }
        }

        hasFullDungeonInfo = true;
      }

      auto lastDungeonStatus =
          "{\"dungeons\":" + key->QueryAPI("v2/account/dungeons") + "}";
      auto dungeonFrequenterStatus =
          "{\"dungeons\":" + key->QueryAPI("v2/account/achievements?ids=2963") +
          "}";
      Object json;
      Object json2;
      json.parse(lastDungeonStatus);
      json2.parse(dungeonFrequenterStatus);

      if (json.has<Array>("dungeons")) {
        auto dungeonData = json.get<Array>("dungeons").values();

        for (auto& x : dungeonData) {
          if (!x->is<String>()) continue;

          auto eventName = x->get<String>();
          for (auto& d : dungeons)
            for (auto& p : d.paths) {
              if (p.name == eventName) p.finished = true;
            }
        }
      }

      if (json2.has<Array>("dungeons")) {
        auto dungeonData = json2.get<Array>("dungeons").values();

        for (auto& d : dungeons)
          for (auto& p : d.paths) p.frequenter = false;

        if (!dungeonData.empty() && dungeonData[0]->is<Object>()) {
          Object obj = dungeonData[0]->get<Object>();
          if (obj.has<Array>("bits")) {
            auto bits = obj.get<Array>("bits").values();
            if (bits.size() > 0) {
              for (auto& bit : bits) {
                if (bit->is<Number>()) {
                  auto frequentedID = static_cast<int32_t>(bit->get<Number>());
                  for (auto& d : dungeons)
                    for (auto& p : d.paths) {
                      if (dungeonToAchievementMap.find(p.name) !=
                          dungeonToAchievementMap.end()) {
                        if (dungeonToAchievementMap[p.name] == frequentedID)
                          p.frequenter = true;
                      }
                    }
                }
              }
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

  if (hasFullDungeonInfo) {
    int32_t posy = 1;

    int32_t textwidth = 0;
    for (const auto& d : dungeons)
      textwidth = max(textwidth, f->GetWidth(d.shortName.c_str(), false));

    for (auto& d : dungeons) {
      f->Write(API, (d.shortName + ":").c_str(), CPoint(0, posy + 1),
               0xffffffff);
      int32_t posx = textwidth + f->GetLineHeight() / 2;
      for (int y = 0; y < d.paths.size(); y++) {
        auto& p = d.paths[y];

        CRect r = CRect(posx, posy, posx + f->GetLineHeight() * 2,
                        posy + f->GetLineHeight() - 1);
        CRect cr = API->GetCropRect();
        API->SetCropRect(ClientToScreen(r));
        posx += f->GetLineHeight() * 2 + 1;
        if (y == 0) posx += f->GetLineHeight() / 2;
        API->DrawRect(r, p.finished ? 0x8033cc11 : 0x80cc3322);
        std::string s = y == 0 ? "S" : FormatString("P%d", y);

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

        CPoint tp = f->GetTextPosition(s, r + CRect(-3, 0, 0, 0), WBTA_CENTERX,
                                       WBTA_CENTERY, WBTT_NONE);
        tp.y = posy + 1;
        f->Write(API, s, tp, 0xffffffff);
        API->DrawRectBorder(r, p.frequenter ? 0xffffcc00 : 0x80000000);
        API->SetCropRect(cr);
      }
      posy += f->GetLineHeight();
    }
  } else
    f->Write(API, DICT("waitingforapi"), CPoint(0, 0), 0xffffffff);

  DrawBorder(API);
}

DungeonProgress::DungeonProgress(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position) {
  dungeonToAchievementMap["ac_story"] = 4;
  dungeonToAchievementMap["hodgins"] = 5;
  dungeonToAchievementMap["detha"] = 6;
  dungeonToAchievementMap["tzark"] = 7;
  dungeonToAchievementMap["cm_story"] = 12;
  dungeonToAchievementMap["asura"] = 13;
  dungeonToAchievementMap["seraph"] = 14;
  dungeonToAchievementMap["butler"] = 15;
  dungeonToAchievementMap["ta_story"] = 20;
  dungeonToAchievementMap["leurent"] = 21;
  dungeonToAchievementMap["vevina"] = 22;
  dungeonToAchievementMap["aetherpath"] = 23;
  dungeonToAchievementMap["se_story"] = 16;
  dungeonToAchievementMap["fergg"] = 17;
  dungeonToAchievementMap["rasalov"] = 18;
  dungeonToAchievementMap["koptev"] = 19;
  dungeonToAchievementMap["cof_story"] = 28;
  dungeonToAchievementMap["ferrah"] = 29;
  dungeonToAchievementMap["magg"] = 30;
  dungeonToAchievementMap["rhiannon"] = 31;
  dungeonToAchievementMap["hotw_story"] = 24;
  dungeonToAchievementMap["butcher"] = 25;
  dungeonToAchievementMap["plunderer"] = 26;
  dungeonToAchievementMap["zealot"] = 27;
  dungeonToAchievementMap["coe_story"] = 0;
  dungeonToAchievementMap["submarine"] = 1;
  dungeonToAchievementMap["teleporter"] = 2;
  dungeonToAchievementMap["front_door"] = 3;
  dungeonToAchievementMap["jotun"] = 8;
  dungeonToAchievementMap["mursaat"] = 9;
  dungeonToAchievementMap["forgotten"] = 10;
  dungeonToAchievementMap["seer"] = 11;
}

DungeonProgress::~DungeonProgress() {
  if (fetchThread.joinable()) fetchThread.join();
}

CWBItem* DungeonProgress::Factory(CWBItem* Root, CXMLNode& node, CRect& Pos) {
  return DungeonProgress::Create(Root, Pos).get();
}

bool DungeonProgress::IsMouseTransparent(CPoint& ClientSpacePoint,
                                         WBMESSAGE MessageType) {
  return true;
}
