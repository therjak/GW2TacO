#include "BuildInfo.h"
#include "BuildCount.h"

#ifdef _DEBUG
CString TacOBuild = CString::Format( "%.3d.%dd", RELEASECOUNT, BUILDCOUNT );
#else
CString TacOBuild = CString::Format( "%.3d.%dr", RELEASECOUNT, BUILDCOUNT );
#endif

CString buildDateTime = CString::Format( __DATE__ " " __TIME__ );

int32_t TacORelease = RELEASECOUNT;
int32_t TacOBuildCount = BUILDCOUNT;
