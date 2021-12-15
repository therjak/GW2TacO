#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "BaseConfig.h"

#ifdef MEMORY_TRACKING

void* __cdecl operator new(size_t size, const char* file, int32_t line);
void* __cdecl operator new[](size_t size, const char* file, int32_t line);
void __cdecl operator delete(void* pointer, const char* file, int32_t line);
void __cdecl operator delete[](void* pointer, const char* file, int32_t line);

#define new new (__FILE__, __LINE__)

#include "CriticalSection.h"
#include "StackTracker.h"

class CAllocationInfo {
 public:
  const char* File = nullptr;
  int32_t Line = 0;
  int32_t Size = 0;

#ifdef ENABLE_MALLOC_STACK_TRACE
  CStackTracker Stack;
#endif

  CAllocationInfo() = default;
  ;
  CAllocationInfo(const char* file, int32_t line, int32_t size) {
    File = file;
    Line = line;
    Size = size;
  };
};

class CMemTracker {
  LIGHTWEIGHT_CRITICALSECTION critsec;

  std::unique_ptr<std::unordered_map<void*, CAllocationInfo>> MemTrackerPool;
  bool Paused;
  bool IgnoreMissing;

 public:
  CMemTracker();
  virtual ~CMemTracker();
  void AddPointer(void* p, const char* file, int32_t line, int32_t size);
  void RemovePointer(void* p);
  bool SetMissingIgnore(bool b) {
    CLightweightCriticalSection cs(&critsec);
    const bool old = IgnoreMissing;
    IgnoreMissing = b;
    return old;
  }

  void Pause() { Paused = true; }
  void Resume() { Paused = false; }

  int32_t GetAllocatedMemorySize();
};

extern CMemTracker memTracker;
extern uint32_t TotalAllocCount;
extern uint32_t CurrentAllocCount;

#endif

#define SAFEDELETE(x) \
  {                   \
    delete x;         \
    x = NULL;         \
  }
#define SAFEDELETEA(x) \
  {                    \
    delete[] x;        \
    x = NULL;          \
  }
#define SAFEFREE(x) \
  {                 \
    free(x);        \
    x = NULL;       \
  }

#ifdef MEMORY_TRACKING
#define IGNOREFREEERRORS(x)         \
  do {                              \
    memTracker.SetMissingIgnore(x); \
  } while (0)
#else
#define IGNOREFREEERRORS(x)
#endif
