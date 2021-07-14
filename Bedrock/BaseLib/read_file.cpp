#include "read_file.h"

#include <Windows.h>
#include <fileapi.h>

#include <string>
#include <string_view>

namespace baselib {

std::string ReadFile(std::string_view name) {
  HANDLE hFile =
      CreateFile(name.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                 NULL, OPEN_EXISTING, NULL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    return std::string();
  }
  int32_t size = GetFileSize(hFile, NULL);
  auto ret = std::string(size, 0);
  DWORD nRead = 0;
  bool b = ::ReadFile(hFile, ret.data(), size, &nRead, NULL);
  CloseHandle(hFile);
  if (!b || nRead != size) {
    return std::string();
  }
  return ret;
}

}  // namespace baselib
