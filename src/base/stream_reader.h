#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class CStreamReader {
 public:
  CStreamReader();
  virtual ~CStreamReader();

  // general purpose reading functions with bitstream support
  // these aren't virtual as they all fall back on ReadStream at one point and
  // should not be overridden
  int32_t Read(void* lpBuf, uint32_t nCount);
  uint32_t ReadDWord();
  uint8_t ReadByte();

  [[nodiscard]] virtual int64_t GetLength() const = 0;
  bool eof();

 private:
  // this reads nCount bytes from the stream
  virtual int32_t ReadStream(void* lpBuf, uint32_t nCount) = 0;
  [[nodiscard]] virtual int64_t GetOffset() const = 0;
  uint32_t ReadBits(uint32_t BitCount);

  uint32_t readerBitOffset;
  uint8_t readerLastChar;
};

class CStreamReaderMemory : public CStreamReader {
 public:
  CStreamReaderMemory();
  ~CStreamReaderMemory() override;

  int32_t Open(uint8_t* data, uint32_t size);
  int32_t Open(std::string_view filename);
  std::string ReadLine();

  [[nodiscard]] const uint8_t* GetData() const;
  [[nodiscard]] int64_t GetLength() const override;

 private:
  int32_t ReadStream(void* lpBuf, uint32_t nCount) override;
  [[nodiscard]] int64_t GetOffset() const override;

  std::vector<uint8_t> Data;
  std::vector<uint8_t>::size_type Offset = 0;
};
