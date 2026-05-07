module;

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

export module taco.web;

import math;
import whiteboard;

export struct WvwObjectiveData {
  std::string id;
  std::string name;
  std::string type;
  int sector_id = 0;
  int map_id = 0;
  std::string map_type;
  std::vector<float> coord;
  std::vector<float> label_coord;
  std::string marker;
  std::string chat_link;
  int upgrade_id = 0;
};

export struct WvwMapData {
  int id = 0;
  std::string name;
  int min_level = 0;
  int max_level = 0;
  int default_floor = 0;
  std::string type;
  std::vector<int> floors;
  int region_id = 0;
  std::string region_name;
  int continent_id = 0;
  std::string continent_name;
  std::optional<math::Rect> map_rect;
  std::optional<math::Rect> continent_rect;
};

export struct WvwMatchObjective {
  std::string id;
  std::string type;
  std::string owner;
  std::string last_flipped;
  std::string claimed_by;
  std::string claimed_at;
  int points_tick = 0;
  int points_capture = 0;
  std::vector<int> guild_upgrades;
  int yaks_delivered = 0;
};

export struct WvwMatchBonus {
  std::string type;
  std::string owner;
};

export struct WvwMatchMap {
  int id = 0;
  std::string type;
  std::unordered_map<std::string, int> scores;
  std::vector<WvwMatchBonus> bonuses;
  std::unordered_map<std::string, int> deaths;
  std::unordered_map<std::string, int> kills;
  std::vector<WvwMatchObjective> objectives;
};

export struct WvwMatchSkirmishMap {
  std::string type;
  std::unordered_map<std::string, int> scores;
};

export struct WvwMatchSkirmish {
  int id = 0;
  std::unordered_map<std::string, int> scores;
  std::vector<WvwMatchSkirmishMap> map_scores;
};

export struct WvwMatch {
  std::string id;
  std::string start_time;
  std::string end_time;
  std::unordered_map<std::string, int> scores;
  std::unordered_map<std::string, int> worlds;
  std::unordered_map<std::string, std::vector<int>> all_worlds;
  std::unordered_map<std::string, int> deaths;
  std::unordered_map<std::string, int> kills;
  std::unordered_map<std::string, int> victory_points;
  std::vector<WvwMatchMap> maps;
  std::vector<WvwMatchSkirmish> skirmishes;
};

export std::vector<WvwObjectiveData> ParseWvwObjectives(
    const std::string& json_data);
export WvwMapData ParseWvwMapData(const std::string& json_data);
export WvwMatch ParseWvwMatch(const std::string& json_data);

export struct TransactionItem {
  int32_t transaction_id = 0;
  int32_t item_id = 0;
  int32_t price = 0;
  int32_t quantity = 0;
  std::string created;
};

export struct CommercePriceListing {
  int32_t quantity = 0;
  int32_t unit_price = 0;
};

export struct CommercePrice {
  int32_t id = 0;
  bool whitelisted = false;
  CommercePriceListing buys;
  CommercePriceListing sells;
};

export struct GW2ItemData {
  int32_t item_id = 0;
  std::string name;
  gui::WBATLASHANDLE icon = 0;
  int32_t buy_price = 0;
  int32_t sell_price = 0;
  std::string icon_file;
};

export std::vector<TransactionItem> ParseTransactionList(
    const std::string& json_data, const std::string& root_key);
export std::vector<CommercePrice> ParseCommercePrices(
    const std::string& items_json);
export std::vector<GW2ItemData> ParseGW2Items(const std::string& items_json);

export struct Achievement {
  bool done = false;
  std::vector<int32_t> bits;
};

export std::unordered_map<int32_t, Achievement> ParseAchievements(
    const std::string& achievements_data);