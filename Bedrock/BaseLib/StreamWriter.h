#pragma once

#include <Windows.h>

#include <cstdint>
#include <memory>
#include <string_view>

using uint8_view = std::basic_string_view<uint8_t>;

class CStreamWriter {
  uint8_t writerCurrentChar;
  virtual int32_t WriteStream(std::string_view data) = 0;
  bool Write(void* lpBuf, uint32_t nCount);

 public:
  CStreamWriter();
  virtual ~CStreamWriter();

  bool Write(std::string_view data);
  bool Write(uint8_view data);
  bool WriteDWord(uint32_t data);
  bool WriteByte(uint8_t data);
};

class CStreamWriterMemory : public CStreamWriter {
  std::unique_ptr<uint8_t[]> Data;
  uint32_t BufferSize;
  uint32_t DataLength;

  int32_t WriteStream(std::string_view) override;

 public:
  CStreamWriterMemory();
  ~CStreamWriterMemory() override;

  // std::string_view GetData();
  uint8_t* GetData();
  uint32_t GetLength();

  void Flush();
};

class CStreamWriterFile : public CStreamWriter {
  HANDLE File;

  int32_t WriteStream(std::string_view) override;

 public:
  CStreamWriterFile();
  ~CStreamWriterFile() override;

  int32_t Open(std::string_view filename);
  void Flush();
};
