#pragma once

#include <windows.h>

#include <cstdint>
#include <memory>
#include <string_view>

using uint8_view = std::basic_string_view<uint8_t>;

class CStreamWriter {
 public:
  CStreamWriter();
  virtual ~CStreamWriter();

  bool Write(std::string_view data);
  bool Write(uint8_view data);
  bool WriteDWord(uint32_t data);
  bool WriteByte(uint8_t data);

 private:
  virtual int32_t WriteStream(std::string_view data) = 0;
  bool Write(void* lpBuf, uint32_t nCount);

  uint8_t writerCurrentChar;
};

class CStreamWriterMemory : public CStreamWriter {
 public:
  CStreamWriterMemory();
  ~CStreamWriterMemory() override;

  // std::string_view GetData();
  uint8_t* GetData();
  uint32_t GetLength();

  void Flush();

 private:
  int32_t WriteStream(std::string_view) override;

  std::unique_ptr<uint8_t[]> Data;
  uint32_t BufferSize;
  uint32_t DataLength;
};

class CStreamWriterFile : public CStreamWriter {
 public:
  CStreamWriterFile();
  ~CStreamWriterFile() override;

  int32_t Open(std::string_view filename);
  void Flush();

 private:
  int32_t WriteStream(std::string_view) override;

  HANDLE File;
};
