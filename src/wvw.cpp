module;

#include <atomic>
#include <cstdio>
#include <ctime>
#include <format>
#include <future>
#include <string>
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

WvwMapData ParseWvwMapData(const std::string& json_data) {
  WvwMapData data;
  jsonxx::Object map;
  if (!map.parse(json_data)) return data;

  if (map.has<jsonxx::Number>("id"))
    data.id = static_cast<int>(map.get<jsonxx::Number>("id"));
  if (map.has<jsonxx::String>("name"))
    data.name = map.get<jsonxx::String>("name");
  if (map.has<jsonxx::Number>("min_level"))
    data.min_level = static_cast<int>(map.get<jsonxx::Number>("min_level"));
  if (map.has<jsonxx::Number>("max_level"))
    data.max_level = static_cast<int>(map.get<jsonxx::Number>("max_level"));
  if (map.has<jsonxx::Number>("default_floor"))
    data.default_floor =
        static_cast<int>(map.get<jsonxx::Number>("default_floor"));
  if (map.has<jsonxx::String>("type"))
    data.type = map.get<jsonxx::String>("type");

  if (map.has<jsonxx::Array>("floors")) {
    for (auto& v : map.get<jsonxx::Array>("floors").values()) {
      if (v->is<jsonxx::Number>())
        data.floors.push_back(static_cast<int>(v->get<jsonxx::Number>()));
    }
  }

  if (map.has<jsonxx::Number>("region_id"))
    data.region_id = static_cast<int>(map.get<jsonxx::Number>("region_id"));
  if (map.has<jsonxx::String>("region_name"))
    data.region_name = map.get<jsonxx::String>("region_name");
  if (map.has<jsonxx::Number>("continent_id"))
    data.continent_id =
        static_cast<int>(map.get<jsonxx::Number>("continent_id"));
  if (map.has<jsonxx::String>("continent_name"))
    data.continent_name = map.get<jsonxx::String>("continent_name");

  auto parse_rect = [](const jsonxx::Array& arr) -> std::optional<math::Rect> {
    if (arr.values().size() != 2) return std::nullopt;
    int rect_values[4];
    int rect_cnt = 0;
    for (int x = 0; x < 2; x++) {
      if (!arr.values()[x]->is<jsonxx::Array>()) return std::nullopt;
      auto coords = arr.values()[x]->get<jsonxx::Array>().values();
      if (coords.size() != 2) return std::nullopt;
      for (int y = 0; y < 2; y++) {
        if (!coords[y]->is<jsonxx::Number>()) return std::nullopt;
        rect_values[rect_cnt++] =
            static_cast<int>(coords[y]->get<jsonxx::Number>());
      }
    }
    return math::Rect(rect_values[0], rect_values[1], rect_values[2],
                      rect_values[3]);
  };

  if (map.has<jsonxx::Array>("map_rect")) {
    data.map_rect = parse_rect(map.get<jsonxx::Array>("map_rect"));
  }
  if (map.has<jsonxx::Array>("continent_rect")) {
    data.continent_rect = parse_rect(map.get<jsonxx::Array>("continent_rect"));
  }

  return data;
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
          auto map_data = ParseWvwMapData(wvw_map_data);

          if (map_data.continent_rect) {
            wvw_continent_rects[map_id] = *map_data.continent_rect;
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

WvwMatch ParseWvwMatch(const std::string& json_data) {
  WvwMatch result;
  jsonxx::Object o;
  if (!o.parse(json_data)) return result;

  if (o.has<jsonxx::String>("id")) result.id = o.get<jsonxx::String>("id");
  if (o.has<jsonxx::String>("start_time")) result.start_time = o.get<jsonxx::String>("start_time");
  if (o.has<jsonxx::String>("end_time")) result.end_time = o.get<jsonxx::String>("end_time");

  auto parse_string_int_map = [](const jsonxx::Object& obj) {
    std::unordered_map<std::string, int> res;
    for (const auto& kv : obj.kv_map()) {
      if (kv.second->is<jsonxx::Number>()) {
        res[kv.first] = static_cast<int>(kv.second->get<jsonxx::Number>());
      }
    }
    return res;
  };

  if (o.has<jsonxx::Object>("scores")) result.scores = parse_string_int_map(o.get<jsonxx::Object>("scores"));
  if (o.has<jsonxx::Object>("worlds")) result.worlds = parse_string_int_map(o.get<jsonxx::Object>("worlds"));
  if (o.has<jsonxx::Object>("deaths")) result.deaths = parse_string_int_map(o.get<jsonxx::Object>("deaths"));
  if (o.has<jsonxx::Object>("kills")) result.kills = parse_string_int_map(o.get<jsonxx::Object>("kills"));
  if (o.has<jsonxx::Object>("victory_points")) result.victory_points = parse_string_int_map(o.get<jsonxx::Object>("victory_points"));

  if (o.has<jsonxx::Object>("all_worlds")) {
    auto aw = o.get<jsonxx::Object>("all_worlds");
    for (const auto& kv : aw.kv_map()) {
      if (kv.second->is<jsonxx::Array>()) {
        std::vector<int> worlds;
        for (auto& v : kv.second->get<jsonxx::Array>().values()) {
          if (v->is<jsonxx::Number>()) worlds.push_back(static_cast<int>(v->get<jsonxx::Number>()));
        }
        result.all_worlds[kv.first] = worlds;
      }
    }
  }

  if (o.has<jsonxx::Array>("maps")) {
    for (auto& m : o.get<jsonxx::Array>("maps").values()) {
      if (!m->is<jsonxx::Object>()) continue;
      auto map_obj = m->get<jsonxx::Object>();
      WvwMatchMap map_data;
      if (map_obj.has<jsonxx::Number>("id")) map_data.id = static_cast<int>(map_obj.get<jsonxx::Number>("id"));
      if (map_obj.has<jsonxx::String>("type")) map_data.type = map_obj.get<jsonxx::String>("type");
      if (map_obj.has<jsonxx::Object>("scores")) map_data.scores = parse_string_int_map(map_obj.get<jsonxx::Object>("scores"));
      if (map_obj.has<jsonxx::Object>("deaths")) map_data.deaths = parse_string_int_map(map_obj.get<jsonxx::Object>("deaths"));
      if (map_obj.has<jsonxx::Object>("kills")) map_data.kills = parse_string_int_map(map_obj.get<jsonxx::Object>("kills"));
      
      if (map_obj.has<jsonxx::Array>("bonuses")) {
        for (auto& b : map_obj.get<jsonxx::Array>("bonuses").values()) {
          if (!b->is<jsonxx::Object>()) continue;
          auto bonus_obj = b->get<jsonxx::Object>();
          WvwMatchBonus bonus;
          if (bonus_obj.has<jsonxx::String>("type")) bonus.type = bonus_obj.get<jsonxx::String>("type");
          if (bonus_obj.has<jsonxx::String>("owner")) bonus.owner = bonus_obj.get<jsonxx::String>("owner");
          map_data.bonuses.push_back(bonus);
        }
      }

      if (map_obj.has<jsonxx::Array>("objectives")) {
        for (auto& obj : map_obj.get<jsonxx::Array>("objectives").values()) {
          if (!obj->is<jsonxx::Object>()) continue;
          auto objective = obj->get<jsonxx::Object>();
          WvwMatchObjective obj_data;
          
          if (objective.has<jsonxx::String>("id")) obj_data.id = objective.get<jsonxx::String>("id");
          if (objective.has<jsonxx::String>("type")) obj_data.type = objective.get<jsonxx::String>("type");
          if (objective.has<jsonxx::String>("owner")) obj_data.owner = objective.get<jsonxx::String>("owner");
          if (objective.has<jsonxx::String>("last_flipped")) obj_data.last_flipped = objective.get<jsonxx::String>("last_flipped");
          if (objective.has<jsonxx::String>("claimed_by")) obj_data.claimed_by = objective.get<jsonxx::String>("claimed_by");
          if (objective.has<jsonxx::String>("claimed_at")) obj_data.claimed_at = objective.get<jsonxx::String>("claimed_at");
          if (objective.has<jsonxx::Number>("points_tick")) obj_data.points_tick = static_cast<int>(objective.get<jsonxx::Number>("points_tick"));
          if (objective.has<jsonxx::Number>("points_capture")) obj_data.points_capture = static_cast<int>(objective.get<jsonxx::Number>("points_capture"));
          if (objective.has<jsonxx::Number>("yaks_delivered")) obj_data.yaks_delivered = static_cast<int>(objective.get<jsonxx::Number>("yaks_delivered"));
          
          if (objective.has<jsonxx::Array>("guild_upgrades")) {
            for (auto& gu : objective.get<jsonxx::Array>("guild_upgrades").values()) {
              if (gu->is<jsonxx::Number>()) obj_data.guild_upgrades.push_back(static_cast<int>(gu->get<jsonxx::Number>()));
            }
          }
          
          map_data.objectives.push_back(obj_data);
        }
      }
      result.maps.push_back(map_data);
    }
  }

  if (o.has<jsonxx::Array>("skirmishes")) {
    for (auto& s : o.get<jsonxx::Array>("skirmishes").values()) {
      if (!s->is<jsonxx::Object>()) continue;
      auto skirmish_obj = s->get<jsonxx::Object>();
      WvwMatchSkirmish skirmish;
      
      if (skirmish_obj.has<jsonxx::Number>("id")) skirmish.id = static_cast<int>(skirmish_obj.get<jsonxx::Number>("id"));
      if (skirmish_obj.has<jsonxx::Object>("scores")) skirmish.scores = parse_string_int_map(skirmish_obj.get<jsonxx::Object>("scores"));
      
      if (skirmish_obj.has<jsonxx::Array>("map_scores")) {
        for (auto& ms : skirmish_obj.get<jsonxx::Array>("map_scores").values()) {
          if (!ms->is<jsonxx::Object>()) continue;
          auto map_score_obj = ms->get<jsonxx::Object>();
          WvwMatchSkirmishMap map_score;
          if (map_score_obj.has<jsonxx::String>("type")) map_score.type = map_score_obj.get<jsonxx::String>("type");
          if (map_score_obj.has<jsonxx::Object>("scores")) map_score.scores = parse_string_int_map(map_score_obj.get<jsonxx::Object>("scores"));
          skirmish.map_scores.push_back(map_score);
        }
      }
      result.skirmishes.push_back(skirmish);
    }
  }

  return result;
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

    WvwMatch match = ParseWvwMatch(wvw_objective_ids);
    if (!match.maps.empty()) {
      std::vector<WvwPoiUpdate> updates;
      for (const auto& map : match.maps) {
        for (const auto& objective : map.objectives) {
          if (objective.id.empty()) continue;

          WvwPoiUpdate update{.id_ = objective.id};
          if (objective.owner == "Red")
            update.owner_ = WvwPoiUpdate::Team::kRed;
          else if (objective.owner == "Green")
            update.owner_ = WvwPoiUpdate::Team::kGreen;
          else if (objective.owner == "Blue")
            update.owner_ = WvwPoiUpdate::Team::kBlue;
          else
            update.owner_ = WvwPoiUpdate::Team::kNone;

          time_t flip_time = 0;
          char flags = 0;
          ParseISO8601(objective.last_flipped.c_str(), flip_time, flags);
          update.last_flipped_ = flip_time;
          updates.push_back(update);
        }
      }
      wvw_poi_updates.push(updates);
    }
  });

  last_update_time = now;
}
