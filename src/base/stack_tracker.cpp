#include "src/base/stack_tracker.h"

#ifdef ENABLE_STACKTRACKER_CLASS

#include <windows.h>
// windows needs to be on the top.
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")
#include <tchar.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

#include "src/base/logger.h"

bool CStackTracker::DbgInitialized = false;

CStackTracker::CStackTracker() {
  memset(Stack, 0, sizeof(void*) * STACK_TRACE_DEPTH);
#ifdef _DEBUG
  RtlCaptureStackBackTrace(4, STACK_TRACE_DEPTH, Stack, nullptr);
#else
  RtlCaptureStackBackTrace(0, STACK_TRACE_DEPTH, Stack, NULL);
#endif
}

CStackTracker::CStackTracker(void* Context) {
  InitializeSym();
  memset(Stack, 0, sizeof(void*) * STACK_TRACE_DEPTH);

  CONTEXT* context = static_cast<CONTEXT*>(Context);

  DWORD machine_type;
  STACKFRAME frame;
  ZeroMemory(&frame, sizeof(frame));
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrFrame.Mode = AddrModeFlat;
  frame.AddrStack.Mode = AddrModeFlat;
#ifdef _M_X64
  frame.AddrPC.Offset = context->Rip;
  frame.AddrFrame.Offset = context->Rbp;
  frame.AddrStack.Offset = context->Rsp;
  machine_type = IMAGE_FILE_MACHINE_AMD64;
#else
  frame.AddrPC.Offset = context->Eip;
  frame.AddrFrame.Offset = context->Ebp;
  frame.AddrStack.Offset = context->Esp;
  machine_type = IMAGE_FILE_MACHINE_I386;
#endif

  for (int32_t i = 0; i < STACK_TRACE_DEPTH; i++) {
    if (StackWalk(machine_type, GetCurrentProcess(), GetCurrentThread(), &frame,
                  context, nullptr, SymFunctionTableAccess, SymGetModuleBase,
                  nullptr)) {
      if (i >= 0) {
        Stack[i] = (void*)(frame.AddrPC.Offset);
      }
    } else {
      break;
    }
  }
}

void CStackTracker::DumpToLog(LOGVERBOSITY v) {
  InitializeSym();

  for (auto& x : Stack) {
    if (x) {
      DWORD dwDisplacement;
      IMAGEHLP_LINE line;

      line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

      if (SymGetLineFromAddr(GetCurrentProcess(), (DWORD)x, &dwDisplacement,
                             &line)) {
        LOGSIMPLE(v, _T( "\t\t%s (%d)" ), line.FileName, line.LineNumber);
      } else {
        LOGSIMPLE(v, _T( "\t\tUnresolved address: %8X" ), x);
      }
    }
  }
  LOGSIMPLE(v, _T( "" ));
}

void CStackTracker::DumpToDebugOutput() {
  InitializeSym();

  for (auto& x : Stack) {
    if (x) {
      DWORD dwDisplacement;
      IMAGEHLP_LINE line;

      line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

      if (SymGetLineFromAddr(GetCurrentProcess(), (DWORD)x, &dwDisplacement,
                             &line)) {
        std::stringstream ss;
        ss << "\t\t" << line.FileName << " (" << line.LineNumber << ")\n";
        OutputDebugString(ss.str().c_str());
      } else {
        std::stringstream ss;
        ss << "\t\tUnresolved address: " << std::hex << x << '\n';
        OutputDebugString(ss.str().c_str());
      }
    }
  }
  OutputDebugString("\n");
}

std::string CStackTracker::DumpToString() {
  std::stringstream ss;

  InitializeSym();

  for (auto& x : Stack) {
    if (x) {
      DWORD dwDisplacement;
      IMAGEHLP_LINE line;

      line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

      if (SymGetLineFromAddr(GetCurrentProcess(), (DWORD)x, &dwDisplacement,
                             &line)) {
        ss << "\t\t" << line.FileName << " (" << line.LineNumber << ")\n";
      } else {
        ss << "\t\tUnresolved address: " << std::hex << x << '\n';
      }
    }
  }
  ss << '\n';

  return ss.str();
}

void CStackTracker::InitializeSym() {
  if (!DbgInitialized) {
    SymInitialize(GetCurrentProcess(), nullptr, true);
    SymSetOptions(SYMOPT_LOAD_LINES);
    DbgInitialized = true;
  }
}

#endif
