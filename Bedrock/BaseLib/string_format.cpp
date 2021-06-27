#include "string_format.h"

#include <cstdarg>

std::string FormatString(std::string_view format, ...) {
  if (format.empty()) {
    return std::string();
  }

  va_list args;
  va_start(args, format);
  std::string ret = FormatString(format, args);
  va_end(args);

  return ret;
}

std::string FormatString(std::string_view format, va_list args) {
  if (format.empty()) {
    return std::string();
  }

  va_list args_copy;
  va_copy(args_copy, args);
  int n = std::vsnprintf(nullptr, 0, format.data(), args_copy);
  va_end(args_copy);
  std::string ret(n + 1, 0);
  std::vsnprintf(ret.data(), ret.size(), format.data(), args);

  return ret;
}
