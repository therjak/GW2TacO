module;
#include <ctime>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/base/lock_free_queue.h"
#include "src/gw2_tactical.h"

export module taco.wvw;

import math;

export struct WvwObjective {
  std::string id;
  std::string type;
  std::string map_type;
  int map_id = 0;
  int objective_id = 0;
  math::Vector3 coord;
  std::string marker;
  std::string chat_link;

  std::string name;
  std::string name_token;
};

export struct WvwPoiUpdate {
  enum class Team {
    kRed,
    kGreen,
    kBlue,
    kNone,
  };
  std::string id;
  time_t last_flipped = 0;
  Team owner;
};

export LockFreeQueue<std::vector<WvwPoiUpdate>> wvw_poi_updates;
export std::vector<WvwObjective> wvw_objectives;

export void LoadWvwObjectives();
export void UpdateWvwStatus();
export bool wvw_can_be_rendered = false;
export std::unordered_map<std::string, POI> wvw_pois;
