#include "BaseLib.h"
#include <DbgHelp.h>
#include <time.h>

#include <string>
#include <string_view>

#include "string_format.h"

std::string BuildVersion;

LONG WINAPI baseCrashTracker(struct _EXCEPTION_POINTERS *excpInfo) {
  if (IsDebuggerPresent()) return EXCEPTION_CONTINUE_SEARCH;

  int nMoreDetail = 1;

  SYSTEMTIME st;
  GetLocalTime(&st);

  CreateDirectory(_T( "Crashlogs" ), NULL);

  int32_t filename[1024];
  ZeroMemory(filename, 1024);
  auto fname = FormatString(
      _T( "Crashlogs\\crashlog_%s_%04d_%02d_%02d_%02d_%02d_%02d.dmp" ),
      BuildVersion.c_str(), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute,
      st.wSecond);

  HANDLE hFile =
      CreateFile(fname.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
  if (hFile == INVALID_HANDLE_VALUE) return EXCEPTION_EXECUTE_HANDLER;

  MINIDUMP_EXCEPTION_INFORMATION eInfo;
  eInfo.ThreadId = GetCurrentThreadId();
  eInfo.ExceptionPointers = excpInfo;
  eInfo.ClientPointers = FALSE;

  MiniDumpWriteDump(
      GetCurrentProcess(), GetCurrentProcessId(), hFile,
      nMoreDetail
          ? (MINIDUMP_TYPE)(MiniDumpWithDataSegs + MiniDumpWithHandleData +
                            MiniDumpWithIndirectlyReferencedMemory)
          : MiniDumpNormal,
      excpInfo ? &eInfo : NULL, NULL, NULL);

  CloseHandle(hFile);

  CStackTracker Stack((void *)excpInfo->ContextRecord);
  auto Result = Stack.DumpToString();

  time_t rawtime;
  struct tm timeinfo;
  time(&rawtime);
  localtime_s(&timeinfo, &rawtime);

  char CrashString[2048];
  memset(CrashString, 0, 2048);
  sprintf_s(CrashString,
            "Crash occurred at: %d-%.2d-%.2d %.2d:%.2d:%.2d\nBuild version: "
            "%s\nException Type: %.8x\n%s\n\0",
            timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, BuildVersion.c_str(),
            excpInfo->ExceptionRecord->ExceptionCode, Result.c_str());

  TBOOL Saved = false;

  FILE *f = NULL;
  if (!fopen_s(&f, "crash.log", "at")) {
    if (f) {
      fputs(CrashString, f);
      fclose(f);
      Saved = true;
    }
  }

  LOG(LOG_ERROR, std::string(CrashString).c_str());
  Stack.DumpToDebugOutput();

  Logger.Close();

  if (Saved)
    MessageBox(
        NULL,
        _T( "Application has crashed.\nStack trace has been saved to crash.log" ),
        _T( "Crash" ), MB_ICONERROR);
  else
    MessageBox(
        NULL,
        _T( "Application has crashed.\nFailed to save stack trace to crash.log" ),
        _T( "Crash" ), MB_ICONERROR);

  return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI FullDumpCrashTracker(struct _EXCEPTION_POINTERS *excpInfo) {
  if (IsDebuggerPresent()) return EXCEPTION_CONTINUE_SEARCH;

  int nMoreDetail = 0;

  SYSTEMTIME st;
  GetLocalTime(&st);

  CreateDirectory(_T( "Crashlogs" ), NULL);

  int32_t filename[1024];
  ZeroMemory(filename, 1024);
  auto fname = FormatString(
      _T( "Crashlogs\\crashlog_%s_%04d_%02d_%02d_%02d_%02d_%02d.dmp" ),
      BuildVersion.c_str(), st.wYear, st.wMonth, st.wDay, st.wHour,
      st.wMinute, st.wSecond);

  HANDLE hFile =
      CreateFile(fname.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
  if (hFile == INVALID_HANDLE_VALUE) return EXCEPTION_EXECUTE_HANDLER;

  MINIDUMP_EXCEPTION_INFORMATION eInfo;
  eInfo.ThreadId = GetCurrentThreadId();
  eInfo.ExceptionPointers = excpInfo;
  eInfo.ClientPointers = FALSE;

  MiniDumpWriteDump(
      GetCurrentProcess(), GetCurrentProcessId(), hFile,
      // nMoreDetail ? (MINIDUMP_TYPE)(MiniDumpWithDataSegs +
      // MiniDumpWithHandleData + MiniDumpWithIndirectlyReferencedMemory) :
      // MiniDumpNormal,
      (MINIDUMP_TYPE)(MiniDumpWithFullMemory | MiniDumpWithFullMemoryInfo |
                      MiniDumpWithHandleData | MiniDumpWithUnloadedModules |
                      MiniDumpWithThreadInfo),
      excpInfo ? &eInfo : NULL, NULL, NULL);

  CloseHandle(hFile);

  Logger.Close();

  // if ( Saved )
  MessageBox(
      NULL,
      _T( "Application has crashed.\nDump has been saved to the Crashlogs folder" ),
      _T( "Crash" ), MB_ICONERROR);
  // else
  //  MessageBox( NULL, _T( "Application has crashed.\nFailed to save dump to
  //  the Crashlogs folder" ), _T( "Crash" ), MB_ICONERROR );

  return EXCEPTION_EXECUTE_HANDLER;
}

void InitializeCrashTracker(std::string_view Build,
                            LPTOP_LEVEL_EXCEPTION_FILTER Tracker) {
  BuildVersion = Build;
  SetUnhandledExceptionFilter(Tracker);
}
