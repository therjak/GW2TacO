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
import time;

using math::Rect;
using math::Size;
using math::Vector3;

bool wvwCanBeRendered = false;
std::vector<WvwObjective> wvw_objectives;
std::string FetchHTTPS(std::string_view url, std::string_view path);
std::unordered_map<std::string, POI> wvwPOIs;
std::unordered_map<int, bool> wvwmap_ids;

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

void LoadWvwObjectives() {
  // https://api.guildwars2.com/v2/wvw/objectives

  static std::future<void> wvwPollTask = std::async(std::launch::async, []() {
    std::unordered_map<int, Vector3> wvw_objective_coords;
    std::unordered_map<int, Rect> wvw_continent_rects;

    auto wvw_objectives_raw =
        FetchHTTPS("api.guildwars2.com", "/v2/wvw/objectives?ids=all");

    jsonxx::Array wvwobjs;
    wvwobjs.parse(wvw_objectives_raw);
    auto objs = wvwobjs.values();

    for (auto& x : objs) {
      if (!x->is<jsonxx::Object>()) continue;

      auto obj = x->get<jsonxx::Object>();

      if (!obj.has<jsonxx::String>("id")) continue;

      auto objid = obj.get<jsonxx::String>("id");

      int map_id = 0, objident = 0;
      if (std::sscanf(objid.c_str(), "%d-%d", &map_id, &objident) != 2)
        continue;

      if (!obj.has<jsonxx::Number>("map_id")) continue;

      if (obj.get<jsonxx::Number>("map_id") != map_id) continue;

      wvwmap_ids[map_id] = true;

      if (obj.has<jsonxx::Array>("coord")) {
        if (wvw_continent_rects.find(map_id) == wvw_continent_rects.end()) {
          auto mapPath = std::format("/v2/maps?id={:d}", map_id);
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
            wvw_continent_rects[map_id] =
                Rect(continentRectValues[0], continentRectValues[1],
                     continentRectValues[2], continentRectValues[3]);
          }
        }

        if (wvw_continent_rects.find(map_id) == wvw_continent_rects.end()) {
          continue;
        }

        auto coord = obj.get<jsonxx::Array>("coord").values();
        if (coord.size() == 3) {
          Vector3 v(coord[0]->is<jsonxx::Number>()
                        ? static_cast<float>(coord[0]->get<jsonxx::Number>())
                        : 0,
                    coord[1]->is<jsonxx::Number>()
                        ? static_cast<float>(coord[1]->get<jsonxx::Number>())
                        : 0,
                    coord[2]->is<jsonxx::Number>()
                        ? static_cast<float>(coord[2]->get<jsonxx::Number>())
                        : 0);

          Rect& r = wvw_continent_rects[map_id];
          Vector3 offset =
              Vector3((r.x1 + r.x2) / 2.0f, 0, (r.y1 + r.y2) / 2.0f);

          if (objident == 15 && abs(v.x - 11766.3) < 1 &&
              abs(v.y - 14793.5) < 1 &&
              abs(v.z - (-2133.39)) < 1)  // Langor fix-hack
          {
            v.x = 11462.5;
            v.y = 15600 - 2650 / 24;
            v.z -= 500;
          }

          wvw_objective_coords[objident] = Vector3(
              GameToWorldCoords((v.x - offset.x) * 24), GameToWorldCoords(-v.z),
              GameToWorldCoords((-(v.y - offset.z)) * 24));
        }
      }

      if (wvw_objective_coords.find(objident) == wvw_objective_coords.end()) {
        continue;
      }

      WvwObjective o;
      o.id_ = objid;
      o.map_id_ = map_id;
      o.objective_id_ = objident;
      o.coord_ = wvw_objective_coords[objident];

      if (obj.has<jsonxx::String>("type"))
        o.type_ = obj.get<jsonxx::String>("type");

      if (obj.has<jsonxx::String>("name")) {
        o.name_token_ = o.name_ = obj.get<jsonxx::String>("name");
      }

      for (char& n : o.name_token_) {
        if (!isalnum(n)) {
          n = '_';
        } else {
          n = tolower(n);
        }
      }

      POI poi;
      poi.position = o.coord_;
      poi.map_id = o.map_id_;
      poi.icon = DefaultIconHandle;
      poi.wvw_objective_id = wvw_objectives.size();

      wvw_objectives.push_back(o);

      CoCreateGuid(&poi.guid);

      auto cat = GetCategory("Tactical.WvW." + o.type_);

      if (cat) poi.SetCategory(cat);

      poi.type_data_.behavior_ = POIBehavior::WvwObjective;

      wvwPOIs[o.id_] = poi;
    }

    UpdateWvwStatus();

    wvwCanBeRendered = true;
  });
}

LockFreeQueue<std::vector<WvwPoiUpdate>> wvw_poi_updates;

void UpdateWvwStatus() {
  if (wvwmap_ids.find(mumbleLink.map_id) == wvwmap_ids.end()) {
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
    auto wvw_objective_ids = FetchHTTPS("api.guildwars2.com", apiPath);

    jsonxx::Object o;
    o.parse(wvw_objective_ids);
    if (o.has<jsonxx::Array>("maps")) {
      auto m = o.get<jsonxx::Array>("maps").values();
      std::vector<WvwPoiUpdate> updates;
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

          WvwPoiUpdate update = {
              .id_ = id,
          };

          std::string owner;
          if (objective.has<jsonxx::String>("owner")) {
            owner = objective.get<jsonxx::String>("owner");
          }

          if (owner == "Red") {
            update.owner_ = WvwPoiUpdate::Team::kRed;
          } else if (owner == "Green") {
            update.owner_ = WvwPoiUpdate::Team::kGreen;
          } else if (owner == "Blue") {
            update.owner_ = WvwPoiUpdate::Team::kBlue;
          } else {
            update.owner_ = WvwPoiUpdate::Team::kNone;
          }

          std::string last_flipped_str;
          if (objective.has<jsonxx::String>("last_flipped")) {
            last_flipped_str = objective.get<jsonxx::String>("last_flipped");
          }

          time_t flipTime = 0;
          char flags = 0;
          parseISO8601(last_flipped_str.c_str(), flipTime, flags);
          update.last_flipped_ = flipTime;

          updates.push_back(update);
        }
      }
      wvw_poi_updates.push(updates);
    }
  });

  lastUpdateTime = now;
}
