#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

class CStreamReader {
  uint32_t readerBitOffset;
  uint8_t readerLastChar;
  // this reads nCount bytes from the stream
  virtual int32_t ReadStream(void* lpBuf, uint32_t nCount) = 0;
  [[nodiscard]] virtual int64_t GetOffset() const = 0;
  uint32_t ReadBits(uint32_t BitCount);

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
};

class CStreamReaderMemory : public CStreamReader {
  std::unique_ptr<uint8_t[]> Data;
  uint64_t DataSize;
  uint64_t Offset;

  int32_t ReadStream(void* lpBuf, uint32_t nCount) override;
  [[nodiscard]] int64_t GetOffset() const override;

 public:
  CStreamReaderMemory();
  ~CStreamReaderMemory() override;

  int32_t Open(uint8_t* data, uint32_t size);
  int32_t Open(std::string_view filename);
  std::string ReadLine();

  [[nodiscard]] uint8_t* GetData() const;
  [[nodiscard]] int64_t GetLength() const override;
};
