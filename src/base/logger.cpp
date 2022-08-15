#include "src/base/logger.h"

#include <windows.h>
//
#include <debugapi.h>

#include <cstdint>
#include <ctime>
#include <format>
#include <string>
#include <string_view>

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
  if (!Append) {
    return (!fopen_s(&f, Filename.data(), "wt"));
  } else {
    return (!fopen_s(&f, Filename.data(), "at"));
  }
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

void CLogger::Log(LOGVERBOSITY v, bool Prefix, bool AddTimeStamp,
                  std::string_view String) {
  if (Verbosity < v) return;

  auto ts = [AddTimeStamp]() -> std::string {
    if (!AddTimeStamp) {
      return "";
    }
    time_t rawtime = 0;
    tm timeinfo{};

    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);

    return std::format("[{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}] ",
                       timeinfo.tm_year + 1900, timeinfo.tm_mon + 1,
                       timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min,
                       timeinfo.tm_sec);
  };

  auto pf = [Prefix, v]() -> std::string {
    if (Prefix) {
      if (v < LOGVERBOSITY::LOG_WARNING) {
        return "(Err)  ";
      } else if (v < LOGVERBOSITY::LOG_INFO) {
        return "(Warn) ";
      } else if (v < LOGVERBOSITY::LOG_DEBUG) {
        return "(Info) ";
      } else {
        return "(Dbg)  ";
      }
    }
    return "";
  };

  auto LogString = std::format("{:s}{:s}{:s}", ts(), pf(), String);

  for (auto& output : Outputs) {
    output->Process(v, LogString);
  }

  NewEntryCount++;
}

void CLogger::Close() { Outputs.clear(); }

CLogger::~CLogger() = default;

CLogger::CLogger() {
  Verbosity = LOGGER_BASE_OUTPUT_VERBOSITY;
  Outputs.emplace_back(std::make_unique<CLoggerOutput_DebugOutput>());
}
