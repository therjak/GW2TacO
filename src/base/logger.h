#pragma once
#include <cstdint>
#include <ctime>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

enum class LOGVERBOSITY : uint16_t {
  LOG_ERROR = 0x100,
  LOG_WARNING = 0x200,
  LOG_INFO = 0x300,
  LOG_DEBUG = 0x400,
  LOG_NONE = 0x500,
};

// base verbosity for the log:
constexpr auto LOGGER_BASE_OUTPUT_VERBOSITY = LOGVERBOSITY::LOG_DEBUG;

// logged items with a verbosity above this value are killed compile time:
constexpr auto LOGGER_DROPPED_OUTPUT_VERBOSITY = LOGVERBOSITY::LOG_WARNING;

class CLoggerOutput {
 public:
  CLoggerOutput();
  virtual ~CLoggerOutput();
  virtual void Process(LOGVERBOSITY v, const std::string& String);
};

class CLoggerOutput_DebugOutput : public CLoggerOutput {
 public:
  CLoggerOutput_DebugOutput();
  ~CLoggerOutput_DebugOutput() override;
  void Process(LOGVERBOSITY v, const std::string& String) override;
};

class CLoggerOutput_File : public CLoggerOutput {
  FILE* f;
  std::string fname;
  bool Append;

 public:
  CLoggerOutput_File();
  explicit CLoggerOutput_File(std::string_view Filename, bool append = true);
  ~CLoggerOutput_File() override;
  bool OpenLogFile(std::string_view Filename, bool Append = true);
  void Process(LOGVERBOSITY v, const std::string& String) override;
};

class CLogger {
  std::vector<std::unique_ptr<CLoggerOutput>> Outputs;
  LOGVERBOSITY Verbosity = LOGVERBOSITY::LOG_NONE;
  int32_t NewEntryCount = 0;

 public:
  CLogger();
  virtual ~CLogger();
  void Close();
  void LLog(LOGVERBOSITY v, bool Prefix, bool TimeStamp,
            std::string_view String, ...);
  void Log(LOGVERBOSITY v, bool Prefix, bool TimeStamp,
           std::string_view String);
  void SetVerbosity(LOGVERBOSITY v);
  void AddOutput(std::unique_ptr<CLoggerOutput>&& Output);
  void ResetEntryCounter();
  int32_t GetNewEntryCount();
};

extern CLogger Logger;

template <class... Args>
void Log(LOGVERBOSITY v, const std::string_view fmt, Args&&... args) {
  Logger.Log(v, true, true, std::format(fmt, args...));
}

template <class... Args>
void Log_Warn(const std::string_view fmt, Args&&... args) {
  Log(LOGVERBOSITY::LOG_WARNING, fmt, args...);
}

template <class... Args>
void Log_Err(const std::string_view fmt, Args&&... args) {
  Log(LOGVERBOSITY::LOG_ERROR, fmt, args...);
}

template <class... Args>
void Log_Dbg(const std::string_view fmt, Args&&... args) {
  Log(LOGVERBOSITY::LOG_DEBUG, fmt, args...);
}

template <class... Args>
void Log_Nfo(const std::string_view fmt, Args&&... args) {
  Log(LOGVERBOSITY::LOG_INFO, fmt, args...);
}

#define LOG(v, s, ...) Logger.LLog(v, true, true, s, __VA_ARGS__)

#define LOG_WARN(s, ...) LOG(LOGVERBOSITY::LOG_WARNING, s, __VA_ARGS__)
#define LOG_ERR(s, ...) LOG(LOGVERBOSITY::LOG_ERROR, s, __VA_ARGS__)
#define LOG_DBG(s, ...) LOG(LOGVERBOSITY::LOG_DEBUG, s, __VA_ARGS__)
#define LOG_NFO(s, ...) LOG(LOGVERBOSITY::LOG_INFO, s, __VA_ARGS__)
