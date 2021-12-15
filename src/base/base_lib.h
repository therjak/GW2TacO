#pragma once
#include "base_config.h"
#include "memory.h"
#include "stack_tracker.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <tchar.h>

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "assert.h"
#include "crash_tracker.h"
#include "critical_section.h"
#include "logger.h"
#include "matrix.h"
#include "plane.h"
#include "quaternion.h"
#include "rectangle.h"
#include "socket.h"
#include "vector.h"

#define NoEmptyFile()              \
  namespace {                      \
  char NoEmptyFileDummy##__LINE__; \
  }
