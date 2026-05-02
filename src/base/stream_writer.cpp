#include "src/base/stream_writer.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>

#include "src/base/assert.h"
#include "src/base/logger.h"

CStreamWriter::CStreamWriter() : writerCurrentChar(0) {}

CStreamWriter::~CStreamWriter() = default;

bool CStreamWriter::Write(std::string_view data) {
  return WriteStream(data) == data.size();
}

bool CStreamWriter::Write(uint8_view data) {
  std::string_view d(reinterpret_cast<const char*>(data.data()), data.size());
  return WriteStream(d) == d.size();
}

bool CStreamWriter::Write(void* lpBuf, uint32_t nCount) {
  return WriteStream(std::string_view(reinterpret_cast<char*>(lpBuf),
                                      nCount)) == nCount;
}

bool CStreamWriter::WriteByte(uint8_t data) { return Write(&data, 1); }

bool CStreamWriter::WriteDWord(uint32_t data) { return Write(&data, 4); }

//////////////////////////////////////////////////////////////////////////
// streamwritermemory

CStreamWriterMemory::CStreamWriterMemory() : CStreamWriter(), Data() {
  Data.reserve(1024);
}

CStreamWriterMemory::~CStreamWriterMemory() = default;

int32_t CStreamWriterMemory::WriteStream(std::string_view data) {
  Data.insert(Data.end(), data.begin(), data.end());
  return static_cast<int32_t>(data.size());
}

const uint8_t* CStreamWriterMemory::GetData() const { return &Data[0]; }

uint32_t CStreamWriterMemory::GetLength() const {
  return static_cast<uint32_t>(Data.size());
}

void CStreamWriterMemory::Flush() { Data.clear(); }

//////////////////////////////////////////////////////////////////////////
// streamwriterfile

CStreamWriterFile::CStreamWriterFile() : CStreamWriter() { File = nullptr; }

CStreamWriterFile::~CStreamWriterFile() {
  if (File) std::fclose(File);
}

int32_t CStreamWriterFile::WriteStream(std::string_view data) {
  if (!File) return 0;
  size_t nWritten = std::fwrite(data.data(), 1, data.size(), File);
  return static_cast<int32_t>(nWritten);
}

int32_t CStreamWriterFile::Open(std::string_view Filename) {
  if (File) std::fclose(File);  // close previous handle

  std::string fn(Filename);

  File = std::fopen(fn.c_str(), "wb");
  if (!File) {
    Log_Err("[writer] Error opening file '{:s}': {:s}", Filename,
            std::strerror(errno));
    return 0;
  }
  return 1;
}

void CStreamWriterFile::Flush() {
  if (File) {
    std::fflush(File);
  }
}
