#include "src/base/logger.h"

#include <windows.h>
//
#include <debugapi.h>

#include <cstdint>
#include <cstdio>
#include <ctime>
#include <format>
#include <string>
#include <string_view>
#include <utility>

Logger logger;

void LoggerOutput::Process(LOGVERBOSITY v, const std::string& String) {}

LoggerOutput::~LoggerOutput() = default;

LoggerOutput::LoggerOutput() = default;

void LoggerOutput_DebugOutput::Process(LOGVERBOSITY v,
                                       const std::string& String) {
  OutputDebugString(String.c_str());
  OutputDebugString("\n");
}

LoggerOutput_DebugOutput::~LoggerOutput_DebugOutput() = default;

LoggerOutput_DebugOutput::LoggerOutput_DebugOutput() = default;

void LoggerOutput_File::Process(LOGVERBOSITY v, const std::string& String) {
  if (!f) {
    if (!OpenLogFile(fname, Append)) return;
    if (!f) return;
  }
  fprintf(f, "%s\n", String.c_str());
}

bool LoggerOutput_File::OpenLogFile(std::string_view Filename,
                                    bool Append /*= true*/) {
  if (!Append) {
    return (!fopen_s(&f, Filename.data(), "wt"));
  } else {
    return (!fopen_s(&f, Filename.data(), "at"));
  }
}

LoggerOutput_File::~LoggerOutput_File() {
  if (f) fclose(f);
  f = nullptr;
}

LoggerOutput_File::LoggerOutput_File(std::string_view Filename,
                                     bool append /*= true*/) {
  Append = append;
  fname = Filename;
  OpenLogFile(Filename, Append);
}

LoggerOutput_File::LoggerOutput_File() {
  f = nullptr;
  Append = true;
  fname = "log.log";
}

int32_t Logger::GetNewEntryCount() { return NewEntryCount; }

void Logger::ResetEntryCounter() { NewEntryCount = 0; }

void Logger::AddOutput(std::unique_ptr<LoggerOutput>&& Output) {
  Outputs.emplace_back(std::move(Output));
}

void Logger::SetVerbosity(LOGVERBOSITY v) { Verbosity = v; }

void Logger::Log(LOGVERBOSITY v, bool Prefix, bool AddTimeStamp,
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

void Logger::Close() { Outputs.clear(); }

Logger::~Logger() = default;

Logger::Logger() {
  Verbosity = LOGGER_BASE_OUTPUT_VERBOSITY;
  Outputs.emplace_back(std::make_unique<LoggerOutput_DebugOutput>());
}
