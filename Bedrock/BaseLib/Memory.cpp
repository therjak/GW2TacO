#include "Memory.h"

#include <cstdlib>

#include "BaseConfig.h"

#ifdef MEMORY_TRACKING

#include <tchar.h>
#include <windows.h>

#include <cstdio>
#include <new>

#undef new

// this should force the memTracker variable to be constructed before everything
// else:
#pragma warning(disable : 4074)
#pragma init_seg(compiler)
CMemTracker memTracker;

uint32_t TotalLeaked = 0;
uint32_t TotalAllocCount = 0;
uint32_t CurrentAllocCount = 0;

CMemTracker::CMemTracker() {
  InitializeLightweightCS(&critsec);
  Paused = true;
  MemTrackerPool =
      std::make_unique<std::unordered_map<void*, CAllocationInfo>>();
  Paused = false;
  IgnoreMissing = false;
}

void DumpMemleakEntry(CAllocationInfo& e) {
  OutputDebugString(
      (std::string("Leak:\t") + std::to_string(e.Size) + " bytes\n").c_str());

#ifndef ENABLE_MALLOC_STACK_TRACE
  OutputDebugString(
      (std::string("\t\t") + e.File + " (" + e.Line + ")\n").c_str());
#else
  e.Stack.DumpToDebugOutput();
#endif
  TotalLeaked += e.Size;
}

CMemTracker::~CMemTracker() {
  Paused = true;

  if (!MemTrackerPool->empty()) {
    // report leaks
    OutputDebugString("\n---Memleaks start here---\n\n");
    for (auto& item : *MemTrackerPool) {
      DumpMemleakEntry(item.second);
    }
    OutputDebugString((std::string("\tTotal bytes leaked: ") +
                       std::to_string(TotalLeaked) + "\n\n")
                          .c_str());
  } else {
    OutputDebugString(
        _T( "**********************************************************\n\t\t\t\t\tNo memleaks found.\n**********************************************************\n\n" ));
  }

  std::unique_ptr<std::unordered_map<void*, CAllocationInfo>> temp;
  std::swap(MemTrackerPool, temp);
}

void CMemTracker::AddPointer(void* p, const char* file, int32_t line,
                             int32_t size) {
  CLightweightCriticalSection cs(&critsec);
  if (MemTrackerPool && !Paused && p) {
    Paused = true;
    CAllocationInfo& allocinfo =
        CAllocationInfo(const_cast<char*>(file), line, size);
    MemTrackerPool->operator[](p) = allocinfo;
    Paused = false;
    CurrentAllocCount++;
    TotalAllocCount++;
  }
}

void CMemTracker::RemovePointer(void* p) {
  CLightweightCriticalSection cs(&critsec);
  if (!Paused && p) {
    CurrentAllocCount--;

    Paused = true;
    if (MemTrackerPool) {
      if (MemTrackerPool->find(p) != MemTrackerPool->end())
        MemTrackerPool->erase(p);
      else if (!IgnoreMissing) {
        //					OutputDebugString(_T("[mem]
        // Trying to delete non logged, possibly already freed memory
        // block!\n"));
        // #ifdef
        //_DEBUG 					CStackTracker s(-4);
        //#else CStackTracker s; #endif
        // s.DumpToDebugOutput();
      }
    }

    Paused = false;
  }
}

int32_t CMemTracker::GetAllocatedMemorySize() {
  Pause();

  int32_t mem = 0;
  for (auto& item : *MemTrackerPool) {
    mem += item.second.Size;
  }

  Resume();
  return mem;
}

//#pragma push_macro("new")
//#undef new

void* __cdecl operator new(size_t size, const char* file, int32_t line) {
  void* p = malloc(size);
  memTracker.AddPointer(p, file, line, size);
  return p;
}

void* __cdecl operator new[](size_t size, const char* file, int32_t line) {
  void* p = malloc(size);
  memTracker.AddPointer(p, file, line, size);
  return p;
}

//#pragma pop_macro("new")

void __cdecl operator delete(void* pointer) {
  memTracker.RemovePointer(pointer);
  free(pointer);
}

void __cdecl operator delete[](void* pointer) {
  memTracker.RemovePointer(pointer);
  free(pointer);
}

void __cdecl operator delete(void* pointer, const char* file, int32_t line) {
  memTracker.RemovePointer(pointer);
  free(pointer);
}

void __cdecl operator delete[](void* pointer, const char* file, int32_t line) {
  memTracker.RemovePointer(pointer);
  free(pointer);
}

#endif
