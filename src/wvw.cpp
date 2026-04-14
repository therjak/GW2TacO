module;

#include <atomic>
#include <ctime>
#include <format>
#include <future>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

#include "src/gw2_tactical.h"
#include "src/util/jsonxx.h"

module taco.wvw;

import math;
import taco.gw2;
import taco.language;
import taco.mumble_link;
import taco.overlay_config;
import taco.poi_behavior;
import taco.time;

using math::CRect;
using math::CSize;
using math::CVector3;

bool wvwCanBeRendered = false;
std::vector<WvWObjective> wvwObjectives;
std::string FetchHTTPS(std::string_view url, std::string_view path);
std::unordered_map<std::string, POI> wvwPOIs;
std::unordered_map<int, bool> wvwMapIDs;

constexpr int DayFlag = 0x001000;
constexpr int DhmsFlag = 0x001111;
constexpr int HourFlag = 0x000100;
constexpr int HmsFlag = 0x000111;
constexpr int MinFlag = 0x000010;
constexpr int MsFlag = 0x000011;
constexpr int SecFlag = 0x000001;

void parseISO8601(const char* text, time_t& isotime, char& flag) {
  const char* c = nullptr;
  int num = 0;

  struct tm tmstruct {};

  int year = 0;
  int month = 0;
  int seconds = 0;
  int minutes = 0;
  int hours = 0;
  int days = 0;

  int dateflags = 0; /* flag which date component we've seen */

  c = text;
  isotime = 0;

  if (*c++ == 'P') {
    /* duration */
    flag = 'D';
    while (*c != '\0') {
      num = 0;
      while (*c >= '0' && *c <= '9') {
        /* assumes ASCII sequence! */
        num = 10 * num + *c++ - '0';
      }

      switch (*c++) {
        case 'D':
          if (dateflags & DhmsFlag) {
            /* day, hour, min or sec already set */
            return;
          } else {
            dateflags |= DayFlag;
            days = num;
          }
          break;
        case 'H':
          if (dateflags & HmsFlag) {
            /* hour, min or sec already set */
            return;
          } else {
            dateflags |= DayFlag;
            hours = num;
          }
          break;
        case 'M':
          if (dateflags & MsFlag) {
            /* min or sec already set */
            return;
          } else {
            dateflags |= MinFlag;
            minutes = num;
          }
          break;
        case 'S':
          if (dateflags & SecFlag) {
            /* sec already set */
            return;
          } else {
            dateflags |= SecFlag;
            seconds = num;
          }
          break;
        default:
          return;
      }
    }
    isotime = seconds + 60 * minutes + 3600 * hours + 86400 * days;
  } else {
    /* point in time, must be one of
    CCYYMMDD
    CCYY-MM-DD
    CCYYMMDDTHHMM
    CCYY-MM-DDTHH:MM
    CCYYMMDDTHHMMSS
    CCYY-MM-DDTHH:MM:SS
    */
    c = text;
    flag = 'T';

    /* NOTE: we have to check for the extended format first,
    because otherwise the separting '-' will be interpreted
    by sscanf as signs of a 1 digit integer .... :-(  */

    if (sscanf_s(text, "%4u-%2u-%2u", &year, &month, &days) == 3) {
      c += 10;
    } else if (sscanf_s(text, "%4u%2u%2u", &year, &month, &days) == 3) {
      c += 8;
    } else {
      return;
    }

    tmstruct.tm_year = year - 1900;
    tmstruct.tm_mon = month - 1;
    tmstruct.tm_mday = days;

    if (*c == '\0') {
      tmstruct.tm_hour = 0;
      tmstruct.tm_sec = 0;
      tmstruct.tm_min = 0;
      isotime = _mkgmtime(&tmstruct);
    } else if (*c == 'T') {
      /* time of day part */
      c++;
      if (sscanf_s(c, "%2d%2d", &hours, &minutes) == 2) {
        c += 4;
      } else if (sscanf_s(c, "%2d:%2d", &hours, &minutes) == 2) {
        c += 5;
      } else {
        return;
      }

      if (*c == ':') {
        c++;
      }

      if (*c != '\0') {
        if (sscanf_s(c, "%2d", &seconds) == 1) {
          c += 2;
        } else {
          return;
        }
        if (*c != '\0' && *c != 'Z') { /* something left? */
          return;
        }
      }
      tmstruct.tm_hour = hours;
      tmstruct.tm_min = minutes;
      tmstruct.tm_sec = seconds;
      isotime = _mkgmtime(&tmstruct);
    }

    else {
      return;
    }
  }
}

void LoadWvWObjectives() {
  // https://api.guildwars2.com/v2/wvw/objectives

  static std::future<void> wvwPollTask = std::async(std::launch::async, []() {
    std::unordered_map<int, CVector3> wvwObjectiveCoords;
    std::unordered_map<int, CRect> wvwContinentRects;

    auto wvwobjectives =
        FetchHTTPS("api.guildwars2.com", "/v2/wvw/objectives?ids=all");

    jsonxx::Array wvwobjs;
    wvwobjs.parse(wvwobjectives);
    auto objs = wvwobjs.values();

    for (auto& x : objs) {
      if (!x->is<jsonxx::Object>()) continue;

      auto obj = x->get<jsonxx::Object>();

      if (!obj.has<jsonxx::String>("id")) continue;

      auto objid = obj.get<jsonxx::String>("id");

      int mapID = 0, objident = 0;
      if (std::sscanf(objid.c_str(), "%d-%d", &mapID, &objident) != 2) continue;

      if (!obj.has<jsonxx::Number>("map_id")) continue;

      if (obj.get<jsonxx::Number>("map_id") != mapID) continue;

      wvwMapIDs[mapID] = true;

      if (obj.has<jsonxx::Array>("coord")) {
        if (wvwContinentRects.find(mapID) == wvwContinentRects.end()) {
          auto mapPath = std::format("/v2/maps?id={:d}", mapID);
          auto wvwMapData = FetchHTTPS("api.guildwars2.com", mapPath);

          jsonxx::Object map;
          map.parse(wvwMapData);
          if (!map.has<jsonxx::Array>("continent_rect")) continue;

          auto continentRectArray =
              map.get<jsonxx::Array>("continent_rect").values();
          if (continentRectArray.size() != 2) continue;

          int continentRectCnt = 0;
          int continentRectValues[4];
          bool ok = true;

          for (int x = 0; x < 2; x++) {
            if (!continentRectArray[x]->is<jsonxx::Array>()) {
              ok = false;
              break;
            }
            auto continentRectCoords =
                continentRectArray[x]->get<jsonxx::Array>().values();
            if (continentRectCoords.size() != 2) {
              ok = false;
              break;
            }

            for (int y = 0; y < 2; y++) {
              if (!continentRectCoords[y]->is<jsonxx::Number>()) {
                ok = false;
                break;
              }
              continentRectValues[continentRectCnt++] = static_cast<int>(
                  continentRectCoords[y]->get<jsonxx::Number>());
            }
          }

          if (ok) {
            wvwContinentRects[mapID] =
                CRect(continentRectValues[0], continentRectValues[1],
                      continentRectValues[2], continentRectValues[3]);
          }
        }

        if (wvwContinentRects.find(mapID) == wvwContinentRects.end()) {
          continue;
        }

        auto coord = obj.get<jsonxx::Array>("coord").values();
        if (coord.size() == 3) {
          CVector3 v(coord[0]->is<jsonxx::Number>()
                         ? static_cast<float>(coord[0]->get<jsonxx::Number>())
                         : 0,
                     coord[1]->is<jsonxx::Number>()
                         ? static_cast<float>(coord[1]->get<jsonxx::Number>())
                         : 0,
                     coord[2]->is<jsonxx::Number>()
                         ? static_cast<float>(coord[2]->get<jsonxx::Number>())
                         : 0);

          CRect& r = wvwContinentRects[mapID];
          CVector3 offset =
              CVector3((r.x1 + r.x2) / 2.0f, 0, (r.y1 + r.y2) / 2.0f);

          if (objident == 15 && abs(v.x - 11766.3) < 1 &&
              abs(v.y - 14793.5) < 1 &&
              abs(v.z - (-2133.39)) < 1)  // Langor fix-hack
          {
            v.x = 11462.5;
            v.y = 15600 - 2650 / 24;
            v.z -= 500;
          }

          wvwObjectiveCoords[objident] = CVector3(
              GameToWorldCoords((v.x - offset.x) * 24), GameToWorldCoords(-v.z),
              GameToWorldCoords((-(v.y - offset.z)) * 24));
        }
      }

      if (wvwObjectiveCoords.find(objident) == wvwObjectiveCoords.end()) {
        continue;
      }

      WvWObjective o;
      o.id = objid;
      o.mapID = mapID;
      o.objectiveID = objident;
      o.coord = wvwObjectiveCoords[objident];

      if (obj.has<jsonxx::String>("type"))
        o.type = obj.get<jsonxx::String>("type");

      if (obj.has<jsonxx::String>("name")) {
        o.nameToken = o.name = obj.get<jsonxx::String>("name");
      }

      for (char& n : o.nameToken) {
        if (!isalnum(n)) {
          n = '_';
        } else {
          n = tolower(n);
        }
      }

      POI poi;
      poi.position = o.coord;
      poi.mapID = o.mapID;
      poi.icon = DefaultIconHandle;
      poi.wvwObjectiveID = wvwObjectives.size();

      wvwObjectives.push_back(o);

      CoCreateGuid(&poi.guid);

      auto cat = GetCategory("Tactical.WvW." + o.type);

      if (cat) poi.SetCategory(cat);

      poi.typeData.behavior = POIBehavior::WvWObjective;

      wvwPOIs[o.id] = poi;
    }

    UpdateWvWStatus();

    wvwCanBeRendered = true;
  });
}

LockFreeQueue<std::vector<WvWPOIUpdate>> wvwPOIUpdates;

void UpdateWvWStatus() {
  if (wvwMapIDs.find(mumbleLink.mapID) == wvwMapIDs.end()) {
    return;
  }

  static std::future<void> wvwUpdateTask;
  static std::chrono::steady_clock::time_point lastUpdateTime;

  if (wvwUpdateTask.valid()) {
    if (wvwUpdateTask.wait_for(std::chrono::seconds(0)) !=
        std::future_status::ready) {
      return;
    }
  }

  auto now = std::chrono::steady_clock::now();
  if (std::chrono::duration_cast<std::chrono::milliseconds>(now -
                                                            lastUpdateTime)
          .count() < 5000) {
    return;
  }

  wvwUpdateTask = std::async(std::launch::async, []() {
    GW2::APIKeyManager::Status status = GW2::apiKeyManager.GetStatus();
    if (status != GW2::APIKeyManager::Status::OK) {
      return;
    }
    GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();
    if (!key) {
      return;
    }

    if (!key->Valid()) {
      return;
    }
    if (!key->HasCaps("account")) {
      return;
    }

    auto apiPath = std::format("/v2/wvw/matches?world={:d}", key->WorldID());
    auto wvwobjectiveids = FetchHTTPS("api.guildwars2.com", apiPath);

    jsonxx::Object o;
    o.parse(wvwobjectiveids);
    if (o.has<jsonxx::Array>("maps")) {
      auto m = o.get<jsonxx::Array>("maps").values();
      std::vector<WvWPOIUpdate> updates;
      for (auto& x : m) {
        if (!x->is<jsonxx::Object>()) continue;

        auto map = x->get<jsonxx::Object>();

        if (!map.has<jsonxx::Array>("objectives")) continue;

        auto objs = map.get<jsonxx::Array>("objectives").values();
        for (auto& obj : objs) {
          if (!obj->is<jsonxx::Object>()) continue;
          auto objective = obj->get<jsonxx::Object>();

          std::string id;
          if (objective.has<jsonxx::String>("id")) {
            id = objective.get<jsonxx::String>("id");
          } else {
            continue;
          }

          WvWPOIUpdate update = {
              .id = id,
          };

          std::string owner;
          if (objective.has<jsonxx::String>("owner")) {
            owner = objective.get<jsonxx::String>("owner");
          }

          if (owner == "Red") {
            update.owner = WvWPOIUpdate::Team::kRed;
          } else if (owner == "Green") {
            update.owner = WvWPOIUpdate::Team::kGreen;
          } else if (owner == "Blue") {
            update.owner = WvWPOIUpdate::Team::kBlue;
          } else {
            update.owner = WvWPOIUpdate::Team::kNone;
          }

          std::string lastFlipped;
          if (objective.has<jsonxx::String>("last_flipped")) {
            lastFlipped = objective.get<jsonxx::String>("last_flipped");
          }

          time_t flipTime = 0;
          char flags = 0;
          parseISO8601(lastFlipped.c_str(), flipTime, flags);
          update.lastFlipped = flipTime;

          updates.push_back(update);
        }
      }
      wvwPOIUpdates.push(updates);
    }
  });

  lastUpdateTime = now;
}
