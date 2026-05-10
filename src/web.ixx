module;

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
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

// https://api.guildwars2.com/v2/wvw/objectives
// https://wiki.guildwars2.com/wiki/API:2/wvw/objectives
export std::vector<WvwObjectiveData> ParseWvwObjectives(
    const std::string& json_data);

// https://api.guildwars2.com/v2/maps
// https://wiki.guildwars2.com/wiki/API:2/maps
export WvwMapData ParseWvwMapData(const std::string& json_data);

// https://api.guildwars2.com/v2/wvw/matches
// https://wiki.guildwars2.com/wiki/API:2/wvw/matches
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

// https://api.guildwars2.com/v2/commerce/transactions/current/buys
// https://api.guildwars2.com/v2/commerce/transactions/current/sells
// https://wiki.guildwars2.com/wiki/API:2/commerce/transactions
export std::vector<TransactionItem> ParseTransactionList(
    const std::string& json_data, const std::string& root_key);

// https://api.guildwars2.com/v2/commerce/prices
// https://wiki.guildwars2.com/wiki/API:2/commerce/prices
export std::vector<CommercePrice> ParseCommercePrices(
    const std::string& items_json);

// https://api.guildwars2.com/v2/items
// https://wiki.guildwars2.com/wiki/API:2/items
export std::vector<GW2ItemData> ParseGW2Items(const std::string& items_json);

export struct Achievement {
  bool done = false;
  std::vector<int32_t> bits;
};

// https://api.guildwars2.com/v2/account/achievements
// https://wiki.guildwars2.com/wiki/API:2/account/achievements
export std::unordered_map<int32_t, Achievement> ParseAchievements(
    const std::string& achievements_data);

export struct AccountAchievement {
  int32_t id = 0;
  std::optional<int32_t> current;
  std::optional<int32_t> max;
  bool done = false;
  std::vector<int32_t> bits;
  std::optional<int32_t> repeated;
  std::optional<bool> unlocked;
};

// https://api.guildwars2.com/v2/account/achievements?ids=12345
// https://wiki.guildwars2.com/wiki/API:2/account/achievements
export AccountAchievement ParseAccountAchievement(const std::string& json_data);

export struct TokenInfo {
  std::string id;
  std::optional<std::string> name;
  std::optional<std::vector<std::string>> permissions;
};

// https://api.guildwars2.com/v2/tokeninfo
// https://wiki.guildwars2.com/wiki/API:2/tokeninfo
export TokenInfo ParseTokenInfo(const std::string& json_data);

export struct AccountInfo {
  std::string id;
  std::optional<std::string> name;
  std::optional<int32_t> age;
  std::optional<int32_t> world;
  std::optional<std::vector<std::string>> guilds;
  std::optional<std::vector<std::string>> guild_leader;
  std::optional<std::string> created;
  std::optional<std::vector<std::string>> access;
  std::optional<bool> commander;
  std::optional<int32_t> fractal_level;
  std::optional<int32_t> daily_ap;
  std::optional<int32_t> monthly_ap;
  std::optional<int32_t> wvw_rank;
  std::optional<std::string> last_modified;
};

// https://api.guildwars2.com/v2/account
// https://wiki.guildwars2.com/wiki/API:2/account
export AccountInfo ParseAccountInfo(const std::string& json_data);

// https://api.guildwars2.com/v2/account/dungeons
// https://api.guildwars2.com/v2/account/mapchests
// https://api.guildwars2.com/v2/account/raids
// https://api.guildwars2.com/v2/account/worldbosses
// https://api.guildwars2.com/v2/characters
// https://wiki.guildwars2.com/wiki/API:2/account/dungeons
// https://wiki.guildwars2.com/wiki/API:2/account/mapchests
// https://wiki.guildwars2.com/wiki/API:2/account/raids
// https://wiki.guildwars2.com/wiki/API:2/account/worldbosses
// https://wiki.guildwars2.com/wiki/API:2/characters
export std::unordered_set<std::string> ParseArray(const std::string& json_data);
