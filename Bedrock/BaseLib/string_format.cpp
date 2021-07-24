#include "string_format.h"

#include <Windows.h>

#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <string>
#include <string_view>
#include <vector>

std::string_view whitespaces(" \t\f\v\n\r");

std::vector<std::string> Split(std::string_view input, std::string_view delim) {
  size_t start;
  size_t end = 0;
  std::vector<std::string> out;

  while ((start = input.find_first_not_of(delim, end)) !=
         std::string_view::npos) {
    end = input.find_first_of(delim, start);
    out.emplace_back(input.substr(start, end - start));
  }
  return out;
}

std::vector<std::string> SplitByWhitespace(std::string_view input) {
  return Split(input, whitespaces);
}

std::string_view Trim(std::string_view s) {
  {
    auto it = std::find_if_not(s.begin(), s.end(), std::isspace);
    s = s.substr(it - s.begin());
  }
  auto it = std::find_if_not(s.rbegin(), s.rend(), std::isspace);
  return s.substr(0, s.rend() - it);
}

static const char* B64chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const int B64index[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63, 52, 53, 54, 55, 56, 57,
    58, 59, 60, 61, 0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
    7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 0,  0,  0,  0,  63, 0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

const std::string B64Encode(const void* data, const size_t& len) {
  std::string result((len + 2) / 3 * 4, '=');
  auto p = static_cast<const unsigned char*>(data);
  auto str = result.data();
  size_t j = 0, pad = len % 3;
  const size_t last = len - pad;

  for (size_t i = 0; i < last; i += 3) {
    int n = int(p[i]) << 16 | int(p[i + 1]) << 8 | p[i + 2];
    str[j++] = B64chars[n >> 18];
    str[j++] = B64chars[n >> 12 & 0x3F];
    str[j++] = B64chars[n >> 6 & 0x3F];
    str[j++] = B64chars[n & 0x3F];
  }
  /// Set padding
  if (pad) {
    int n = --pad ? int(p[last]) << 8 | p[last + 1] : p[last];
    str[j++] = B64chars[pad ? n >> 10 & 0x3F : n >> 2];
    str[j++] = B64chars[pad ? n >> 4 & 0x03F : n << 4 & 0x3F];
    str[j++] = pad ? B64chars[n << 2 & 0x3F] : '=';
  }
  return result;
}

const std::string B64Decode(const void* data, const size_t& len) {
  if (len == 0) return "";

  auto p = static_cast<const unsigned char*>(data);
  size_t j = 0, pad1 = len % 4 || p[len - 1] == '=',
         pad2 = pad1 && (len % 4 > 2 || p[len - 2] != '=');
  const size_t last = (len - pad1) / 4 << 2;
  std::string result(last / 4 * 3 + pad1 + pad2, '\0');
  auto str = result.data();

  for (size_t i = 0; i < last; i += 4) {
    int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 |
            B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
    str[j++] = n >> 16;
    str[j++] = n >> 8 & 0xFF;
    str[j++] = n & 0xFF;
  }
  if (pad1) {
    int n = B64index[p[last]] << 18 | B64index[p[last + 1]] << 12;
    str[j++] = n >> 16;
    if (pad2) {
      n |= B64index[p[last + 2]] << 6;
      str[j++] = n >> 8 & 0xFF;
    }
  }
  return result;
}

std::string B64Encode(std::string_view str64) {
  return B64Encode(str64.data(), str64.size());
}

std::string B64Decode(std::string_view str64) {
  return B64Decode(str64.data(), str64.size());
}

std::wstring string2wstring(std::string_view s) {
  if (s.empty()) return {};
  int wchars_num =
      MultiByteToWideChar(CP_UTF8, 0, s.data(), s.size(), nullptr, 0);
  std::wstring wstr(wchars_num, 0);
  MultiByteToWideChar(CP_UTF8, 0, s.data(), s.size(), wstr.data(), wstr.size());
  return wstr;
}

std::string wstring2string(std::wstring_view s) {
  if (s.empty()) return {};
  int chars_num = WideCharToMultiByte(CP_UTF8, 0, s.data(), s.size(), nullptr,
                                      0, nullptr, nullptr);
  std::string str(chars_num, 0);
  WideCharToMultiByte(CP_UTF8, 0, s.data(), s.size(), str.data(), chars_num,
                      nullptr, nullptr);
  return str;
}

uint32_t CalculateHash(std::string_view s) {
  uint32_t Hash = 5381;

  if (s.empty()) return Hash;

  // djb2 hash
  for (const auto& c : s) {
    Hash = ((Hash << 5) + Hash) + c;
  }
  return Hash;
}
