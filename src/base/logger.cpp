#include "src/base/logger.h"

#include <windows.h>
//
#include <debugapi.h>
#include <tchar.h>

#include <cstdint>
#include <ctime>
#include <string>
#include <string_view>

#include "src/base/string_format.h"

CLogger Logger;

void CLoggerOutput::Process(LOGVERBOSITY v, const std::string& String) {}

CLoggerOutput::~CLoggerOutput() = default;

CLoggerOutput::CLoggerOutput() = default;

void CLoggerOutput_DebugOutput::Process(LOGVERBOSITY v,
                                        const std::string& String) {
  OutputDebugString(String.c_str());
  OutputDebugString("\n");
}

CLoggerOutput_DebugOutput::~CLoggerOutput_DebugOutput() = default;

CLoggerOutput_DebugOutput::CLoggerOutput_DebugOutput() = default;

void CLoggerOutput_File::Process(LOGVERBOSITY v, const std::string& String) {
  if (!f) {
    if (!OpenLogFile(fname, Append)) return;
    if (!f) return;
  }
  fprintf(f, "%s\n", String.c_str());
}

bool CLoggerOutput_File::OpenLogFile(std::string_view Filename,
                                     bool Append /*= true*/) {
  if (!Append)
    return (!fopen_s(&f, Filename.data(), "wt"));
  else
    return (!fopen_s(&f, Filename.data(), "at"));
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
  fname = "log.log";
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

  auto TimeStamp =
      FormatString("[%.4d-%.2d-%.2d %.2d:%.2d:%.2d] ", timeinfo.tm_year + 1900,
                   timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour,
                   timeinfo.tm_min, timeinfo.tm_sec);

  if (Prefix) {
    if (v < LOGVERBOSITY::LOG_WARNING)
      LogString = "(Err)  " + LogString;
    else if (v < LOGVERBOSITY::LOG_INFO)
      LogString = "(Warn) " + LogString;
    else if (v < LOGVERBOSITY::LOG_DEBUG)
      LogString = "(Info) " + LogString;
    else
      LogString = "(Dbg)  " + LogString;
  }

  if (AddTimeStamp) LogString = TimeStamp + LogString;

  for (auto& output : Outputs) output->Process(v, LogString);

  NewEntryCount++;
}

void CLogger::Close() { Outputs.clear(); }

CLogger::~CLogger() = default;

CLogger::CLogger() {
  Verbosity = LOGGER_BASE_OUTPUT_VERBOSITY;
  Outputs.emplace_back(std::make_unique<CLoggerOutput_DebugOutput>());
}
