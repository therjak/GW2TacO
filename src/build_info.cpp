#include "src/build_info.h"

#include "src/build_count.h"

namespace {
const std::string taco_version(TACO_VERSION);
}

#ifdef NDEBUG 
const std::string TacOBuild = taco_version;
#else
const std::string TacOBuild = taco_version + ":d";
#endif

const std::string buildDateTime(__DATE__ " " __TIME__);
