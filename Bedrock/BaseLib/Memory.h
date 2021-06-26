#pragma once

#include "Types.h"
#include "BaseConfig.h"

#ifdef MEMORY_TRACKING

void* __cdecl operator new( size_t size, const TS8* file, int32_t line );
void* __cdecl operator new[]( size_t size, const TS8* file, int32_t line );
void __cdecl operator delete( void* pointer, const TS8* file, int32_t line );
void __cdecl operator delete[]( void* pointer, const TS8* file, int32_t line );

#define new new(__FILE__, __LINE__)

#include "Dictionary.h"
#include "StackTracker.h"

class CAllocationInfo
{
public:
  TS8 *File;
  int32_t Line;
  int32_t Size;

#ifdef ENABLE_MALLOC_STACK_TRACE
  CStackTracker Stack;
#endif

  CAllocationInfo() {};
  CAllocationInfo( TS8 *file, int32_t line, int32_t size )
  {
    File = file;
    Line = line;
    Size = size;
  };
};

class CMemTracker
{
  LIGHTWEIGHT_CRITICALSECTION critsec;

  CDictionary<void*, CAllocationInfo> *MemTrackerPool;
  TBOOL Paused;
  TBOOL IgnoreMissing;

public:

  CMemTracker();
  virtual ~CMemTracker();
  void AddPointer( void *p, const TS8* file, int32_t line, int32_t size );
  void RemovePointer( void *p );
  TBOOL SetMissingIgnore( TBOOL b )
  {
    CLightweightCriticalSection cs( &critsec );
    TBOOL old = IgnoreMissing;
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

#define SAFEDELETE(x)  do { if (x) delete x;     x=NULL; } while (0)
#define SAFEDELETEA(x) do { if (x) delete[] x;   x=NULL; } while (0)
#define SAFEFREE(x)    do { if (x) free(x);      x=NULL; } while (0)

#ifdef MEMORY_TRACKING
#define IGNOREFREEERRORS(x) do { memTracker.SetMissingIgnore(x); } while(0)
#else
#define IGNOREFREEERRORS(x)
#endif
