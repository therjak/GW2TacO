module;
#include <string>
#include "src/build_count.h"

module taco.build_info;

namespace {
const std::string taco_version(TACO_VERSION);
}

#ifdef NDEBUG 
const std::string taco_build_ = taco_version;
#else
const std::string taco_build_ = taco_version + ":d";
#endif

const std::string build_date_time_(__DATE__ " " __TIME__);
