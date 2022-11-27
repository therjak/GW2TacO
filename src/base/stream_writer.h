#pragma once

#include <windows.h>

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

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
  const uint8_t* GetData() const;
  uint32_t GetLength() const;

  void Flush();

 private:
  int32_t WriteStream(std::string_view) override;

  std::vector<uint8_t> Data;
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
