module;

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "nlohmann/json.hpp"

module taco.web;

import math;

std::vector<WvwObjectiveData> ParseWvwObjectives(const std::string& json_data) {
  std::vector<WvwObjectiveData> result;
  nlohmann::json wvw_objs = nlohmann::json::parse(json_data, nullptr, false);
  if (wvw_objs.is_discarded() || !wvw_objs.is_array()) return result;

  for (const auto& obj : wvw_objs) {
    if (!obj.is_object()) continue;

    WvwObjectiveData data;
    if (obj.contains("id") && obj["id"].is_string()) data.id = obj["id"].get<std::string>();
    if (obj.contains("name") && obj["name"].is_string()) data.name = obj["name"].get<std::string>();
    if (obj.contains("type") && obj["type"].is_string()) data.type = obj["type"].get<std::string>();
    if (obj.contains("sector_id") && obj["sector_id"].is_number()) data.sector_id = obj["sector_id"].get<int>();
    if (obj.contains("map_id") && obj["map_id"].is_number()) data.map_id = obj["map_id"].get<int>();
    if (obj.contains("map_type") && obj["map_type"].is_string()) data.map_type = obj["map_type"].get<std::string>();
    if (obj.contains("marker") && obj["marker"].is_string()) data.marker = obj["marker"].get<std::string>();
    if (obj.contains("chat_link") && obj["chat_link"].is_string()) data.chat_link = obj["chat_link"].get<std::string>();
    if (obj.contains("upgrade_id") && obj["upgrade_id"].is_number()) data.upgrade_id = obj["upgrade_id"].get<int>();

    if (obj.contains("coord") && obj["coord"].is_array()) {
      for (const auto& v : obj["coord"]) {
        if (v.is_number()) {
          data.coord.push_back(v.get<float>());
        }
      }
    }
    if (obj.contains("label_coord") && obj["label_coord"].is_array()) {
      for (const auto& v : obj["label_coord"]) {
        if (v.is_number()) {
          data.label_coord.push_back(v.get<float>());
        }
      }
    }
    result.push_back(data);
  }
  return result;
}

WvwMapData ParseWvwMapData(const std::string& json_data) {
  WvwMapData data;
  nlohmann::json map = nlohmann::json::parse(json_data, nullptr, false);
  if (map.is_discarded() || !map.is_object()) return data;

  if (map.contains("id") && map["id"].is_number()) data.id = map["id"].get<int>();
  if (map.contains("name") && map["name"].is_string()) data.name = map["name"].get<std::string>();
  if (map.contains("min_level") && map["min_level"].is_number()) data.min_level = map["min_level"].get<int>();
  if (map.contains("max_level") && map["max_level"].is_number()) data.max_level = map["max_level"].get<int>();
  if (map.contains("default_floor") && map["default_floor"].is_number()) data.default_floor = map["default_floor"].get<int>();
  if (map.contains("type") && map["type"].is_string()) data.type = map["type"].get<std::string>();

  if (map.contains("floors") && map["floors"].is_array()) {
    for (const auto& v : map["floors"]) {
      if (v.is_number()) data.floors.push_back(v.get<int>());
    }
  }

  if (map.contains("region_id") && map["region_id"].is_number()) data.region_id = map["region_id"].get<int>();
  if (map.contains("region_name") && map["region_name"].is_string()) data.region_name = map["region_name"].get<std::string>();
  if (map.contains("continent_id") && map["continent_id"].is_number()) data.continent_id = map["continent_id"].get<int>();
  if (map.contains("continent_name") && map["continent_name"].is_string()) data.continent_name = map["continent_name"].get<std::string>();

  auto parse_rect = [](const nlohmann::json& arr) -> std::optional<math::Rect> {
    if (!arr.is_array() || arr.size() != 2) return std::nullopt;
    int rect_values[4];
    int rect_cnt = 0;
    for (int x = 0; x < 2; x++) {
      if (!arr[x].is_array() || arr[x].size() != 2) return std::nullopt;
      for (int y = 0; y < 2; y++) {
        if (!arr[x][y].is_number()) return std::nullopt;
        rect_values[rect_cnt++] = arr[x][y].get<int>();
      }
    }
    return math::Rect(rect_values[0], rect_values[1], rect_values[2], rect_values[3]);
  };

  if (map.contains("map_rect")) {
    data.map_rect = parse_rect(map["map_rect"]);
  }
  if (map.contains("continent_rect")) {
    data.continent_rect = parse_rect(map["continent_rect"]);
  }

  return data;
}

WvwMatch ParseWvwMatch(const std::string& json_data) {
  WvwMatch result;
  nlohmann::json o = nlohmann::json::parse(json_data, nullptr, false);
  if (o.is_discarded() || !o.is_object()) return result;

  if (o.contains("id") && o["id"].is_string()) result.id = o["id"].get<std::string>();
  if (o.contains("start_time") && o["start_time"].is_string()) result.start_time = o["start_time"].get<std::string>();
  if (o.contains("end_time") && o["end_time"].is_string()) result.end_time = o["end_time"].get<std::string>();

  auto parse_string_int_map = [](const nlohmann::json& obj) {
    std::unordered_map<std::string, int> res;
    if (obj.is_object()) {
      for (auto it = obj.begin(); it != obj.end(); ++it) {
        if (it.value().is_number()) {
          res[it.key()] = it.value().get<int>();
        }
      }
    }
    return res;
  };

  if (o.contains("scores")) result.scores = parse_string_int_map(o["scores"]);
  if (o.contains("worlds")) result.worlds = parse_string_int_map(o["worlds"]);
  if (o.contains("deaths")) result.deaths = parse_string_int_map(o["deaths"]);
  if (o.contains("kills")) result.kills = parse_string_int_map(o["kills"]);
  if (o.contains("victory_points")) result.victory_points = parse_string_int_map(o["victory_points"]);

  if (o.contains("all_worlds") && o["all_worlds"].is_object()) {
    for (auto it = o["all_worlds"].begin(); it != o["all_worlds"].end(); ++it) {
      if (it.value().is_array()) {
        std::vector<int> worlds;
        for (const auto& v : it.value()) {
          if (v.is_number()) worlds.push_back(v.get<int>());
        }
        result.all_worlds[it.key()] = worlds;
      }
    }
  }

  if (o.contains("maps") && o["maps"].is_array()) {
    for (const auto& m : o["maps"]) {
      if (!m.is_object()) continue;
      WvwMatchMap map_data;
      if (m.contains("id") && m["id"].is_number()) map_data.id = m["id"].get<int>();
      if (m.contains("type") && m["type"].is_string()) map_data.type = m["type"].get<std::string>();
      if (m.contains("scores")) map_data.scores = parse_string_int_map(m["scores"]);
      if (m.contains("deaths")) map_data.deaths = parse_string_int_map(m["deaths"]);
      if (m.contains("kills")) map_data.kills = parse_string_int_map(m["kills"]);

      if (m.contains("bonuses") && m["bonuses"].is_array()) {
        for (const auto& b : m["bonuses"]) {
          if (!b.is_object()) continue;
          WvwMatchBonus bonus;
          if (b.contains("type") && b["type"].is_string()) bonus.type = b["type"].get<std::string>();
          if (b.contains("owner") && b["owner"].is_string()) bonus.owner = b["owner"].get<std::string>();
          map_data.bonuses.push_back(bonus);
        }
      }

      if (m.contains("objectives") && m["objectives"].is_array()) {
        for (const auto& obj : m["objectives"]) {
          if (!obj.is_object()) continue;
          WvwMatchObjective obj_data;

          if (obj.contains("id") && obj["id"].is_string()) obj_data.id = obj["id"].get<std::string>();
          if (obj.contains("type") && obj["type"].is_string()) obj_data.type = obj["type"].get<std::string>();
          if (obj.contains("owner") && obj["owner"].is_string()) obj_data.owner = obj["owner"].get<std::string>();
          if (obj.contains("last_flipped") && obj["last_flipped"].is_string()) obj_data.last_flipped = obj["last_flipped"].get<std::string>();
          if (obj.contains("claimed_by") && obj["claimed_by"].is_string()) obj_data.claimed_by = obj["claimed_by"].get<std::string>();
          if (obj.contains("claimed_at") && obj["claimed_at"].is_string()) obj_data.claimed_at = obj["claimed_at"].get<std::string>();
          if (obj.contains("points_tick") && obj["points_tick"].is_number()) obj_data.points_tick = obj["points_tick"].get<int>();
          if (obj.contains("points_capture") && obj["points_capture"].is_number()) obj_data.points_capture = obj["points_capture"].get<int>();
          if (obj.contains("yaks_delivered") && obj["yaks_delivered"].is_number()) obj_data.yaks_delivered = obj["yaks_delivered"].get<int>();

          if (obj.contains("guild_upgrades") && obj["guild_upgrades"].is_array()) {
            for (const auto& gu : obj["guild_upgrades"]) {
              if (gu.is_number()) obj_data.guild_upgrades.push_back(gu.get<int>());
            }
          }

          map_data.objectives.push_back(obj_data);
        }
      }
      result.maps.push_back(map_data);
    }
  }

  if (o.contains("skirmishes") && o["skirmishes"].is_array()) {
    for (const auto& s : o["skirmishes"]) {
      if (!s.is_object()) continue;
      WvwMatchSkirmish skirmish;

      if (s.contains("id") && s["id"].is_number()) skirmish.id = s["id"].get<int>();
      if (s.contains("scores")) skirmish.scores = parse_string_int_map(s["scores"]);

      if (s.contains("map_scores") && s["map_scores"].is_array()) {
        for (const auto& ms : s["map_scores"]) {
          if (!ms.is_object()) continue;
          WvwMatchSkirmishMap map_score;
          if (ms.contains("type") && ms["type"].is_string()) map_score.type = ms["type"].get<std::string>();
          if (ms.contains("scores")) map_score.scores = parse_string_int_map(ms["scores"]);
          skirmish.map_scores.push_back(map_score);
        }
      }
      result.skirmishes.push_back(skirmish);
    }
  }

  return result;
}

bool ParseTransaction(const nlohmann::json& object, TransactionItem* output) {
  if (!object.is_object() ||
      !object.contains("id") || !object["id"].is_number() ||
      !object.contains("item_id") || !object["item_id"].is_number() ||
      !object.contains("price") || !object["price"].is_number() ||
      !object.contains("quantity") || !object["quantity"].is_number() ||
      !object.contains("created") || !object["created"].is_string()) {
    return false;
  }
  output->transaction_id = object["id"].get<int64_t>();
  output->item_id = object["item_id"].get<int32_t>();
  output->price = object["price"].get<int32_t>();
  output->quantity = object["quantity"].get<int32_t>();
  output->created = object["created"].get<std::string>();
  return true;
}

std::vector<TransactionItem> ParseTransactionList(const std::string& json_data) {
  std::vector<TransactionItem> result;
  nlohmann::json json = nlohmann::json::parse(json_data, nullptr, false);
  if (json.is_discarded() || !json.is_array()) return result;

  for (const auto& x : json) {
    TransactionItem item_data;
    if (ParseTransaction(x, &item_data)) {
      result.push_back(item_data);
    }
  }

  return result;
}

std::vector<CommercePrice> ParseCommercePrices(const std::string& items_json) {
  std::vector<CommercePrice> result;
  nlohmann::json item_json = nlohmann::json::parse(items_json, nullptr, false);
  if (item_json.is_discarded() || !item_json.is_object()) return result;

  if (item_json.contains("items") && item_json["items"].is_array()) {
    for (const auto& item : item_json["items"]) {
      if (!item.is_object()) continue;

      if (!item.contains("id") || !item["id"].is_number() ||
          !item.contains("buys") || !item["buys"].is_object() ||
          !item.contains("sells") || !item["sells"].is_object()) {
        continue;
      }

      CommercePrice price;
      price.id = item["id"].get<int32_t>();
      if (item.contains("whitelisted") && item["whitelisted"].is_boolean()) {
        price.whitelisted = item["whitelisted"].get<bool>();
      }

      const auto& buys_ = item["buys"];
      const auto& sells_ = item["sells"];

      if (buys_.contains("quantity") && buys_["quantity"].is_number()) {
        price.buys.quantity = buys_["quantity"].get<int32_t>();
      }
      if (buys_.contains("unit_price") && buys_["unit_price"].is_number()) {
        price.buys.unit_price = buys_["unit_price"].get<int32_t>();
      }

      if (sells_.contains("quantity") && sells_["quantity"].is_number()) {
        price.sells.quantity = sells_["quantity"].get<int32_t>();
      }
      if (sells_.contains("unit_price") && sells_["unit_price"].is_number()) {
        price.sells.unit_price = sells_["unit_price"].get<int32_t>();
      }

      result.push_back(price);
    }
  }

  return result;
}

std::vector<GW2ItemData> ParseGW2Items(const std::string& items_json) {
  std::vector<GW2ItemData> result;
  nlohmann::json item_json = nlohmann::json::parse(items_json, nullptr, false);
  if (item_json.is_discarded() || !item_json.is_object()) return result;

  if (item_json.contains("items") && item_json["items"].is_array()) {
    for (const auto& item : item_json["items"]) {
      if (!item.is_object()) continue;

      GW2ItemData item_data;
      if (!item.contains("name") || !item["name"].is_string() ||
          !item.contains("id") || !item["id"].is_number()) {
        continue;
      }
      item_data.name = item["name"].get<std::string>();
      item_data.item_id = item["id"].get<int32_t>();
      if (item.contains("icon") && item["icon"].is_string()) {
        item_data.icon_file = item["icon"].get<std::string>();
      }

      result.push_back(item_data);
    }
  }

  return result;
}

std::unordered_map<int32_t, Achievement> ParseAchievements(
    const std::string& achievements_data) {
  std::unordered_map<int32_t, Achievement> result;

  nlohmann::json json = nlohmann::json::parse(achievements_data, nullptr, false);
  if (json.is_discarded() || !json.is_object()) return result;

  if (!json.contains("achievements") || !json["achievements"].is_array()) return result;

  for (const auto& data : json["achievements"]) {
    if (!data.is_object()) continue;

    if (!data.contains("done") || !data["done"].is_boolean()) continue;
    bool done = data["done"].get<bool>();

    if (!data.contains("id") || !data["id"].is_number()) continue;
    int32_t achi_id = data["id"].get<int32_t>();
    result[achi_id].done = done;

    if (!done && data.contains("bits") && data["bits"].is_array()) {
      auto& bit_array = result[achi_id].bits;
      for (const auto& bit : data["bits"]) {
        if (!bit.is_number()) continue;
        bit_array.push_back(bit.get<int32_t>());
      }
    } else if (done) {
      result[achi_id].bits.clear();
    }
  }

  return result;
}

AccountAchievement ParseAccountAchievement(const std::string& json_data) {
  AccountAchievement result;
  nlohmann::json parsed = nlohmann::json::parse(json_data, nullptr, false);
  if (parsed.is_discarded()) return result;

  nlohmann::json obj;
  if (parsed.is_array() && !parsed.empty() && parsed[0].is_object()) {
    obj = parsed[0];
  } else if (parsed.is_object()) {
    obj = parsed;
  } else {
    return result;
  }

  if (obj.contains("id") && obj["id"].is_number()) {
    result.id = obj["id"].get<int32_t>();
  }
  if (obj.contains("current") && obj["current"].is_number()) {
    result.current = obj["current"].get<int32_t>();
  }
  if (obj.contains("max") && obj["max"].is_number()) {
    result.max = obj["max"].get<int32_t>();
  }
  if (obj.contains("done") && obj["done"].is_boolean()) {
    result.done = obj["done"].get<bool>();
  }
  if (obj.contains("repeated") && obj["repeated"].is_number()) {
    result.repeated = obj["repeated"].get<int32_t>();
  }
  if (obj.contains("unlocked") && obj["unlocked"].is_boolean()) {
    result.unlocked = obj["unlocked"].get<bool>();
  }
  if (obj.contains("bits") && obj["bits"].is_array()) {
    for (const auto& bit : obj["bits"]) {
      if (bit.is_number()) {
        result.bits.push_back(bit.get<int32_t>());
      }
    }
  }

  return result;
}

TokenInfo ParseTokenInfo(const std::string& json_data) {
  TokenInfo result;
  nlohmann::json json = nlohmann::json::parse(json_data, nullptr, false);
  if (json.is_discarded() || !json.is_object()) return result;

  if (json.contains("id") && json["id"].is_string()) {
    result.id = json["id"].get<std::string>();
  }
  if (json.contains("name") && json["name"].is_string()) {
    result.name = json["name"].get<std::string>();
  }
  if (json.contains("permissions") && json["permissions"].is_array()) {
    std::vector<std::string> perms;
    for (const auto& v : json["permissions"]) {
      if (v.is_string()) {
        perms.push_back(v.get<std::string>());
      }
    }
    result.permissions = std::move(perms);
  }
  return result;
}

AccountInfo ParseAccountInfo(const std::string& json_data) {
  AccountInfo result;
  nlohmann::json json = nlohmann::json::parse(json_data, nullptr, false);
  if (json.is_discarded() || !json.is_object()) return result;

  if (json.contains("id") && json["id"].is_string()) result.id = json["id"].get<std::string>();
  if (json.contains("name") && json["name"].is_string()) result.name = json["name"].get<std::string>();
  if (json.contains("age") && json["age"].is_number()) result.age = json["age"].get<int32_t>();
  if (json.contains("world") && json["world"].is_number()) result.world = json["world"].get<int32_t>();

  auto parse_string_array =
      [](const nlohmann::json& obj,
         const std::string& key) -> std::optional<std::vector<std::string>> {
    if (!obj.contains(key) || !obj[key].is_array()) return std::nullopt;
    std::vector<std::string> res;
    for (const auto& v : obj[key]) {
      if (v.is_string()) res.push_back(v.get<std::string>());
    }
    return res;
  };

  result.guilds = parse_string_array(json, "guilds");
  result.guild_leader = parse_string_array(json, "guild_leader");
  if (json.contains("created") && json["created"].is_string()) result.created = json["created"].get<std::string>();
  result.access = parse_string_array(json, "access");

  if (json.contains("commander") && json["commander"].is_boolean()) result.commander = json["commander"].get<bool>();
  if (json.contains("fractal_level") && json["fractal_level"].is_number()) result.fractal_level = json["fractal_level"].get<int32_t>();
  if (json.contains("daily_ap") && json["daily_ap"].is_number()) result.daily_ap = json["daily_ap"].get<int32_t>();
  if (json.contains("monthly_ap") && json["monthly_ap"].is_number()) result.monthly_ap = json["monthly_ap"].get<int32_t>();
  if (json.contains("wvw_rank") && json["wvw_rank"].is_number()) result.wvw_rank = json["wvw_rank"].get<int32_t>();
  if (json.contains("last_modified") && json["last_modified"].is_string()) result.last_modified = json["last_modified"].get<std::string>();

  return result;
}

std::unordered_set<std::string> ParseArray(const std::string& json_data) {
  std::unordered_set<std::string> result;
  nlohmann::json arr = nlohmann::json::parse(json_data, nullptr, false);
  if (arr.is_discarded() || !arr.is_array()) return result;

  for (const auto& v : arr) {
    if (v.is_string()) {
      result.emplace(v.get<std::string>());
    }
  }
  return result;
}
