#pragma once
#include <cstdint>
#include <ctime>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "ring_buffer.h"

#define ENABLE_LOGGING

#define LOG_TO_DEBUGOUTPUT
//#define LOG_TO_STDOUT

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

class CLoggerOutput_StdOut : public CLoggerOutput {
 public:
  CLoggerOutput_StdOut();
  ~CLoggerOutput_StdOut() override;
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

class CLoggerOutput_RingBuffer : public CLoggerOutput {
  CRingBuffer<std::string> Buffer;

 public:
  CLoggerOutput_RingBuffer();
  ~CLoggerOutput_RingBuffer() override;
  void Process(LOGVERBOSITY v, const std::string& String) override;
  void Dump(std::string_view fname);
};

class CLogger {
  std::vector<std::unique_ptr<CLoggerOutput>> Outputs;
  LOGVERBOSITY Verbosity = LOGVERBOSITY::LOG_NONE;
  int32_t NewEntryCount = 0;

 public:
  CLogger();
  virtual ~CLogger();
  void Close();
  void Log(LOGVERBOSITY v, bool Prefix, bool TimeStamp, std::string_view String,
           ...);
  void SetVerbosity(LOGVERBOSITY v);
  void AddOutput(std::unique_ptr<CLoggerOutput>&& Output);
  void ResetEntryCounter();
  int32_t GetNewEntryCount();
};

extern CLogger Logger;

#ifdef ENABLE_LOGGING
#define LOG(v, s, ...) Logger.Log(v, true, true, s, __VA_ARGS__)
#else
#define LOG(v, s, ...) \
  do {                 \
  } while (0)
#endif

#ifdef ENABLE_LOGGING
#define LOGSIMPLE(v, s, ...) Logger.Log(v, false, false, s, __VA_ARGS__)
#else
#define LOGSIMPLE(v, s, ...) \
  do {                       \
  } while (0)
#endif

#define LOG_WARN(s, ...) LOG(LOGVERBOSITY::LOG_WARNING, s, __VA_ARGS__)
#define LOG_ERR(s, ...) LOG(LOGVERBOSITY::LOG_ERROR, s, __VA_ARGS__)
#define LOG_DBG(s, ...) LOG(LOGVERBOSITY::LOG_DEBUG, s, __VA_ARGS__)
#define LOG_NFO(s, ...) LOG(LOGVERBOSITY::LOG_INFO, s, __VA_ARGS__)

#define LOG_SETVERBOSITY(v) Logger.SetVerbosity(v)
#define LOG_ADDOUTPUT(v) Logger.AddOutput(v)

/*
#define FORCEDOPENDEBUGFILE()                                                  \
  do {                                                                         \
    HANDLE h =                                                                 \
        CreateFile("debug.log", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, \
                   NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);          \
    CloseHandle(h);                                                            \
  } while (0)
#define FORCEDDEBUGLOG(v, ...)                                               \
  do {                                                                       \
    HANDLE h = CreateFile("debug.log", FILE_APPEND_DATA, FILE_SHARE_READ, 0, \
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);          \
    char str[1024];                                                          \
    for (int yyy = 0; yyy < 1024; yyy++) str[yyy] = 0;                       \
    sprintf_s(str, 1023, v, __VA_ARGS__);                                    \
    int xxx = 0;                                                             \
    for (int zzz = 0; zzz < 1023; zzz++)                                     \
      if (str[zzz] != 0)                                                     \
        xxx++;                                                               \
      else                                                                   \
        break;                                                               \
    str[xxx] = '\n';                                                         \
    WriteFile(h, str, xxx + 1, NULL, NULL);                                  \
    CloseHandle(h);                                                          \
  } while (0)
*/

#define FORCEDOPENDEBUGFILE() \
  do {                        \
  } while (0)
#define FORCEDDEBUGLOG(v, ...) \
  do {                         \
  } while (0)
