#pragma once
#include "Bedrock/BaseLib/BaseLib.h"

#include <string>

class WvWObjective
{
public:
  std::string id;
  std::string type;
  std::string mapType;
  int mapID = 0;
  int objectiveID = 0;
  CVector3 coord;
  std::string marker;
  std::string chatLink;

  std::string name;
  std::string nameToken;
};

void LoadWvWObjectives();
void UpdateWvWStatus();
