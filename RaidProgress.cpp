#include "RaidProgress.h"

#include <cctype>

#include "Bedrock/BaseLib/string_format.h"
#include "Bedrock/UtilLib/jsonxx.h"
#include "GW2API.h"
#include "Language.h"
#include "OverlayConfig.h"

using namespace jsonxx;

void BeautifyString(std::string& str) {
  for (uint32_t x = 0; x < str.size(); x++) {
    if (str[x] == '_') {
      str[x] = ' ';
    }

    if (x == 0 || str[x - 1] == ' ') {
      str[x] = std::toupper(str[x]);
    }
    if (x > 0 && str[x - 1] == ' ') {
      if (str[x] == 'O' && x + 1 < str.size() && str[x + 1] == 'f') {
        str[x] = 'o';
      }
      if (str[x] == 'T' && x + 2 < str.size() && str[x + 1] == 'h' &&
          str[x + 2] == 'e') {
        str[x] = 't';
      }
    }
  }
}

void RaidProgress::OnDraw(CWBDrawAPI* API) {
  if (!HasConfigValue("CompactRaidWindow"))
    SetConfigValue("CompactRaidWindow", 0);

  bool compact = GetConfigValue("CompactRaidWindow");

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
      if (!hasFullRaidInfo) {
        Object json;

        auto globalRaidInfo = "{\"raids\":" + key->QueryAPI("/v2/raids") + "}";
        json.parse(globalRaidInfo);

        if (json.has<Array>("raids")) {
          auto raidData = json.get<Array>("raids").values();

          for (auto& x : raidData) {
            if (!x->is<String>()) continue;

            Raid r;
            r.name = x->get<String>();
            if (!r.name.empty()) {
              r.shortName = std::toupper(r.name[0]);

              for (unsigned int y = 0; y + 1 < r.name.size(); y++) {
                if (r.name[y] == '_' || r.name[y] == ' ') {
                  if (r.name[y + 1] == 'o' && y + 2 < r.name.size() &&
                      r.name[y + 2] == 'f') {
                    r.shortName += 'o';
                  } else if (r.name[y + 1] == 't' && y + 3 < r.name.size() &&
                             r.name[y + 2] == 'h' && r.name[y + 3] == 'e') {
                    r.shortName += 't';
                  } else {
                    r.shortName += std::toupper(r.name[y + 1]);
                  }
                }
              }
              r.shortName += ":";
            }

            r.configName = "showraid_" + r.name;
            for (char& y : r.configName)
              if (!isalnum(y))
                y = '_';
              else
                y = std::tolower(y);

            auto raidInfo = key->QueryAPI("/v2/raids/" + r.name);
            Object raidJson;
            raidJson.parse(raidInfo);

            if (raidJson.has<Array>("wings")) {
              auto wings = raidJson.get<Array>("wings").values();
              for (auto& y : wings) {
                auto wing = y->get<Object>();
                Wing w;
                if (wing.has<String>("id")) w.name = wing.get<String>("id");

                if (wing.has<Array>("events")) {
                  auto events = wing.get<Array>("events").values();
                  for (auto& event : events) {
                    auto _event = event->get<Object>();
                    RaidEvent e;
                    if (_event.has<String>("id"))
                      e.name = _event.get<String>("id");
                    if (_event.has<String>("type"))
                      e.type = _event.get<String>("type");

                    w.events.push_back(e);
                  }
                }
                r.wings.push_back(w);
              }
            }

            BeautifyString(r.name);
            raids.push_back(r);
          }
        }

        hasFullRaidInfo = true;
      }

      auto lastRaidStatus =
          "{\"raids\":" + key->QueryAPI("/v2/account/raids") + "}";
      Object json;
      json.parse(lastRaidStatus);

      if (json.has<Array>("raids")) {
        auto raidData = json.get<Array>("raids").values();

        for (auto& x : raidData) {
          if (!x->is<String>()) continue;

          auto eventName = x->get<String>();
          for (auto& r : raids)
            for (auto& w : r.wings)
              for (auto& e : w.events) {
                if (e.name == eventName) e.finished = true;
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

  if (hasFullRaidInfo) {
    int32_t posx = 0;
    if (compact) {
      for (const auto& r : raids)
        posx = max(posx, f->GetWidth(r.shortName.c_str()));
    }
    posx += 3;
    int32_t oposx = posx;

    int32_t posy = 0;
    for (auto& r : raids) {
      if (HasConfigValue(r.configName.c_str()) &&
          !GetConfigValue(r.configName.c_str()))
        continue;

      if (!compact) {
        f->Write(API, DICT(r.configName.c_str(), r.name.c_str()),
                 CPoint(0, posy + 1), CColor{0xffffffff});
        posy += f->GetLineHeight();
      } else {
        f->Write(API, r.shortName.c_str(), CPoint(0, posy + 1),
                 CColor{0xffffffff});
      }
      for (size_t y = 0; y < r.wings.size(); y++) {
        auto& w = r.wings[y];

        if (!compact)
          posx = f->GetLineHeight() * 1;
        else
          posx = oposx;

        if (!compact)
          f->Write(API, DICT("raid_wing") + FormatString("%d:", y + 1),
                   CPoint(posx, posy + 1), CColor{0xffffffff});

        if (!compact) posx = f->GetLineHeight() * 3;

        int cnt = 1;

        for (auto& e : w.events) {
          CRect r = CRect(posx, posy, posx + f->GetLineHeight() * 2,
                          posy + f->GetLineHeight() - 1);
          CRect cr = API->GetCropRect();
          API->SetCropRect(ClientToScreen(r));
          posx += f->GetLineHeight() * 2 + 1;
          API->DrawRect(r,
                        e.finished ? CColor{0x8033cc11} : CColor{0x80cc3322});
          auto s = e.type[0] == 'B'
                       ? (DICT("raid_boss") + FormatString("%d", cnt))
                       : DICT("raid_event");

          if (e.type[0] == 'B') cnt++;

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
  } else
    f->Write(API, DICT("waitingforapi"), CPoint(0, 0), CColor{0xffffffff});

  DrawBorder(API);
}

RaidProgress::RaidProgress(CWBItem* Parent, CRect Position)
    : CWBItem(Parent, Position) {}

RaidProgress::~RaidProgress() {
  if (fetchThread.joinable()) fetchThread.join();
}

CWBItem* RaidProgress::Factory(CWBItem* Root, CXMLNode& node, CRect& Pos) {
  return RaidProgress::Create(Root, Pos).get();
}

bool RaidProgress::IsMouseTransparent(const CPoint& ClientSpacePoint,
                                      WBMESSAGE MessageType) {
  return true;
}

std::vector<Raid>& RaidProgress::GetRaids() { return raids; }
