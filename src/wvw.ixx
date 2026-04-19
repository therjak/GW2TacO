module;
#include <ctime>
#include <string>
#include <vector>

#include "src/base/lock_free_queue.h"

export module taco.wvw;

import math;

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

export extern LockFreeQueue<std::vector<WvwPoiUpdate>> wvw_poi_updates;
export extern std::vector<WvwObjective> wvw_objectives;

export void LoadWvwObjectives();
export void UpdateWvwStatus();
