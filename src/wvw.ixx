module;
#include <ctime>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/base/lock_free_queue.h"
#include "src/gw2_tactical.h"

export module taco.wvw;

import math;

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

export class WvwObjective {
 public:
  std::string id_;
  std::string type_;
  std::string map_type_;
  int map_id_ = 0;
  int objective_id_ = 0;
  math::Vector3 coord_;
  std::string marker_;
  std::string chat_link_;

  std::string name_;
  std::string name_token_;
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

export struct WvwPoiUpdate {
  enum class Team {
    kRed,
    kGreen,
    kBlue,
    kNone,
  };
  std::string id_;
  time_t last_flipped_ = 0;
  Team owner_;
};

export LockFreeQueue<std::vector<WvwPoiUpdate>> wvw_poi_updates;
export std::vector<WvwObjective> wvw_objectives;

export void LoadWvwObjectives();
export void UpdateWvwStatus();
export std::vector<WvwObjectiveData> ParseWvwObjectives(
    const std::string& json_data);
export WvwMapData ParseWvwMapData(const std::string& json_data);
export WvwMatch ParseWvwMatch(const std::string& json_data);
export bool wvw_can_be_rendered = false;
export std::unordered_map<std::string, POI> wvw_pois;
