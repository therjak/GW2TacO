#include "src/base/stream_reader.h"

#include <cstdio>

#include <algorithm>
#include <array>
#include <string_view>

#include "src/base/assert.h"

//////////////////////////////////////////////////////////////////////////
// base class implementation

CStreamReader::CStreamReader() {
  readerBitOffset = 0;
  readerLastChar = 0;
}

CStreamReader::~CStreamReader() = default;

int32_t CStreamReader::Read(void* lpBuf, uint32_t nCount) {
  if (readerBitOffset == 0) {  // non bitstream mode
    return ReadStream(lpBuf, nCount);
  }

  // bitstream mode
  for (uint32_t x = 0; x < nCount; x++) {
    (static_cast<uint8_t*>(lpBuf))[x] = ReadBits(8);
  }

  return nCount;
}

uint32_t CStreamReader::ReadDWord() {
  uint32_t i = 0;
  BASEASSERT(Read(&i, sizeof(uint32_t)) == sizeof(uint32_t));
  return i;
}

uint8_t CStreamReader::ReadByte() {
  uint8_t i = 0;
  BASEASSERT(Read(&i, sizeof(uint8_t)) == sizeof(uint8_t));
  return i;
}

uint32_t CStreamReader::ReadBits(uint32_t BitCount) {
  BASEASSERT(BitCount <= 64);

  uint32_t result = 0;

  while (BitCount > 0) {
    // read next byte if needed
    if (readerBitOffset == 0) BASEASSERT(ReadStream(&readerLastChar, 1) == 1);

    uint32_t count = std::min(8 - readerBitOffset, BitCount);
    uint32_t mask = (1 << count) - 1;

    uint8_t bits = readerLastChar >> readerBitOffset;
    result |= (bits & mask) << (BitCount - count);
    BitCount -= count;

    readerBitOffset = (readerBitOffset + count) & 7;
  }

  return result;
}

std::string CStreamReaderMemory::ReadLine() {
  std::string result;

  std::array<char, 2> s{{0, 0}};

  do {
    if (GetOffset() == GetLength()) break;

    s[0] = ReadByte();

    if (s[0] != '\n' && s[0] != '\r') result += s[0];

  } while (s[0] && s[0] != '\n');

  return result;
}

bool CStreamReader::eof() { return GetOffset() >= GetLength(); }

//////////////////////////////////////////////////////////////////////////
// streamreader memory

CStreamReaderMemory::CStreamReaderMemory() : CStreamReader() {}

CStreamReaderMemory::~CStreamReaderMemory() = default;

int32_t CStreamReaderMemory::ReadStream(void* lpBuf, uint32_t nCount) {
  auto bytestoread =
      std::max(static_cast<std::vector<uint8_t>::size_type>(0),
               std::min(static_cast<std::vector<uint8_t>::size_type>(nCount),
                        Data.size() - Offset));
  memcpy(lpBuf, &Data[Offset], static_cast<size_t>(bytestoread));
  Offset += bytestoread;
  return static_cast<int32_t>(bytestoread);
}

int32_t CStreamReaderMemory::Open(uint8_t* data, uint32_t size) {
  if (!data || !size) return 0;

  Offset = 0;
  Data.resize(size);
  memcpy(&Data[0], data, size);

  return 1;
}

int32_t CStreamReaderMemory::Open(std::string_view Filename) {
  std::string filename_str(Filename);
  FILE* file = std::fopen(filename_str.c_str(), "rb");
  if (!file) return 0;

  std::fseek(file, 0, SEEK_END);
  long tDataSize = std::ftell(file);
  std::fseek(file, 0, SEEK_SET);

  if (tDataSize < 0) {
    std::fclose(file);
    return 0;
  }

  auto tData = std::vector<uint8_t>(tDataSize);
  size_t nRead = 0;
  if (tDataSize > 0) {
    nRead = std::fread(tData.data(), 1, tDataSize, file);
  }

  if (nRead == static_cast<size_t>(tDataSize)) {
    // all ok
    Data = std::move(tData);
    Offset = 0;
  }

  std::fclose(file);
  return nRead == static_cast<size_t>(tDataSize);
}

const uint8_t* CStreamReaderMemory::GetData() const { return &Data[0]; }

int64_t CStreamReaderMemory::GetLength() const {
  return static_cast<int64_t>(Data.size());
}

int64_t CStreamReaderMemory::GetOffset() const {
  return static_cast<int64_t>(Offset);
}
