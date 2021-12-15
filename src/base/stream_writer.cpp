#include "src/base/stream_writer.h"

#include "src/base/assert.h"
#include "src/base/logger.h"

CStreamWriter::CStreamWriter() { writerCurrentChar = 0; }

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

CStreamWriterMemory::CStreamWriterMemory() : CStreamWriter() {
  Data = std::make_unique<uint8_t[]>(1024);
  BufferSize = 1024;
  DataLength = 0;
}

CStreamWriterMemory::~CStreamWriterMemory() = default;

int32_t CStreamWriterMemory::WriteStream(std::string_view data) {
  if (DataLength + data.size() > BufferSize) {
    BufferSize = static_cast<uint32_t>((BufferSize + data.size()) * 1.2f);
    auto temp = std::make_unique<uint8_t[]>(BufferSize);
    std::swap(Data, temp);
    memcpy(Data.get(), temp.get(), DataLength);
  }

  memcpy(Data.get() + DataLength, data.data(), data.size());
  DataLength += data.size();

  return data.size();
}

uint8_t* CStreamWriterMemory::GetData() { return Data.get(); }

uint32_t CStreamWriterMemory::GetLength() { return DataLength; }

void CStreamWriterMemory::Flush() {
  Data = std::make_unique<uint8_t[]>(1024);
  BufferSize = 1024;
  DataLength = 0;
}

//////////////////////////////////////////////////////////////////////////
// streamwriterfile

CStreamWriterFile::CStreamWriterFile() : CStreamWriter() { File = nullptr; }

CStreamWriterFile::~CStreamWriterFile() {
  if (File) CloseHandle(File);
}

int32_t CStreamWriterFile::WriteStream(std::string_view data) {
  DWORD nWritten = 0;
  BOOL b = WriteFile(File, data.data(), data.size(), &nWritten, nullptr);
  if (!b) return 0;
  return nWritten;
}

int32_t CStreamWriterFile::Open(std::string_view Filename) {
  if (File) CloseHandle(File);  // close previous handle

  std::string fn(Filename);

  File =
      CreateFile(fn.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                 nullptr, OPEN_ALWAYS, NULL, nullptr);
  CloseHandle(File);

  File =
      CreateFile(fn.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                 nullptr, OPEN_ALWAYS | TRUNCATE_EXISTING, NULL, nullptr);
  if (File == INVALID_HANDLE_VALUE) {
    LPTSTR pMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  nullptr, GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  reinterpret_cast<LPTSTR>(&pMsgBuf), 0, nullptr);

    LOG_ERR("[writer] Error opening file '%s': %s",
            std::string(Filename).c_str(), pMsgBuf);
    LocalFree(pMsgBuf);
    return 0;
  }
  return 1;
}

void CStreamWriterFile::Flush() {
  SetFilePointer(File, 0, nullptr, FILE_BEGIN);
  SetEndOfFile(File);
}
