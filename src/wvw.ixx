module;
#include <ctime>
#include <string>
#include <vector>

#include "src/base/lock_free_queue.h"

export module taco.wvw;

import math;

export class WvwObjective {
 public:
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

export extern LockFreeQueue<std::vector<WvwPoiUpdate>> wvw_poi_updates;

export void LoadWvwObjectives();
export void UpdateWvwStatus();
