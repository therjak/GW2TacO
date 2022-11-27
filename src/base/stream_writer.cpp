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

CStreamWriterMemory::CStreamWriterMemory() : CStreamWriter(), Data() {
  Data.reserve(1024);
}

CStreamWriterMemory::~CStreamWriterMemory() = default;

int32_t CStreamWriterMemory::WriteStream(std::string_view data) {
  Data.insert(Data.end(), data.begin(), data.end());
  return data.size();
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
                 nullptr, OPEN_ALWAYS, 0, nullptr);
  CloseHandle(File);

  File =
      CreateFile(fn.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                 nullptr, OPEN_ALWAYS | TRUNCATE_EXISTING, 0, nullptr);
  if (File == INVALID_HANDLE_VALUE) {
    LPTSTR pMsgBuf = nullptr;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  nullptr, GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  reinterpret_cast<LPTSTR>(&pMsgBuf), 0, nullptr);

    Log_Err("[writer] Error opening file '{:s}': {:s}", Filename, pMsgBuf);
    LocalFree(pMsgBuf);
    return 0;
  }
  return 1;
}

void CStreamWriterFile::Flush() {
  SetFilePointer(File, 0, nullptr, FILE_BEGIN);
  SetEndOfFile(File);
}
