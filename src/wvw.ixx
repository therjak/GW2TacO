module;
#include <ctime>
#include <string>
#include <vector>

#include "src/base/lock_free_queue.h"

export module taco.wvw;

import math;

export class WvWObjective {
 public:
  std::string id;
  std::string type;
  std::string mapType;
  int mapID = 0;
  int objectiveID = 0;
  math::Vector3 coord;
  std::string marker;
  std::string chatLink;

  std::string name;
  std::string nameToken;
};

export struct WvWPOIUpdate {
  enum class Team {
    kRed,
    kGreen,
    kBlue,
    kNone,
  };
  std::string id;
  time_t lastFlipped = 0;
  Team owner;
};

export extern LockFreeQueue<std::vector<WvWPOIUpdate>> wvwPOIUpdates;

export void LoadWvWObjectives();
export void UpdateWvWStatus();
