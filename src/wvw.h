#pragma once
#include <string>
#include <vector>

#include "src/base/lock_free_queue.h"
#include "src/base/vector.h"

class WvWObjective {
 public:
  std::string id;
  std::string type;
  std::string mapType;
  int mapID = 0;
  int objectiveID = 0;
  math::CVector3 coord;
  std::string marker;
  std::string chatLink;

  std::string name;
  std::string nameToken;
};

struct WvWPOIUpdate {
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

extern LockFreeQueue<std::vector<WvWPOIUpdate>> wvwPOIUpdates;

void LoadWvWObjectives();
void UpdateWvWStatus();
