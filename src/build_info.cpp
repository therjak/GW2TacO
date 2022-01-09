#include "src/build_info.h"

#include <format>

#include "src/build_count.h"

#ifdef _DEBUG
std::string TacOBuild = std::format("{:03d}.{:d}d", RELEASECOUNT, BUILDCOUNT);
#else
std::string TacOBuild = std::format("{:03d}.{:d}r", RELEASECOUNT, BUILDCOUNT);
#endif

std::string buildDateTime(__DATE__ " " __TIME__);

int32_t TacORelease = RELEASECOUNT;
int32_t TacOBuildCount = BUILDCOUNT;
