module;

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/util/jsonxx.h"

module taco.web;

import math;

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

WvwMatch ParseWvwMatch(const std::string& json_data) {
  WvwMatch result;
  jsonxx::Object o;
  if (!o.parse(json_data)) return result;

  if (o.has<jsonxx::String>("id")) result.id = o.get<jsonxx::String>("id");
  if (o.has<jsonxx::String>("start_time"))
    result.start_time = o.get<jsonxx::String>("start_time");
  if (o.has<jsonxx::String>("end_time"))
    result.end_time = o.get<jsonxx::String>("end_time");

  auto parse_string_int_map = [](const jsonxx::Object& obj) {
    std::unordered_map<std::string, int> res;
    for (const auto& kv : obj.kv_map()) {
      if (kv.second->is<jsonxx::Number>()) {
        res[kv.first] = static_cast<int>(kv.second->get<jsonxx::Number>());
      }
    }
    return res;
  };

  if (o.has<jsonxx::Object>("scores"))
    result.scores = parse_string_int_map(o.get<jsonxx::Object>("scores"));
  if (o.has<jsonxx::Object>("worlds"))
    result.worlds = parse_string_int_map(o.get<jsonxx::Object>("worlds"));
  if (o.has<jsonxx::Object>("deaths"))
    result.deaths = parse_string_int_map(o.get<jsonxx::Object>("deaths"));
  if (o.has<jsonxx::Object>("kills"))
    result.kills = parse_string_int_map(o.get<jsonxx::Object>("kills"));
  if (o.has<jsonxx::Object>("victory_points"))
    result.victory_points =
        parse_string_int_map(o.get<jsonxx::Object>("victory_points"));

  if (o.has<jsonxx::Object>("all_worlds")) {
    auto aw = o.get<jsonxx::Object>("all_worlds");
    for (const auto& kv : aw.kv_map()) {
      if (kv.second->is<jsonxx::Array>()) {
        std::vector<int> worlds;
        for (auto& v : kv.second->get<jsonxx::Array>().values()) {
          if (v->is<jsonxx::Number>())
            worlds.push_back(static_cast<int>(v->get<jsonxx::Number>()));
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
      if (map_obj.has<jsonxx::Number>("id"))
        map_data.id = static_cast<int>(map_obj.get<jsonxx::Number>("id"));
      if (map_obj.has<jsonxx::String>("type"))
        map_data.type = map_obj.get<jsonxx::String>("type");
      if (map_obj.has<jsonxx::Object>("scores"))
        map_data.scores =
            parse_string_int_map(map_obj.get<jsonxx::Object>("scores"));
      if (map_obj.has<jsonxx::Object>("deaths"))
        map_data.deaths =
            parse_string_int_map(map_obj.get<jsonxx::Object>("deaths"));
      if (map_obj.has<jsonxx::Object>("kills"))
        map_data.kills =
            parse_string_int_map(map_obj.get<jsonxx::Object>("kills"));

      if (map_obj.has<jsonxx::Array>("bonuses")) {
        for (auto& b : map_obj.get<jsonxx::Array>("bonuses").values()) {
          if (!b->is<jsonxx::Object>()) continue;
          auto bonus_obj = b->get<jsonxx::Object>();
          WvwMatchBonus bonus;
          if (bonus_obj.has<jsonxx::String>("type"))
            bonus.type = bonus_obj.get<jsonxx::String>("type");
          if (bonus_obj.has<jsonxx::String>("owner"))
            bonus.owner = bonus_obj.get<jsonxx::String>("owner");
          map_data.bonuses.push_back(bonus);
        }
      }

      if (map_obj.has<jsonxx::Array>("objectives")) {
        for (auto& obj : map_obj.get<jsonxx::Array>("objectives").values()) {
          if (!obj->is<jsonxx::Object>()) continue;
          auto objective = obj->get<jsonxx::Object>();
          WvwMatchObjective obj_data;

          if (objective.has<jsonxx::String>("id"))
            obj_data.id = objective.get<jsonxx::String>("id");
          if (objective.has<jsonxx::String>("type"))
            obj_data.type = objective.get<jsonxx::String>("type");
          if (objective.has<jsonxx::String>("owner"))
            obj_data.owner = objective.get<jsonxx::String>("owner");
          if (objective.has<jsonxx::String>("last_flipped"))
            obj_data.last_flipped =
                objective.get<jsonxx::String>("last_flipped");
          if (objective.has<jsonxx::String>("claimed_by"))
            obj_data.claimed_by = objective.get<jsonxx::String>("claimed_by");
          if (objective.has<jsonxx::String>("claimed_at"))
            obj_data.claimed_at = objective.get<jsonxx::String>("claimed_at");
          if (objective.has<jsonxx::Number>("points_tick"))
            obj_data.points_tick =
                static_cast<int>(objective.get<jsonxx::Number>("points_tick"));
          if (objective.has<jsonxx::Number>("points_capture"))
            obj_data.points_capture = static_cast<int>(
                objective.get<jsonxx::Number>("points_capture"));
          if (objective.has<jsonxx::Number>("yaks_delivered"))
            obj_data.yaks_delivered = static_cast<int>(
                objective.get<jsonxx::Number>("yaks_delivered"));

          if (objective.has<jsonxx::Array>("guild_upgrades")) {
            for (auto& gu :
                 objective.get<jsonxx::Array>("guild_upgrades").values()) {
              if (gu->is<jsonxx::Number>())
                obj_data.guild_upgrades.push_back(
                    static_cast<int>(gu->get<jsonxx::Number>()));
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

      if (skirmish_obj.has<jsonxx::Number>("id"))
        skirmish.id = static_cast<int>(skirmish_obj.get<jsonxx::Number>("id"));
      if (skirmish_obj.has<jsonxx::Object>("scores"))
        skirmish.scores =
            parse_string_int_map(skirmish_obj.get<jsonxx::Object>("scores"));

      if (skirmish_obj.has<jsonxx::Array>("map_scores")) {
        for (auto& ms :
             skirmish_obj.get<jsonxx::Array>("map_scores").values()) {
          if (!ms->is<jsonxx::Object>()) continue;
          auto map_score_obj = ms->get<jsonxx::Object>();
          WvwMatchSkirmishMap map_score;
          if (map_score_obj.has<jsonxx::String>("type"))
            map_score.type = map_score_obj.get<jsonxx::String>("type");
          if (map_score_obj.has<jsonxx::Object>("scores"))
            map_score.scores = parse_string_int_map(
                map_score_obj.get<jsonxx::Object>("scores"));
          skirmish.map_scores.push_back(map_score);
        }
      }
      result.skirmishes.push_back(skirmish);
    }
  }

  return result;
}

bool ParseTransaction(const jsonxx::Object& object, TransactionItem* output) {
  if (!object.has<jsonxx::Number>("id") ||
      !object.has<jsonxx::Number>("item_id") ||
      !object.has<jsonxx::Number>("price") ||
      !object.has<jsonxx::Number>("quantity") ||
      !object.has<jsonxx::String>("created")) {
    return false;
  }
  output->transaction_id = int32_t(object.get<jsonxx::Number>("id"));
  output->item_id = int32_t(object.get<jsonxx::Number>("item_id"));
  output->price = int32_t(object.get<jsonxx::Number>("price"));
  output->quantity = int32_t(object.get<jsonxx::Number>("quantity"));
  output->created = object.get<jsonxx::String>("created");
  return true;
}

std::vector<TransactionItem> ParseTransactionList(const std::string& json_data,
                                                  const std::string& root_key) {
  std::vector<TransactionItem> result;
  jsonxx::Object json;
  json.parse(json_data);

  if (json.has<jsonxx::Array>(root_key)) {
    auto data = json.get<jsonxx::Array>(root_key).values();

    for (auto& x : data) {
      if (!x->is<jsonxx::Object>()) continue;

      const jsonxx::Object& item = x->get<jsonxx::Object>();

      TransactionItem item_data;
      if (ParseTransaction(item, &item_data)) {
        result.push_back(item_data);
      }
    }
  }

  return result;
}

std::vector<CommercePrice> ParseCommercePrices(const std::string& items_json) {
  std::vector<CommercePrice> result;
  jsonxx::Object item_json;
  item_json.parse(items_json);

  if (item_json.has<jsonxx::Array>("items")) {
    auto items = item_json.get<jsonxx::Array>("items").values();

    for (auto& x : items) {
      if (!x->is<jsonxx::Object>()) continue;

      const jsonxx::Object& item = x->get<jsonxx::Object>();

      if (!item.has<jsonxx::Number>("id") ||
          !item.has<jsonxx::Object>("buys") ||
          !item.has<jsonxx::Object>("sells")) {
        continue;
      }

      CommercePrice price;
      price.id = int32_t(item.get<jsonxx::Number>("id"));
      if (item.has<jsonxx::Boolean>("whitelisted")) {
        price.whitelisted = item.get<jsonxx::Boolean>("whitelisted");
      }

      jsonxx::Object buys_ = item.get<jsonxx::Object>("buys");
      jsonxx::Object sells_ = item.get<jsonxx::Object>("sells");

      if (buys_.has<jsonxx::Number>("quantity")) {
        price.buys.quantity = int32_t(buys_.get<jsonxx::Number>("quantity"));
      }
      if (buys_.has<jsonxx::Number>("unit_price")) {
        price.buys.unit_price =
            int32_t(buys_.get<jsonxx::Number>("unit_price"));
      }

      if (sells_.has<jsonxx::Number>("quantity")) {
        price.sells.quantity = int32_t(sells_.get<jsonxx::Number>("quantity"));
      }
      if (sells_.has<jsonxx::Number>("unit_price")) {
        price.sells.unit_price =
            int32_t(sells_.get<jsonxx::Number>("unit_price"));
      }

      result.push_back(price);
    }
  }

  return result;
}

std::vector<GW2ItemData> ParseGW2Items(const std::string& items_json) {
  std::vector<GW2ItemData> result;
  jsonxx::Object item_json;
  item_json.parse(items_json);

  if (item_json.has<jsonxx::Array>("items")) {
    auto items = item_json.get<jsonxx::Array>("items").values();

    for (auto& x : items) {
      if (!x->is<jsonxx::Object>()) continue;

      const jsonxx::Object& item = x->get<jsonxx::Object>();

      GW2ItemData item_data;
      if (!item.has<jsonxx::String>("name") ||
          !item.has<jsonxx::Number>("id")) {
        continue;
      }
      item_data.name = item.get<jsonxx::String>("name");
      item_data.item_id = int32_t(item.get<jsonxx::Number>("id"));
      if (item.has<jsonxx::String>("icon")) {
        item_data.icon_file = item.get<jsonxx::String>("icon");
      }

      result.push_back(item_data);
    }
  }

  return result;
}

std::unordered_map<int32_t, Achievement> ParseAchievements(
    const std::string& achievements_data) {
  std::unordered_map<int32_t, Achievement> result;

  jsonxx::Object json;
  json.parse(achievements_data);

  if (!json.has<jsonxx::Array>("achievements")) return result;

  auto achi_data = json.get<jsonxx::Array>("achievements").values();

  for (auto& x : achi_data) {
    if (!x->is<jsonxx::Object>()) continue;
    auto& data = x->get<jsonxx::Object>();

    if (!data.has<jsonxx::Boolean>("done")) continue;
    bool done = data.get<jsonxx::Boolean>("done");

    if (!data.has<jsonxx::Number>("id")) continue;
    int32_t achi_id = int32_t(data.get<jsonxx::Number>("id"));
    result[achi_id].done = done;

    if (!done && data.has<jsonxx::Array>("bits")) {
      auto& bit_array = result[achi_id].bits;
      auto bits = data.get<jsonxx::Array>("bits").values();
      for (auto& bit : bits) {
        if (!bit->is<jsonxx::Number>()) continue;
        bit_array.push_back(static_cast<int32_t>(bit->get<jsonxx::Number>()));
      }
    } else if (done) {
      result[achi_id].bits.clear();
    }
  }

  return result;
}

AccountAchievement ParseAccountAchievement(const std::string& json_data) {
  AccountAchievement result;
  jsonxx::Object obj;

  jsonxx::Array arr;
  if (arr.parse(json_data) && !arr.values().empty() &&
      arr.values()[0]->is<jsonxx::Object>()) {
    obj = arr.values()[0]->get<jsonxx::Object>();
  } else if (!obj.parse(json_data)) {
    return result;
  }

  if (obj.has<jsonxx::Number>("id")) {
    result.id = static_cast<int32_t>(obj.get<jsonxx::Number>("id"));
  }
  if (obj.has<jsonxx::Number>("current")) {
    result.current = static_cast<int32_t>(obj.get<jsonxx::Number>("current"));
  }
  if (obj.has<jsonxx::Number>("max")) {
    result.max = static_cast<int32_t>(obj.get<jsonxx::Number>("max"));
  }
  if (obj.has<jsonxx::Boolean>("done")) {
    result.done = obj.get<jsonxx::Boolean>("done");
  }
  if (obj.has<jsonxx::Number>("repeated")) {
    result.repeated = static_cast<int32_t>(obj.get<jsonxx::Number>("repeated"));
  }
  if (obj.has<jsonxx::Boolean>("unlocked")) {
    result.unlocked = obj.get<jsonxx::Boolean>("unlocked");
  }
  if (obj.has<jsonxx::Array>("bits")) {
    auto bits = obj.get<jsonxx::Array>("bits").values();
    for (auto& bit : bits) {
      if (bit->is<jsonxx::Number>()) {
        result.bits.push_back(static_cast<int32_t>(bit->get<jsonxx::Number>()));
      }
    }
  }

  return result;
}

TokenInfo ParseTokenInfo(const std::string& json_data) {
  TokenInfo result;
  jsonxx::Object json;
  if (!json.parse(json_data)) return result;

  if (json.has<jsonxx::String>("id")) {
    result.id = json.get<jsonxx::String>("id");
  }
  if (json.has<jsonxx::String>("name")) {
    result.name = json.get<jsonxx::String>("name");
  }
  if (json.has<jsonxx::Array>("permissions")) {
    std::vector<std::string> perms;
    auto& values = json.get<jsonxx::Array>("permissions").values();
    for (auto v : values) {
      if (v->is<jsonxx::String>()) {
        perms.push_back(v->get<jsonxx::String>());
      }
    }
    result.permissions = std::move(perms);
  }
  return result;
}

AccountInfo ParseAccountInfo(const std::string& json_data) {
  AccountInfo result;
  jsonxx::Object json;
  if (!json.parse(json_data)) return result;

  if (json.has<jsonxx::String>("id"))
    result.id = json.get<jsonxx::String>("id");
  if (json.has<jsonxx::String>("name"))
    result.name = json.get<jsonxx::String>("name");
  if (json.has<jsonxx::Number>("age"))
    result.age = static_cast<int32_t>(json.get<jsonxx::Number>("age"));
  if (json.has<jsonxx::Number>("world"))
    result.world = static_cast<int32_t>(json.get<jsonxx::Number>("world"));

  auto parse_string_array =
      [](const jsonxx::Object& obj,
         const std::string& key) -> std::optional<std::vector<std::string>> {
    if (!obj.has<jsonxx::Array>(key)) return std::nullopt;
    std::vector<std::string> res;
    for (auto v : obj.get<jsonxx::Array>(key).values()) {
      if (v->is<jsonxx::String>()) res.push_back(v->get<jsonxx::String>());
    }
    return res;
  };

  result.guilds = parse_string_array(json, "guilds");
  result.guild_leader = parse_string_array(json, "guild_leader");
  if (json.has<jsonxx::String>("created"))
    result.created = json.get<jsonxx::String>("created");
  result.access = parse_string_array(json, "access");

  if (json.has<jsonxx::Boolean>("commander"))
    result.commander = json.get<jsonxx::Boolean>("commander");
  if (json.has<jsonxx::Number>("fractal_level"))
    result.fractal_level =
        static_cast<int32_t>(json.get<jsonxx::Number>("fractal_level"));
  if (json.has<jsonxx::Number>("daily_ap"))
    result.daily_ap =
        static_cast<int32_t>(json.get<jsonxx::Number>("daily_ap"));
  if (json.has<jsonxx::Number>("monthly_ap"))
    result.monthly_ap =
        static_cast<int32_t>(json.get<jsonxx::Number>("monthly_ap"));
  if (json.has<jsonxx::Number>("wvw_rank"))
    result.wvw_rank =
        static_cast<int32_t>(json.get<jsonxx::Number>("wvw_rank"));
  if (json.has<jsonxx::String>("last_modified"))
    result.last_modified = json.get<jsonxx::String>("last_modified");

  return result;
}

std::unordered_set<std::string> ParseArray(const std::string& json_data) {
  jsonxx::Array arr;
  std::unordered_set<std::string> result;
  if (!arr.parse(json_data)) return result;

  for (auto v : arr.values()) {
    if (v->is<jsonxx::String>()) {
      result.emplace(v->get<jsonxx::String>());
    }
  }
  return result;
}
