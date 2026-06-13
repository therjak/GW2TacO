#pragma once

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

using uint8_view = std::basic_string_view<uint8_t>;

class StreamWriter {
 public:
  StreamWriter();
  virtual ~StreamWriter();

  bool Write(std::string_view data);
  bool Write(uint8_view data);
  bool WriteDWord(uint32_t data);
  bool WriteByte(uint8_t data);

 private:
  virtual int32_t WriteStream(std::string_view data) = 0;
  bool Write(void* lpBuf, uint32_t nCount);

  uint8_t writerCurrentChar;
};

class StreamWriterMemory : public StreamWriter {
 public:
  StreamWriterMemory();
  ~StreamWriterMemory() override;

  // std::string_view GetData();
  const uint8_t* GetData() const;
  uint32_t GetLength() const;

  void Flush();

 private:
  int32_t WriteStream(std::string_view) override;

  std::vector<uint8_t> Data;
};

class StreamWriterFile : public StreamWriter {
 public:
  StreamWriterFile();
  ~StreamWriterFile() override;

  int32_t Open(std::string_view filename);
  void Flush();

 private:
  int32_t WriteStream(std::string_view) override;

  FILE* File;
};
