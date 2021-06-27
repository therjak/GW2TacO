#include "Logger.h"

#include <Windows.h>
#include <debugapi.h>
#include <tchar.h>
#include <time.h>

#include <string>
#include <string_view>

#include "string_format.h"

CLogger Logger;

void CLoggerOutput::Process(LOGVERBOSITY v, std::string_view String) {}

CLoggerOutput::~CLoggerOutput() {}

CLoggerOutput::CLoggerOutput() {}

void CLoggerOutput_DebugOutput::Process(LOGVERBOSITY v,
                                        std::string_view String) {
  OutputDebugString(String.data());
  OutputDebugString(_T( "\n" ));
}

CLoggerOutput_DebugOutput::~CLoggerOutput_DebugOutput() {}

CLoggerOutput_DebugOutput::CLoggerOutput_DebugOutput() {}

void CLoggerOutput_StdOut::Process(LOGVERBOSITY v, std::string_view String) {
  _tprintf(_T( "%s\n" ), String.data());
}

CLoggerOutput_StdOut::~CLoggerOutput_StdOut() {}

CLoggerOutput_StdOut::CLoggerOutput_StdOut() {}

void CLoggerOutput_File::Process(LOGVERBOSITY v, std::string_view String) {
  if (!f) {
    if (!OpenLogFile(fname, Append)) return;
    if (!f) return;
  }
#ifndef UNICODE
  fprintf(f, "%s\n", String.data());
#else
  fwprintf(f, _T( "%s\n" ), String.data());
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
  f = 0;
}

CLoggerOutput_File::CLoggerOutput_File(std::string_view Filename,
                                       bool append /*= true*/) {
  Append = append;
  fname = Filename;
  OpenLogFile(Filename, Append);
}

CLoggerOutput_File::CLoggerOutput_File() {
  f = 0;
  Append = true;
  fname = _T( "log.log" );
}

int32_t CLogger::GetNewEntryCount() { return NewEntryCount; }

void CLogger::ResetEntryCounter() { NewEntryCount = 0; }

void CLogger::AddOutput(std::unique_ptr<CLoggerOutput>&& Output) {
  Outputs.emplace_back(std::move(Output));
}

void CLogger::SetVerbosity(LOGVERBOSITY v) { Verbosity = v; }

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

CLogger::~CLogger() {}

CLogger::CLogger() {
  Verbosity = LOGGER_BASE_OUTPUT_VERBOSITY;
#ifdef LOG_TO_DEBUGOUTPUT
  Outputs.emplace_back(std::make_unique<CLoggerOutput_DebugOutput>());
#endif
}

CLoggerOutput_RingBuffer::CLoggerOutput_RingBuffer() {}

CLoggerOutput_RingBuffer::~CLoggerOutput_RingBuffer() {}

void CLoggerOutput_RingBuffer::Process(LOGVERBOSITY v,
                                       std::string_view String) {
  Buffer.Add(std::string(String));
}

void CLoggerOutput_RingBuffer::Dump(std::string_view fname) {
  FILE* f = nullptr;
  fopen_s(&f, fname.data(), "w+b");
  for (int32_t x = 0; x < Buffer.NumItems(); x++)
    fprintf(f, "%s\n", Buffer[x].c_str());
  fclose(f);
}
