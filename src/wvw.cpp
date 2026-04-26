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

std::unordered_map<int, bool> wvw_map_ids;

// API constants
constexpr int kDayFlag = 0x001000;
constexpr int kDhmsFlag = 0x001111;
constexpr int kHourFlag = 0x000100;
constexpr int kHmsFlag = 0x000111;
constexpr int kMinFlag = 0x000010;
constexpr int kMsFlag = 0x000011;
constexpr int kSecFlag = 0x000001;

std::string FetchHTTPS(std::string_view url, std::string_view path);

void ParseISO8601(const char* text, time_t& iso_time, char& flag) {
  const char* c = text;
  int num = 0;
  struct tm tm_struct {};
  int year = 0;
  int month = 0;
  int seconds = 0;
  int minutes = 0;
  int hours = 0;
  int days = 0;
  /* flag which date component we've seen */
  int date_flags = 0;

  iso_time = 0;

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
          if (date_flags & kDhmsFlag) {
            /* day, hour, min or sec already set */
            return;
          }
          date_flags |= kDayFlag;
          days = num;
          break;
        case 'H':
          if (date_flags & kHmsFlag) {
            /* hour, min or sec already set */
            return;
          }
          date_flags |= kDayFlag;
          hours = num;
          break;
        case 'M':
          if (date_flags & kMsFlag) {
            /* min or sec already set */
            return;
          }
          date_flags |= kMinFlag;
          minutes = num;
          break;
        case 'S':
          if (date_flags & kSecFlag) {
            /* sec already set */
            return;
          }
          date_flags |= kSecFlag;
          seconds = num;
          break;
        default:
          return;
      }
    }
    iso_time = seconds + 60 * minutes + 3600 * hours + 86400 * days;
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

    if (std::sscanf(text, "%4u-%2u-%2u", &year, &month, &days) == 3) {
      c += 10;
    } else if (std::sscanf(text, "%4u%2u%2u", &year, &month, &days) == 3) {
      c += 8;
    } else {
      return;
    }

    tm_struct.tm_year = year - 1900;
    tm_struct.tm_mon = month - 1;
    tm_struct.tm_mday = days;

    if (*c == '\0') {
      tm_struct.tm_hour = 0;
      tm_struct.tm_sec = 0;
      tm_struct.tm_min = 0;
      iso_time = _mkgmtime(&tm_struct);
    } else if (*c == 'T') {
      c++;
      if (std::sscanf(c, "%2d%2d", &hours, &minutes) == 2) {
        c += 4;
      } else if (std::sscanf(c, "%2d:%2d", &hours, &minutes) == 2) {
        c += 5;
      } else {
        return;
      }

      if (*c == ':') c++;

      if (*c != '\0') {
        if (std::sscanf(c, "%2d", &seconds) == 1) {
          c += 2;
        } else {
          return;
        }
        if (*c != '\0' && *c != 'Z') return;
      }
      tm_struct.tm_hour = hours;
      tm_struct.tm_min = minutes;
      tm_struct.tm_sec = seconds;
      iso_time = _mkgmtime(&tm_struct);
    } else {
      return;
    }
  }
}

std::vector<WvwObjectiveData> ParseWvwObjectives(const std::string& json_data) {
  std::vector<WvwObjectiveData> result;
  jsonxx::Array wvw_objs;
  wvw_objs.parse(json_data);
  for (auto& x : wvw_objs.values()) {
    if (!x->is<jsonxx::Object>()) continue;
    auto obj = x->get<jsonxx::Object>();

    WvwObjectiveData data;
    if (obj.has<jsonxx::String>("id")) data.id = obj.get<jsonxx::String>("id");
    if (obj.has<jsonxx::String>("name"))
      data.name = obj.get<jsonxx::String>("name");
    if (obj.has<jsonxx::String>("type"))
      data.type = obj.get<jsonxx::String>("type");
    if (obj.has<jsonxx::Number>("sector_id"))
      data.sector_id = static_cast<int>(obj.get<jsonxx::Number>("sector_id"));
    if (obj.has<jsonxx::Number>("map_id"))
      data.map_id = static_cast<int>(obj.get<jsonxx::Number>("map_id"));
    if (obj.has<jsonxx::String>("map_type"))
      data.map_type = obj.get<jsonxx::String>("map_type");
    if (obj.has<jsonxx::String>("marker"))
      data.marker = obj.get<jsonxx::String>("marker");
    if (obj.has<jsonxx::String>("chat_link"))
      data.chat_link = obj.get<jsonxx::String>("chat_link");
    if (obj.has<jsonxx::Number>("upgrade_id"))
      data.upgrade_id = static_cast<int>(obj.get<jsonxx::Number>("upgrade_id"));

    if (obj.has<jsonxx::Array>("coord")) {
      for (auto& v : obj.get<jsonxx::Array>("coord").values()) {
        if (v->is<jsonxx::Number>()) {
          data.coord.push_back(static_cast<float>(v->get<jsonxx::Number>()));
        }
      }
    }
    if (obj.has<jsonxx::Array>("label_coord")) {
      for (auto& v : obj.get<jsonxx::Array>("label_coord").values()) {
        if (v->is<jsonxx::Number>()) {
          data.label_coord.push_back(
              static_cast<float>(v->get<jsonxx::Number>()));
        }
      }
    }
    result.push_back(data);
  }
  return result;
}

void LoadWvwObjectives() {
  // https://api.guildwars2.com/v2/wvw/objectives

  static std::future<void> wvw_poll_task = std::async(std::launch::async, []() {
    std::unordered_map<int, Vector3> wvw_objective_coords;
    std::unordered_map<int, Rect> wvw_continent_rects;

    auto wvw_objectives_raw =
        FetchHTTPS("api.guildwars2.com", "/v2/wvw/objectives?ids=all");

    auto objs = ParseWvwObjectives(wvw_objectives_raw);

    for (auto& api_obj : objs) {
      if (api_obj.id.empty()) continue;

      int map_id = 0, obj_ident = 0;
      if (std::sscanf(api_obj.id.c_str(), "%d-%d", &map_id, &obj_ident) != 2)
        continue;

      if (api_obj.map_id == 0 || api_obj.map_id != map_id) continue;

      wvw_map_ids[map_id] = true;

      if (!api_obj.coord.empty()) {
        if (wvw_continent_rects.find(map_id) == wvw_continent_rects.end()) {
          auto map_path = std::format("/v2/maps?id={:d}", map_id);
          auto wvw_map_data = FetchHTTPS("api.guildwars2.com", map_path);

          jsonxx::Object map;
          map.parse(wvw_map_data);
          if (!map.has<jsonxx::Array>("continent_rect")) continue;

          auto continent_rect_array =
              map.get<jsonxx::Array>("continent_rect").values();
          if (continent_rect_array.size() != 2) continue;

          int rect_cnt = 0;
          int rect_values[4];
          bool ok = true;

          for (int x = 0; x < 2; x++) {
            if (!continent_rect_array[x]->is<jsonxx::Array>()) {
              ok = false;
              break;
            }
            auto continent_rect_coords =
                continent_rect_array[x]->get<jsonxx::Array>().values();
            if (continent_rect_coords.size() != 2) {
              ok = false;
              break;
            }

            for (int y = 0; y < 2; y++) {
              if (!continent_rect_coords[y]->is<jsonxx::Number>()) {
                ok = false;
                break;
              }
              rect_values[rect_cnt++] = static_cast<int>(
                  continent_rect_coords[y]->get<jsonxx::Number>());
            }
          }

          if (ok) {
            wvw_continent_rects[map_id] = Rect(rect_values[0], rect_values[1],
                                               rect_values[2], rect_values[3]);
          }
        }

        if (wvw_continent_rects.find(map_id) == wvw_continent_rects.end()) {
          continue;
        }

        const auto& coord = api_obj.coord;
        if (coord.size() == 3) {
          Vector3 v(coord[0], coord[1], coord[2]);
          const Rect& r = wvw_continent_rects[map_id];
          Vector3 offset((r.x1 + r.x2) / 2.0f, 0, (r.y1 + r.y2) / 2.0f);

          // Langor fix-hack
          if (obj_ident == 15 && std::abs(v.x - 11766.3f) < 1.0f &&
              std::abs(v.y - 14793.5f) < 1.0f &&
              std::abs(v.z - (-2133.39f)) < 1.0f) {
            v.x = 11462.5f;
            v.y = 15600.0f - 2650.0f / 24.0f;
            v.z -= 500.0f;
          }

          wvw_objective_coords[obj_ident] =
              Vector3(GameToWorldCoords((v.x - offset.x) * 24.0f),
                      GameToWorldCoords(-v.z),
                      GameToWorldCoords((-(v.y - offset.z)) * 24.0f));
        }
      }

      if (wvw_objective_coords.find(obj_ident) == wvw_objective_coords.end()) {
        continue;
      }

      WvwObjective o;
      o.id_ = api_obj.id;
      o.map_id_ = map_id;
      o.objective_id_ = obj_ident;
      o.coord_ = wvw_objective_coords[obj_ident];

      if (!api_obj.type.empty()) o.type_ = api_obj.type;

      if (!api_obj.name.empty()) {
        o.name_token_ = o.name_ = api_obj.name;
      }

      for (char& n : o.name_token_) {
        if (!std::isalnum(static_cast<unsigned char>(n))) {
          n = '_';
        } else {
          n = static_cast<char>(std::tolower(static_cast<unsigned char>(n)));
        }
      }

      POI poi;
      poi.position = o.coord_;
      poi.map_id = o.map_id_;
      poi.icon = DefaultIconHandle;
      poi.wvw_objective_id = static_cast<int>(wvw_objectives.size());

      wvw_objectives.push_back(o);
      CoCreateGuid(&poi.guid);

      auto cat = GetCategory("Tactical.WvW." + o.type_);
      if (cat) poi.SetCategory(cat);
      poi.type_data_.behavior_ = POIBehavior::WvwObjective;
      wvw_pois[o.id_] = poi;
    }
    UpdateWvwStatus();
    wvw_can_be_rendered = true;
  });
}

void UpdateWvwStatus() {
  if (wvw_map_ids.find(mumbleLink.map_id) == wvw_map_ids.end()) return;

  static std::future<void> wvw_update_task;
  static std::chrono::steady_clock::time_point last_update_time;

  if (wvw_update_task.valid()) {
    if (wvw_update_task.wait_for(std::chrono::seconds(0)) !=
        std::future_status::ready) {
      return;
    }
  }

  auto now = std::chrono::steady_clock::now();
  if (std::chrono::duration_cast<std::chrono::milliseconds>(now -
                                                            last_update_time)
          .count() < 5000) {
    return;
  }

  wvw_update_task = std::async(std::launch::async, []() {
    GW2::APIKeyManager::Status status = GW2::apiKeyManager.GetStatus();
    if (status != GW2::APIKeyManager::Status::OK) return;
    GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();
    if (!key || !key->Valid() || !key->HasCaps("account")) return;

    auto api_path = std::format("/v2/wvw/matches?world={:d}", key->WorldID());
    auto wvw_objective_ids = FetchHTTPS("api.guildwars2.com", api_path);

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

          WvwPoiUpdate update{.id_ = id};
          std::string owner;
          if (objective.has<jsonxx::String>("owner"))
            owner = objective.get<jsonxx::String>("owner");

          if (owner == "Red")
            update.owner_ = WvwPoiUpdate::Team::kRed;
          else if (owner == "Green")
            update.owner_ = WvwPoiUpdate::Team::kGreen;
          else if (owner == "Blue")
            update.owner_ = WvwPoiUpdate::Team::kBlue;
          else
            update.owner_ = WvwPoiUpdate::Team::kNone;

          std::string last_flipped_str;
          if (objective.has<jsonxx::String>("last_flipped"))
            last_flipped_str = objective.get<jsonxx::String>("last_flipped");

          time_t flip_time = 0;
          char flags = 0;
          ParseISO8601(last_flipped_str.c_str(), flip_time, flags);
          update.last_flipped_ = flip_time;
          updates.push_back(update);
        }
      }
      wvw_poi_updates.push(updates);
    }
  });

  last_update_time = now;
}
