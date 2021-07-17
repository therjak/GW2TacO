#include "Logger.h"

#include <Windows.h>
#include <debugapi.h>
#include <tchar.h>

#include <cstdint>
#include <ctime>
#include <string>
#include <string_view>

#include "string_format.h"

CLogger Logger;

void CLoggerOutput::Process(LOGVERBOSITY v, const std::string& String) {}

CLoggerOutput::~CLoggerOutput() = default;

CLoggerOutput::CLoggerOutput() = default;

void CLoggerOutput_DebugOutput::Process(LOGVERBOSITY v,
                                        const std::string& String) {
  OutputDebugString(String.c_str());
  OutputDebugString(_T( "\n" ));
}

CLoggerOutput_DebugOutput::~CLoggerOutput_DebugOutput() = default;

CLoggerOutput_DebugOutput::CLoggerOutput_DebugOutput() = default;

void CLoggerOutput_StdOut::Process(LOGVERBOSITY v, const std::string& String) {
  _tprintf(_T( "%s\n" ), String.c_str());
}

CLoggerOutput_StdOut::~CLoggerOutput_StdOut() = default;

CLoggerOutput_StdOut::CLoggerOutput_StdOut() = default;

void CLoggerOutput_File::Process(LOGVERBOSITY v, const std::string& String) {
  if (!f) {
    if (!OpenLogFile(fname, Append)) return;
    if (!f) return;
  }
#ifndef UNICODE
  fprintf(f, "%s\n", String.c_str());
#else
  fwprintf(f, _T( "%s\n" ), String.c_str());
#endif
}

bool CLoggerOutput_File::OpenLogFile(std::string_view Filename,
                                     bool Append /*= true*/) {
#ifndef UNICODE
  if (!Append)
    return (!fopen_s(&f, Filename.data(), "wt"));
  else
    return (!fopen_s(&f, Filename.data(), "at"));
#else
  CString s = Filename;
  char Fname[2048];
  s.WriteAsMultiByte(Fname, 2048);
  if (!Append)
    return (!fopen_s(&f, Fname, "wt, ccs= UTF-8 "));
  else
    return (!fopen_s(&f, Fname, "at, ccs= UTF-8 "));
#endif
}

CLoggerOutput_File::~CLoggerOutput_File() {
  if (f) fclose(f);
  f = nullptr;
}

CLoggerOutput_File::CLoggerOutput_File(std::string_view Filename,
                                       bool append /*= true*/) {
  Append = append;
  fname = Filename;
  OpenLogFile(Filename, Append);
}

CLoggerOutput_File::CLoggerOutput_File() {
  f = nullptr;
  Append = true;
  fname = _T( "log.log" );
}

int32_t CLogger::GetNewEntryCount() { return NewEntryCount; }

void CLogger::ResetEntryCounter() { NewEntryCount = 0; }

void CLogger::AddOutput(std::unique_ptr<CLoggerOutput>&& Output) {
  Outputs.emplace_back(std::move(Output));
}

void CLogger::SetVerbosity(LOGVERBOSITY v) { Verbosity = v; }

std::string FormatString(std::string_view format, va_list args) {
  if (format.empty()) {
    return std::string();
  }

  va_list args_copy;
  va_copy(args_copy, args);
  int n = std::vsnprintf(nullptr, 0, format.data(), args_copy);
  va_end(args_copy);
  std::string ret(n + 1, 0);
  std::vsnprintf(ret.data(), ret.size(), format.data(), args);

  return ret;
}

void CLogger::Log(LOGVERBOSITY v, bool Prefix, bool AddTimeStamp,
                  std::string_view String, ...) {
  if (Verbosity < v) return;

  va_list argList;
  va_start(argList, String);
  auto LogString = FormatString(String, argList);
  va_end(argList);

  time_t rawtime;
  tm timeinfo;

  time(&rawtime);
  localtime_s(&timeinfo, &rawtime);

  auto TimeStamp = FormatString(_T( "[%.4d-%.2d-%.2d %.2d:%.2d:%.2d] " ),
                                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1,
                                timeinfo.tm_mday, timeinfo.tm_hour,
                                timeinfo.tm_min, timeinfo.tm_sec);

  if (Prefix) {
    if (v < LOG_WARNING)
      LogString = _T( "(Err)  " ) + LogString;
    else if (v < LOG_INFO)
      LogString = _T( "(Warn) " ) + LogString;
    else if (v < LOG_DEBUG)
      LogString = _T( "(Info) " ) + LogString;
    else
      LogString = _T( "(Dbg)  " ) + LogString;
  }

  if (AddTimeStamp) LogString = TimeStamp + LogString;

  for (auto& output : Outputs) output->Process(v, LogString);

  NewEntryCount++;
}

void CLogger::Close() { Outputs.clear(); }

CLogger::~CLogger() = default;

CLogger::CLogger() {
  Verbosity = LOGGER_BASE_OUTPUT_VERBOSITY;
#ifdef LOG_TO_DEBUGOUTPUT
  Outputs.emplace_back(std::make_unique<CLoggerOutput_DebugOutput>());
#endif
}

CLoggerOutput_RingBuffer::CLoggerOutput_RingBuffer() = default;

CLoggerOutput_RingBuffer::~CLoggerOutput_RingBuffer() = default;

void CLoggerOutput_RingBuffer::Process(LOGVERBOSITY v,
                                       const std::string& String) {
  Buffer.Add(String);
}

void CLoggerOutput_RingBuffer::Dump(std::string_view fname) {
  FILE* f = nullptr;
  fopen_s(&f, fname.data(), "w+b");
  for (int32_t x = 0; x < Buffer.NumItems(); x++)
    fprintf(f, "%s\n", Buffer[x].c_str());
  fclose(f);
}
