#pragma once

// Call stack tracker class for tracking resource allocations

#include <string>

#include "src/base/base_config.h"
#include "src/base/logger.h"

#ifdef ENABLE_STACKTRACKER_CLASS
#define STACK_TRACE_DEPTH 10

class CStackTracker {
  static bool DbgInitialized;

 public:
  void* Stack[STACK_TRACE_DEPTH];

  CStackTracker();
  explicit CStackTracker(void* Context);
  void DumpToLog(LOGVERBOSITY v);
  void DumpToDebugOutput();
  std::string DumpToString();

  static void InitializeSym();
};

#else

class CStackTracker {
 public:
  CStackTracker(){};
  CStackTracker(void* Context) {}
  virtual ~CStackTracker() {}
  void DumpToLog(LOGVERBOSITY v) {}
  void DumpToDebugOutput() {}
  std::string DumpToString() { return {}; }
};

#endif
