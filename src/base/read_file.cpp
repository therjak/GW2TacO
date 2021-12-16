#include "src/base/read_file.h"

#include <windows.h>
//
#include <fileapi.h>

#include <string>
#include <string_view>

namespace baselib {

std::string ReadFile(std::string_view name) {
  HANDLE hFile =
      CreateFile(name.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                 nullptr, OPEN_EXISTING, 0, nullptr);
  if (hFile == INVALID_HANDLE_VALUE) {
    return std::string();
  }
  int32_t size = GetFileSize(hFile, nullptr);
  auto ret = std::string(size, 0);
  DWORD nRead = 0;
  bool b = ::ReadFile(hFile, ret.data(), size, &nRead, nullptr);
  CloseHandle(hFile);
  if (!b || nRead != size) {
    return std::string();
  }
  return ret;
}

}  // namespace baselib
