#include "BuildInfo.h"
#include "BuildCount.h"

#include "Bedrock/BaseLib/string_format.h"

#ifdef _DEBUG
std::string TacOBuild = FormatString( "%.3d.%dd", RELEASECOUNT, BUILDCOUNT );
#else
std::string TacOBuild = FormatString("%.3d.%dr", RELEASECOUNT, BUILDCOUNT);
#endif

std::string buildDateTime( __DATE__ " " __TIME__ );

int32_t TacORelease = RELEASECOUNT;
int32_t TacOBuildCount = BUILDCOUNT;
