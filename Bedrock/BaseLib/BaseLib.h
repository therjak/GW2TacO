#pragma once
#include "BaseConfig.h"
#include "Memory.h"
#include "StackTracker.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <tchar.h>

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "Assert.h"
#include "CrashTracker.h"
#include "CriticalSection.h"
#include "Logger.h"
#include "Matrix.h"
#include "Plane.h"
#include "Quaternion.h"
#include "Rectangle.h"
#include "Socket.h"
#include "Vector.h"

#define NoEmptyFile()              \
  namespace {                      \
  char NoEmptyFileDummy##__LINE__; \
  }
